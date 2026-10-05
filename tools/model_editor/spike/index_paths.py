import os
import re

from env import CONTENT

_G = re.compile(r"_(\d{3}|lod\d+)\.wmo$")


def wmos():
    for line in open(CONTENT / "fdid.index", encoding="utf-8", errors="replace"):
        path = line.rstrip("\n").partition(";")[2].lower()
        if path.endswith(".wmo") and not _G.search(path) and os.path.isfile(CONTENT / path):
            yield path
