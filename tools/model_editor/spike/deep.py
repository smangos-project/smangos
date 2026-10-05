"""Generic structural equality over wowlib entities (reflection via dir())."""
import numbers

import numpy as np


def _eq(x, y):
    x, y = np.asarray(x), np.asarray(y)
    if x.dtype.names and x.dtype == y.dtype:
        return all(_eq(x[n], y[n]) for n in x.dtype.names)
    nan = x.dtype.kind == "f" and y.dtype.kind == "f"
    return np.array_equal(x, y, equal_nan=nan)


def _public(o):
    return [a for a in dir(o) if not a.startswith("_") and not callable(getattr(type(o), a, None))]


def deep_diff(a, b, path="", depth=0):
    """First difference as a path string, or None if equal."""
    if depth > 12:
        return None
    if isinstance(a, np.ndarray) or isinstance(b, np.ndarray):
        return None if _eq(a, b) else path
    if isinstance(a, (str, bytes, numbers.Number, bool, type(None))) or not (hasattr(a, "__dict__") or hasattr(a, "__len__") or hasattr(type(a), "__getitem__") or _public(a)):
        try:
            return None if a == b else path
        except Exception:  # noqa: BLE001
            return None
    if hasattr(a, "__len__") and hasattr(a, "__getitem__") and not isinstance(a, (str, bytes)):
        if len(a) != len(b):
            return f"{path}.len"
        try:
            na, nb = np.asarray(a), np.asarray(b)
            if na.dtype != object and na.size:
                return None if _eq(na, nb) else f"{path}[]"
        except Exception:  # noqa: BLE001
            pass
        for i in range(len(a)):
            d = deep_diff(a[i], b[i], f"{path}[{i}]", depth + 1)
            if d:
                return d
        return None
    for name in _public(a):
        try:
            va, vb = getattr(a, name), getattr(b, name)
        except Exception:  # noqa: BLE001
            continue
        d = deep_diff(va, vb, f"{path}.{name}", depth + 1)
        if d:
            return d
    return None
