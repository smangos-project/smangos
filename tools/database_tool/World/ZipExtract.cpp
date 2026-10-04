#include "ZipExtract.h"

#include <zlib.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
    uint32_t Le16(uint8_t const* p) { return p[0] | (p[1] << 8); }
    uint32_t Le32(uint8_t const* p) { return Le16(p) | (Le16(p + 2) << 16); }

    bool ReadAt(std::ifstream& in, uint64_t offset, void* out, size_t size)
    {
        in.clear();
        in.seekg(static_cast<std::streamoff>(offset));
        in.read(static_cast<char*>(out), static_cast<std::streamsize>(size));
        return static_cast<size_t>(in.gcount()) == size;
    }
}

std::string ExtractZipEntry(std::filesystem::path const& zipPath, std::string const& entryName,
                            std::filesystem::path const& destination)
{
    std::ifstream in(zipPath, std::ios::binary);
    if (!in)
        return "cannot open " + zipPath.string();

    std::error_code ec;
    uint64_t const fileSize = std::filesystem::file_size(zipPath, ec);
    if (ec)
        return "cannot stat " + zipPath.string();

    // End of central directory: signature 0x06054b50 within the last 64K + 22 bytes.
    size_t const tailSize = static_cast<size_t>(std::min<uint64_t>(fileSize, 65557));
    std::vector<uint8_t> tail(tailSize);
    if (!ReadAt(in, fileSize - tailSize, tail.data(), tailSize))
        return "cannot read zip tail";

    size_t eocd = std::string::npos;
    for (size_t i = tailSize >= 22 ? tailSize - 22 + 1 : 0; i-- > 0;)
        if (Le32(&tail[i]) == 0x06054b50)
        {
            eocd = i;
            break;
        }
    if (eocd == std::string::npos)
        return "not a zip file: " + zipPath.string();

    uint32_t const entryCount = Le16(&tail[eocd + 10]);
    uint32_t const directorySize = Le32(&tail[eocd + 12]);
    uint32_t const directoryOffset = Le32(&tail[eocd + 16]);
    if (entryCount == 0xFFFF || directoryOffset == 0xFFFFFFFFu)
        return "zip64 archives are not supported";

    std::vector<uint8_t> directory(directorySize);
    if (!ReadAt(in, directoryOffset, directory.data(), directorySize))
        return "cannot read zip central directory";

    size_t pos = 0;
    for (uint32_t n = 0; n < entryCount && pos + 46 <= directory.size(); ++n)
    {
        uint8_t const* h = &directory[pos];
        uint32_t const method = Le16(h + 10);
        uint32_t const compressedSize = Le32(h + 20);
        uint32_t const uncompressedSize = Le32(h + 24);
        uint32_t const nameLength = Le16(h + 28);
        uint32_t const extraLength = Le16(h + 30);
        uint32_t const commentLength = Le16(h + 32);
        uint32_t const localOffset = Le32(h + 42);
        std::string const name(reinterpret_cast<char const*>(h + 46), nameLength);
        pos += 46 + nameLength + extraLength + commentLength;
        if (name != entryName)
            continue;

        uint8_t local[30];
        if (!ReadAt(in, localOffset, local, sizeof(local)) || Le32(local) != 0x04034b50)
            return "bad local header for " + entryName;
        uint64_t const dataOffset = localOffset + 30ull + Le16(local + 26) + Le16(local + 28);
        if (method != 0 && method != 8)
            return "unsupported zip compression method for " + entryName;

        std::ofstream out(destination, std::ios::binary | std::ios::trunc);
        if (!out)
            return "cannot write " + destination.string();

        z_stream stream{};
        if (method == 8 && inflateInit2(&stream, -15) != Z_OK)
            return "inflateInit failed";

        std::vector<uint8_t> inBuf(1 << 20), outBuf(1 << 20);
        uint64_t remaining = compressedSize, written = 0, offset = dataOffset;
        int status = Z_OK;
        while (remaining > 0 && status != Z_STREAM_END)
        {
            size_t const chunk = static_cast<size_t>(std::min<uint64_t>(remaining, inBuf.size()));
            if (!ReadAt(in, offset, inBuf.data(), chunk))
            {
                if (method == 8)
                    inflateEnd(&stream);
                return "truncated zip data for " + entryName;
            }
            offset += chunk;
            remaining -= chunk;

            if (method == 0)
            {
                out.write(reinterpret_cast<char const*>(inBuf.data()), static_cast<std::streamsize>(chunk));
                written += chunk;
                continue;
            }

            stream.next_in = inBuf.data();
            stream.avail_in = static_cast<uInt>(chunk);
            do
            {
                stream.next_out = outBuf.data();
                stream.avail_out = static_cast<uInt>(outBuf.size());
                status = inflate(&stream, Z_NO_FLUSH);
                if (status != Z_OK && status != Z_STREAM_END)
                {
                    inflateEnd(&stream);
                    return "inflate failed for " + entryName;
                }
                size_t const produced = outBuf.size() - stream.avail_out;
                out.write(reinterpret_cast<char const*>(outBuf.data()), static_cast<std::streamsize>(produced));
                written += produced;
            } while (stream.avail_out == 0 && status != Z_STREAM_END);
        }
        if (method == 8)
            inflateEnd(&stream);

        out.close();
        if (written != uncompressedSize)
            return "size mismatch extracting " + entryName;
        return {};
    }

    return "no entry " + entryName + " in " + zipPath.string();
}
