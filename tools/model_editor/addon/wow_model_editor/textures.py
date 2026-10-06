"""BLP <-> Blender image conversion with a PNG cache."""
import struct
import zlib
from pathlib import Path

import bpy
import numpy as np
import wowlib
from wowlib.formats import blp as wblp

from . import client, index

PLACEHOLDER = "wow_missing"
missing = {}


def cache_dir():
    d = Path(bpy.path.abspath(client.prefs().cache_dir)) / "blp"
    d.mkdir(parents=True, exist_ok=True)
    return d


def write_png(path, rgba):
    h, w, _ = rgba.shape
    raw = np.concatenate([np.zeros((h, 1), np.uint8), rgba.reshape(h, w * 4)], axis=1).tobytes()

    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data))

    png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(raw, 1)) + chunk(b"IEND", b""))
    Path(path).write_bytes(png)


def decode(fdid):
    """RGBA8 array (top row first) for a texture fdid, or raise."""
    blp = wblp.BLP()
    blp.read(client.session().fs, client.key(fdid))
    img = blp.decode()
    return np.asarray(img.pixels, dtype=np.uint8).reshape(img.height, img.width, 4)


def _placeholder():
    img = bpy.data.images.get(PLACEHOLDER)
    if img is None:
        img = bpy.data.images.new(PLACEHOLDER, 8, 8)
        img.pixels.foreach_set(np.tile(np.array([1, 0, 1, 1], np.float32), 64))
        img.pack()
    return img


def get_image(fdid):
    """Blender image for a texture fdid; undecodable files get the shared magenta placeholder."""
    fdid = int(fdid)
    path = index.path_of(fdid) if fdid else None
    if not fdid:
        return _placeholder()
    png = cache_dir() / f"{fdid}.png"
    try:
        if not png.is_file():
            write_png(png, decode(fdid))
    except Exception as e:  # noqa: BLE001
        missing[fdid] = f"{path or '?'}: {e}"
        return _placeholder()
    img = bpy.data.images.load(str(png), check_existing=True)
    if "wow_fdid" not in img:
        img.name = Path(path).stem if path else str(fdid)
        img["wow_fdid"] = fdid
        img["wow_path"] = path or ""
        img.alpha_mode = "CHANNEL_PACKED"
    return img


def take_missing():
    out = dict(missing)
    missing.clear()
    return out


def image_to_rgba(image):
    w, h = image.size
    px = np.empty(w * h * 4, np.float32)
    image.pixels.foreach_get(px)
    px = np.clip(np.rint(px.reshape(h, w, 4) * 255), 0, 255).astype(np.uint8)
    return px[::-1]


def encode_blp(image):
    """BLP bytes for a Blender image; DXT1 without alpha, else DXT5 unless the source BLP says otherwise."""
    rgba = np.ascontiguousarray(image_to_rgba(image))
    settings = wblp.EncodeSettings()
    src_fdid = image.get("wow_fdid")
    has_alpha = bool((rgba[..., 3] < 255).any())
    if src_fdid:
        try:
            src = wblp.BLP()
            src.read(client.session().fs, client.key(int(src_fdid)))
            settings = wblp.EncodeSettings(src.color_encoding, src.preferred_format, src.alpha_depth, True)
        except Exception:  # noqa: BLE001
            src_fdid = None
    if not src_fdid:
        settings = wblp.EncodeSettings(wblp.ColorEncoding.Dxt,
                                       wblp.PixelFormat.Dxt5 if has_alpha else wblp.PixelFormat.Dxt1,
                                       8 if has_alpha else 0, True)
    wimg = wblp.Image(rgba.shape[1], rgba.shape[0], wowlib.VectorUnsignedChar(rgba.tobytes()))
    blp = wblp.BLP()
    blp.encode(wimg, settings)
    return bytes(blp.write())


def export_blp(image, path):
    """Add `image` as BLP to the file system at a client path; returns its fdid."""
    return client.session().fs.add_file(path, encode_blp(image)).value
