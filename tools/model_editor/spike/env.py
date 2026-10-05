"""Shared setup for the wowlib spike scripts: wheel on sys.path, 1.15 filesystem over .client/Content."""
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent / ".build" / "site"))

import wowlib  # noqa: E402
from wowlib import fs as wfs  # noqa: E402

CONTENT = Path(os.environ.get("MODEL_EDITOR_CONTENT", ROOT / ".client" / "Content"))
FDID_INDEX = CONTENT / "fdid.index"
VERSION = wowlib.versions.classic_era


def open_fs(out_dir):
    settings = wfs.FileSystemSettings(
        CONTENT, VERSION,
        project_directory=Path(out_dir),
        listfile_csv=FDID_INDEX,
        loose_client=True,
    )
    return wfs.FileSystem.open(settings)
