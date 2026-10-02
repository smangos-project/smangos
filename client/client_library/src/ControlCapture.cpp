#define NOMINMAX
#include "ControlCapture.hpp"

#include "SwapChainCapture.hpp"

#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <cstring>

using Microsoft::WRL::ComPtr;

static bool IsBlack(std::vector<std::uint8_t> const& pixels, int width, int height)
{
    for (int y = 0; y < height; y += std::max(1, height / 16)) {
        for (int x = 0; x < width; x += std::max(1, width / 16)) {
            std::uint8_t const* p = &pixels[(static_cast<std::size_t>(y) * width + x) * 4];
            if (p[0] | p[1] | p[2])
                return false;
        }
    }
    return true;
}

// Top-down BGRX pixels of the whole client area.
static bool GrabClient(HWND window, int width, int height, std::vector<std::uint8_t>& pixels)
{
    int frameWidth = 0;
    int frameHeight = 0;
    if (CaptureSwapChain(frameWidth, frameHeight, pixels) && frameWidth == width && frameHeight == height)
        return true;

    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    bool ok = bitmap && bits;
    if (ok) {
        HGDIOBJ previous = SelectObject(dc, bitmap);
        std::size_t const size = static_cast<std::size_t>(width) * height * 4;
        pixels.resize(size);

        // PW_CLIENTONLY | PW_RENDERFULLCONTENT
        PrintWindow(window, dc, 3);
        std::memcpy(pixels.data(), bits, size);
        if (IsBlack(pixels, width, height)) {
            POINT origin = {0, 0};
            ClientToScreen(window, &origin);
            BitBlt(dc, 0, 0, width, height, screen, origin.x, origin.y, SRCCOPY | CAPTUREBLT);
            std::memcpy(pixels.data(), bits, size);
        }
        SelectObject(dc, previous);
    }
    if (bitmap)
        DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return ok;
}

static bool Encode(std::vector<std::uint8_t>& pixels, int width, int height, int outWidth, int outHeight, bool png,
            std::vector<std::uint8_t>& out)
{
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
        return false;

    ComPtr<IWICBitmap> bitmap;
    if (FAILED(factory->CreateBitmapFromMemory(width, height, GUID_WICPixelFormat32bppBGR, width * 4,
                                               static_cast<UINT>(pixels.size()), pixels.data(), &bitmap)))
        return false;

    ComPtr<IWICBitmapSource> source = bitmap;
    if (outWidth != width || outHeight != height) {
        ComPtr<IWICBitmapScaler> scaler;
        if (FAILED(factory->CreateBitmapScaler(&scaler)) ||
            FAILED(scaler->Initialize(bitmap.Get(), outWidth, outHeight, WICBitmapInterpolationModeFant)))
            return false;
        source = scaler;
    }

    ComPtr<IStream> stream;
    ComPtr<IWICBitmapEncoder> encoder;
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> properties;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)) ||
        FAILED(factory->CreateEncoder(png ? GUID_ContainerFormatPng : GUID_ContainerFormatJpeg, nullptr, &encoder)) ||
        FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache)) ||
        FAILED(encoder->CreateNewFrame(&frame, &properties)))
        return false;

    if (!png) {
        PROPBAG2 option = {};
        wchar_t name[] = L"ImageQuality";
        option.pstrName = name;
        VARIANT value;
        VariantInit(&value);
        value.vt = VT_R4;
        value.fltVal = 0.85f;
        properties->Write(1, &option, &value);
    }

    WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
    if (FAILED(frame->Initialize(properties.Get())) || FAILED(frame->SetSize(outWidth, outHeight)) ||
        FAILED(frame->SetPixelFormat(&format)) || FAILED(frame->WriteSource(source.Get(), nullptr)) ||
        FAILED(frame->Commit()) || FAILED(encoder->Commit()))
        return false;

    STATSTG stat = {};
    HGLOBAL global = nullptr;
    if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)) || FAILED(GetHGlobalFromStream(stream.Get(), &global)))
        return false;
    void* data = GlobalLock(global);
    if (!data)
        return false;
    out.assign(static_cast<std::uint8_t*>(data), static_cast<std::uint8_t*>(data) + stat.cbSize.QuadPart);
    GlobalUnlock(global);
    return true;
}

CaptureResult CaptureWindow(HWND window, CaptureOptions const& options)
{
    CaptureResult result;

    RECT client = {};
    if (!GetClientRect(window, &client) || client.right <= 0 || client.bottom <= 0) {
        result.error = "window has no client area (minimized?)";
        return result;
    }
    int const fullWidth = client.right;
    int const fullHeight = client.bottom;

    std::vector<std::uint8_t> full;
    if (!GrabClient(window, fullWidth, fullHeight, full)) {
        result.error = "could not capture the window";
        return result;
    }

    RECT region = {0, 0, fullWidth, fullHeight};
    if (options.hasRegion) {
        region.left = std::clamp<LONG>(options.region.left, 0, fullWidth);
        region.top = std::clamp<LONG>(options.region.top, 0, fullHeight);
        region.right = std::clamp<LONG>(options.region.right, region.left, fullWidth);
        region.bottom = std::clamp<LONG>(options.region.bottom, region.top, fullHeight);
    }
    int const width = region.right - region.left;
    int const height = region.bottom - region.top;
    if (width <= 0 || height <= 0) {
        result.error = "region is empty";
        return result;
    }

    std::vector<std::uint8_t> pixels;
    if (width == fullWidth && height == fullHeight) {
        pixels = std::move(full);
    } else {
        pixels.resize(static_cast<std::size_t>(width) * height * 4);
        for (int y = 0; y < height; ++y)
            std::memcpy(&pixels[static_cast<std::size_t>(y) * width * 4],
                        &full[(static_cast<std::size_t>(region.top + y) * fullWidth + region.left) * 4],
                        static_cast<std::size_t>(width) * 4);
    }

    int outWidth = width;
    int outHeight = height;
    if (options.maxWidth > 0 && width > options.maxWidth) {
        outWidth = options.maxWidth;
        outHeight = std::max(1, static_cast<int>(static_cast<double>(height) * outWidth / width + 0.5));
    }

    if (!Encode(pixels, width, height, outWidth, outHeight, options.png, result.bytes)) {
        result.error = "image encoding failed";
        return result;
    }

    result.ok = true;
    result.mime = options.png ? "image/png" : "image/jpeg";
    result.width = outWidth;
    result.height = outHeight;
    result.scale = static_cast<double>(outWidth) / width;
    result.originX = region.left;
    result.originY = region.top;
    return result;
}
