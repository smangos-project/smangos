"""Copies exported files from the Output directory into the client Content tree and restores the stock files again."""
import hashlib
import json
import shutil
import subprocess
from pathlib import Path


class InstallError(RuntimeError):
    pass


def store_dir(content):
    """Backups and the manifest live next to Content, so the client never sees them."""
    return Path(content).parent / ".model-editor-install"


def _manifest_path(content):
    return store_dir(content) / "manifest.json"


def manifest(content):
    p = _manifest_path(content)
    return json.loads(p.read_text()) if p.is_file() else {}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def online_count():
    """Number of online characters, or None when the database can't be asked."""
    try:
        out = subprocess.run(["mariadb", "-h127.0.0.1", "-umangos", "-pmangos", "characters", "-N", "-e",
                              "select count(*) from characters where online=1"], capture_output=True, text=True, timeout=10)
        return int(out.stdout.strip())
    except (OSError, ValueError, subprocess.SubprocessError):
        return None


def output_files(output):
    out = Path(output)
    return sorted(p.relative_to(out).as_posix() for p in out.rglob("*") if p.is_file())


def stale(content, output, rels=None):
    """Managed files whose Content copy differs from the Output copy."""
    rels = rels if rels is not None else manifest(content).keys()
    return [r for r in rels if (Path(output) / r).is_file() and (not (Path(content) / r).is_file() or sha(Path(content) / r) != sha(Path(output) / r))]


def install(content, output, rels, check_online=True):
    """Copies `rels` (client paths, relative to both trees) from Output into Content, backing up what it replaces."""
    content, output = Path(content), Path(output)
    if check_online and (online_count() or 0) > 0:
        raise InstallError("characters are online; log them out first")
    missing = [r for r in rels if not (output / r).is_file()]
    if missing:
        raise InstallError(f"not in the output directory: {', '.join(missing[:5])}")
    store = store_dir(content)
    man = manifest(content)
    for rel in rels:
        dst = content / rel
        if rel not in man:
            if dst.is_file():
                backup = store / "files" / rel
                backup.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(dst, backup)
                man[rel] = {"had": True, "sha": sha(dst)}
            else:
                man[rel] = {"had": False}
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(output / rel, dst)
    store.mkdir(parents=True, exist_ok=True)
    _manifest_path(content).write_text(json.dumps(man, indent=1, sort_keys=True))
    return list(rels)


def uninstall(content, rels=None, check_online=True):
    """Restores the stock files (all managed ones, or `rels`) and removes files that did not exist before."""
    content = Path(content)
    if check_online and (online_count() or 0) > 0:
        raise InstallError("characters are online; log them out first")
    store, man = store_dir(content), manifest(content)
    done = []
    for rel in sorted(rels if rels is not None else man):
        entry = man.get(rel)
        if entry is None:
            continue
        dst = content / rel
        if entry["had"]:
            backup = store / "files" / rel
            if not backup.is_file() or sha(backup) != entry["sha"]:
                raise InstallError(f"backup of {rel} is missing or changed")
            shutil.copy2(backup, dst)
            backup.unlink()
        else:
            dst.unlink(missing_ok=True)
            for parent in dst.parents:
                if parent == content or any(parent.iterdir()):
                    break
                parent.rmdir()
        del man[rel]
        done.append(rel)
    if man:
        _manifest_path(content).write_text(json.dumps(man, indent=1, sort_keys=True))
    else:
        shutil.rmtree(store, ignore_errors=True)
    return done
