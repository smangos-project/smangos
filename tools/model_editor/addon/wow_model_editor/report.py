"""Findings collected by an export; shared by the WMO and M2 exporters."""


class Report:
    def __init__(self, **stats):
        self.items = []
        self.material_index = {}
        self.new_groups = {}
        self.stats = dict(stats)

    def add(self, severity, where, message):
        self.items.append((severity, where, message))

    @property
    def errors(self):
        return [i for i in self.items if i[0] == "ERROR"]

    def __str__(self):
        return "\n".join(f"{s} {w}: {m}" for s, w, m in self.items)
