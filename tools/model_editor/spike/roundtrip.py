"""Usage: roundtrip.py PATH...  (client-internal .wmo/.m2 paths)"""
import sys
import tempfile

import rt
from env import open_fs

if __name__ == "__main__":
    out = tempfile.mkdtemp(prefix="wl-rt-")
    with open_fs(out) as fs:
        for p in sys.argv[1:]:
            print(rt.roundtrip(fs, out, p))
