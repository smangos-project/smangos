"""Copies files into .client/Content with a restore manifest. Usage: client_check.py install SRC_DIR | restore | online"""
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
CONTENT = ROOT / ".client" / "Content"
BACKUP = Path("/tmp/me-client-backup")
MANIFEST = BACKUP / "manifest.json"


def online():
    out = subprocess.run(["mariadb", "-h127.0.0.1", "-umangos", "-pmangos", "characters", "-N", "-e",
                          "select count(*) from characters where online=1"], capture_output=True, text=True)
    return int(out.stdout.strip() or 0)


def install(src):
    if online():
        raise SystemExit("characters are online, not touching Content")
    manifest = json.loads(MANIFEST.read_text()) if MANIFEST.is_file() else {}
    for f in sorted(Path(src).rglob("*")):
        if not f.is_file():
            continue
        rel = f.relative_to(src).as_posix()
        dst = CONTENT / rel
        if rel not in manifest:
            if dst.is_file():
                b = BACKUP / "files" / rel
                b.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(dst, b)
                manifest[rel] = True
            else:
                manifest[rel] = False
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(f, dst)
        print("installed", rel)
    BACKUP.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest))


def restore():
    if not MANIFEST.is_file():
        return
    for rel, had in json.loads(MANIFEST.read_text()).items():
        dst = CONTENT / rel
        if had:
            shutil.copy2(BACKUP / "files" / rel, dst)
        elif dst.is_file():
            dst.unlink()
        print("restored", rel)
    shutil.rmtree(BACKUP)


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "install":
        install(sys.argv[2])
    elif cmd == "restore":
        restore()
    else:
        print(online())
