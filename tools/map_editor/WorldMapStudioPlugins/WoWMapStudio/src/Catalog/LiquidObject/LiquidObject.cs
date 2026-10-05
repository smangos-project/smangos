namespace WorldMapStudio.WoW;

/// <summary>
/// A <c>dbc_liquidobject</c> row as a catalog entity — the Cataclysm indirection an MH2O instance's
/// <c>liquidObjectOrLVF</c> field (values &gt;= 42) points at, resolving to a real
/// <see cref="LiquidTypeId"/> plus flow and fishing data. Read-only reference data, loaded whole by
/// <see cref="LiquidObjectFactory"/>.
/// </summary>
public sealed class LiquidObject : CatalogEntity
{
    [ScriptProperty]
    public int LiquidObjectId { get; set; }

    [ScriptProperty]
    public int LiquidTypeId { get; set; }

    [ScriptProperty]
    public float FlowDirection { get; set; }

    [ScriptProperty]
    public float FlowSpeed { get; set; }

    [ScriptProperty]
    public bool Fishable { get; set; }

    [ScriptProperty]
    public bool Reflection { get; set; }

    public override string DisplayName => $"Liquid Object {LiquidObjectId} -> type {LiquidTypeId}";
}
