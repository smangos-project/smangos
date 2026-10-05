namespace WorldMapStudio.WoW;

/// <summary>A `dbc_currencytypes` row — the currency a quest's `RewardCurrencyId1..4` /
/// `RequiredCurrencyId1..4` point at.</summary>
public sealed class CurrencyType : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int CategoryID { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Currency Type {ID}: {Name}";
}
