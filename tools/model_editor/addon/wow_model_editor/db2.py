"""Minimal WDC5 reader for the few 1.15 client tables the importer needs (no string fields)."""
import struct

# (kind, count) per field: i/h/b/q signed int of 32/16/8/64 bits, "u" suffix unsigned, f float
LAYOUTS = {
    "creaturedisplayinfo": ["iu", "hu", "hu", "b", "f", "bu", "bu", "i", "hu", "hu", "i", "i", "hu", "hu", "i", "i", "f",
                            "f", "b", "i", "i", "b", "i", "b", "hu", ("i", 4)],
    "creaturemodeldata": [("f", 6), "i", "iu", "iu", "iu", "f", "f", "f", "iu", "iu", "iu", "iu", "iu", "f", "f", "f", "iu",
                          "f", "f", "f", "f", "f", "f", "f", "f", "f", "f", "f", "hu"],
    "creaturedisplayinfogeosetdata": ["bu", "bu"],
}
WIDTH = {"i": 32, "h": 16, "b": 8, "q": 64, "f": 32}


def _bits(data, bitoff, nbits):
    b = bitoff >> 3
    v = int.from_bytes(data[b:b + ((bitoff & 7) + nbits + 7) // 8], "little")
    return (v >> (bitoff & 7)) & ((1 << nbits) - 1)


def _sx(v, n):
    return v - (1 << n) if v & (1 << (n - 1)) else v


def _norm(layout):
    return [(f, 1) if isinstance(f, str) else f for f in layout]


class Table:
    """rows: {id: [field values]}; array fields are lists, relation (parent) ids are appended last."""

    def __init__(self, data, layout):
        d = data
        if d[:4] != b"WDC5":
            raise ValueError("not a WDC5 file")
        self.layout = _norm(layout)
        o = 4 + 4 + 128
        (_, self.field_count, self.record_size, _, _, _, _, _, _) = struct.unpack_from("<9I", d, o)
        o += 36
        self.flags, self.id_index = struct.unpack_from("<HH", d, o)
        o += 4
        (total_fields, _, _, fsi_size, common_size, pallet_size, section_count) = struct.unpack_from("<7I", d, o)
        o += 28
        sections = []
        for _ in range(section_count):
            s = struct.unpack_from("<Q8I", d, o)
            o += 40
            sections.append(dict(off=s[1], nrec=s[2], strsize=s[3], recend=s[4], idsize=s[5], relsize=s[6],
                                 offmap=s[7], copies=s[8]))
        o += 4 * total_fields
        self.fsi = [struct.unpack_from("<HHIIIII", d, o + 24 * i) for i in range(fsi_size // 24)]
        o += fsi_size
        self.pallet = d[o:o + pallet_size]
        o += pallet_size
        common = d[o:o + common_size]
        o += common_size
        self.common, co = [], 0
        for f in self.fsi:
            m = {}
            if f[3] == 2:
                for k in range(f[2] // 8):
                    rid, val = struct.unpack_from("<II", common, co + 8 * k)
                    m[rid] = val
                co += f[2]
            self.common.append(m)
        self.rows = {}
        for s in sections:
            self._section(d, s)

    def _value(self, fi, rec, rid):
        off, size, _, stype, a, _, c = self.fsi[fi]
        if stype == 0:
            n = self.layout[fi][1]
            w = size // n
            return [_bits(rec, off + w * k, w) for k in range(n)], w
        if stype in (1, 5):
            v = _bits(rec, off, size)
            return [_sx(v, size) if stype == 5 else v], 32
        if stype == 2:
            return [self.common[fi].get(rid, a)], 32
        pal = sum(x[2] for x in self.fsi[:fi] if x[3] in (3, 4))
        idx = _bits(rec, off, size)
        if stype == 3:
            return [struct.unpack_from("<I", self.pallet, pal + idx * 4)[0]], 32
        if stype == 4:
            return [struct.unpack_from("<I", self.pallet, pal + (idx * c + j) * 4)[0] for j in range(c)], 32
        raise ValueError(f"field storage {stype}")

    def _convert(self, fi, raw, w):
        kind = self.layout[fi][0]
        base = kind[0]
        if base == "f":
            return [struct.unpack("<f", struct.pack("<I", x & 0xFFFFFFFF))[0] for x in raw]
        bits = WIDTH[base]
        if kind.endswith("u"):
            return [x & ((1 << bits) - 1) for x in raw]
        return [_sx(x & ((1 << bits) - 1), bits) for x in raw] if self.fsi[fi][3] != 1 else raw

    def _section(self, d, s):
        o = s["off"]
        sparse = self.flags & 1
        if sparse:
            o = s["recend"]
            recs = b""
        else:
            recs = d[o:o + s["nrec"] * self.record_size]
            o += s["nrec"] * self.record_size + s["strsize"]
        ids = list(struct.unpack_from("<%dI" % (s["idsize"] // 4), d, o))
        o += s["idsize"]
        copies = [struct.unpack_from("<II", d, o + 8 * k) for k in range(s["copies"])]
        o += 8 * s["copies"]
        offmap = [struct.unpack_from("<IH", d, o + 6 * k) for k in range(s["offmap"])]
        o += 6 * s["offmap"]
        rel = {}
        if s["relsize"]:
            n = struct.unpack_from("<I", d, o)[0]
            for k in range(n):
                fk, ri = struct.unpack_from("<II", d, o + 12 + 8 * k)
                rel[ri] = fk
            o += s["relsize"]
        if sparse:
            offids = list(struct.unpack_from("<%dI" % s["offmap"], d, o))
            for k, (roff, _) in enumerate(offmap):
                p, vals = roff, []
                for kind, n in self.layout:
                    raw = []
                    for _ in range(n):
                        nb = WIDTH[kind[0]] // 8
                        raw.append(int.from_bytes(d[p:p + nb], "little"))
                        p += nb
                    v = self._convert(len(vals), raw, 0)
                    vals.append(v if n > 1 else v[0])
                rid = offids[k] if offids else ids[k]
                if k in rel:
                    vals.append(rel[k])
                self.rows[rid] = vals
        else:
            for r in range(s["nrec"]):
                rec = recs[r * self.record_size:(r + 1) * self.record_size]
                rid = ids[r] if ids else self._value(self.id_index, rec, 0)[0][0]
                vals = []
                for fi in range(len(self.layout)):
                    raw, w = self._value(fi, rec, rid)
                    v = self._convert(fi, raw, w)
                    vals.append(v if len(v) > 1 else v[0])
                if r in rel:
                    vals.append(rel[r])
                self.rows[rid] = vals
        for new, old in copies:
            if old in self.rows:
                self.rows[new] = self.rows[old]


def read(path, name):
    with open(path, "rb") as f:
        return Table(f.read(), LAYOUTS[name]).rows
