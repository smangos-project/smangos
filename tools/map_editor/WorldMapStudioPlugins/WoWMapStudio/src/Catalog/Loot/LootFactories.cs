using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`creature_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class CreatureLootFactory : LootFactory<CreatureLootTemplateRecord>
{
    public CreatureLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Loot";

    protected override string Kind => "Creature";
}

/// <summary>`gameobject_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GameobjectLootFactory : LootFactory<GameobjectLootTemplateRecord>
{
    public GameobjectLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gameobject Loot";

    protected override string Kind => "Gameobject";
}

/// <summary>`reference_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class ReferenceLootFactory : LootFactory<ReferenceLootTemplateRecord>
{
    public ReferenceLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Reference Loot";

    protected override string Kind => "Reference";
}

/// <summary>`item_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class ItemLootFactory : LootFactory<ItemLootTemplateRecord>
{
    public ItemLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Item Loot";

    protected override string Kind => "Item";
}

/// <summary>`skinning_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SkinningLootFactory : LootFactory<SkinningLootTemplateRecord>
{
    public SkinningLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Skinning Loot";

    protected override string Kind => "Skinning";
}

/// <summary>`pickpocketing_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class PickpocketingLootFactory : LootFactory<PickpocketingLootTemplateRecord>
{
    public PickpocketingLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Pickpocketing Loot";

    protected override string Kind => "Pickpocketing";
}

/// <summary>`fishing_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class FishingLootFactory : LootFactory<FishingLootTemplateRecord>
{
    public FishingLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Fishing Loot";

    protected override string Kind => "Fishing";
}

/// <summary>`disenchant_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class DisenchantLootFactory : LootFactory<DisenchantLootTemplateRecord>
{
    public DisenchantLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Disenchant Loot";

    protected override string Kind => "Disenchant";
}

/// <summary>`mail_loot_template` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class MailLootFactory : LootFactory<MailLootTemplateRecord>
{
    public MailLootFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Mail Loot";

    protected override string Kind => "Mail";
}
