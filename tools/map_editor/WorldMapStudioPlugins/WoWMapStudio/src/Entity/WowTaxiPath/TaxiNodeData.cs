namespace WorldMapStudio.WoW;

/// <summary>Per-node metadata for a <see cref="WowTaxiPathComponent"/> vertex — one `dbc_taxipathnode`
/// row's non-geometric columns. <see cref="ContinentId"/> is carried per node (matching the DBC's own
/// per-node <c>ContinentID</c>) even though a single <see cref="WowTaxiPath"/> run is all one
/// continent by construction.</summary>
public readonly record struct TaxiNodeData(int Flags, int Delay, int ArrivalEventId, int DepartureEventId, int ContinentId)
{
    public static readonly TaxiNodeData Default = new(0, 0, 0, 0, 0);
}
