using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_itemsparse` as a browsable catalog: id or name search, the headline fields editable in place.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class ItemFactory : ThinDbcFactory<Item, DbcItemsparseRecord>
{
    public ItemFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Item";

    protected override string DisplayText(DbcItemsparseRecord record) => record.DisplayLang;

    protected override Expression<Func<DbcItemsparseRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.DisplayLang, $"%{term}%");

    protected override Item ToEntity(DbcItemsparseRecord record) => new()
    {
        ID = record.ID,
        Name = record.DisplayLang,
        Description = record.DescriptionLang,
        Quality = record.OverallQualityID,
        ItemLevel = record.ItemLevel,
        RequiredLevel = record.RequiredLevel,
        InventoryType = record.InventoryType,
        Bonding = record.Bonding,
        BuyPrice = record.BuyPrice,
        SellPrice = record.SellPrice,
        StartQuestID = record.StartQuestID,
        IsSaved = true,
    };

    protected override void Write(Item entity, DbcItemsparseRecord record)
    {
        record.DisplayLang = entity.Name;
        record.DescriptionLang = entity.Description;
        record.OverallQualityID = entity.Quality;
        record.ItemLevel = entity.ItemLevel;
        record.RequiredLevel = entity.RequiredLevel;
        record.InventoryType = entity.InventoryType;
        record.Bonding = entity.Bonding;
        record.BuyPrice = entity.BuyPrice;
        record.SellPrice = entity.SellPrice;
        record.StartQuestID = entity.StartQuestID;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(Item e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, 255),
        F.Text("Description", () => e.Description, v => e.Description = v, 255),
        F.Int("Quality", () => e.Quality, v => e.Quality = v),
        F.Int("Item Level", () => e.ItemLevel, v => e.ItemLevel = v),
        F.Int("Required Level", () => e.RequiredLevel, v => e.RequiredLevel = v),
        F.Int("Inventory Type", () => e.InventoryType, v => e.InventoryType = v),
        F.Int("Bonding", () => e.Bonding, v => e.Bonding = v),
        F.Int("Buy Price", () => e.BuyPrice, v => e.BuyPrice = v),
        F.Int("Sell Price", () => e.SellPrice, v => e.SellPrice = v),
        F.Link("Starts Quest", () => e.StartQuestID, v => e.StartQuestID = v, "Quest"),
    ];

    internal Item CallToEntity(DbcItemsparseRecord record) => ToEntity(record);

    internal void CallWrite(Item entity, DbcItemsparseRecord record) => Write(entity, record);
}
