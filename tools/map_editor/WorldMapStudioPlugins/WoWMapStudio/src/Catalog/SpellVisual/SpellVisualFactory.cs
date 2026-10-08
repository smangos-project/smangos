using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellvisual` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellVisualFactory : ThinDbcFactory<SpellVisual, DbcSpellvisualRecord>
{
    public SpellVisualFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Visual";

    protected override string DisplayText(DbcSpellvisualRecord record) => $"Spell Visual {record.ID}";

    protected override SpellVisual ToEntity(DbcSpellvisualRecord record) => new()
    {
        ID = record.ID,
        MissileCastOffset1 = record.MissileCastOffset1,
        MissileCastOffset2 = record.MissileCastOffset2,
        MissileCastOffset3 = record.MissileCastOffset3,
        MissileImpactOffset1 = record.MissileImpactOffset1,
        MissileImpactOffset2 = record.MissileImpactOffset2,
        MissileImpactOffset3 = record.MissileImpactOffset3,
        AnimEventSoundID = record.AnimEventSoundID,
        Flags = record.Flags,
        MissileAttachment = record.MissileAttachment,
        MissileDestinationAttachment = record.MissileDestinationAttachment,
        MissileCastPositionerID = record.MissileCastPositionerID,
        MissileImpactPositionerID = record.MissileImpactPositionerID,
        MissileTargetingKit = record.MissileTargetingKit,
        HostileSpellVisualID = record.HostileSpellVisualID,
        CasterSpellVisualID = record.CasterSpellVisualID,
        SpellVisualMissileSetID = record.SpellVisualMissileSetID,
        DamageNumberDelay = record.DamageNumberDelay,
        LowViolenceSpellVisualID = record.LowViolenceSpellVisualID,
        RaidSpellVisualMissileSetID = record.RaidSpellVisualMissileSetID,
        ReducedUnexpectedCameraMovementSpellVisualID = record.ReducedUnexpectedCameraMovementSpellVisualID,
        AreaModel = record.AreaModel,
        HasMissile = record.HasMissile,
        IsSaved = true,
    };

    protected override void Write(SpellVisual entity, DbcSpellvisualRecord record)
    {
        record.MissileCastOffset1 = entity.MissileCastOffset1;
        record.MissileCastOffset2 = entity.MissileCastOffset2;
        record.MissileCastOffset3 = entity.MissileCastOffset3;
        record.MissileImpactOffset1 = entity.MissileImpactOffset1;
        record.MissileImpactOffset2 = entity.MissileImpactOffset2;
        record.MissileImpactOffset3 = entity.MissileImpactOffset3;
        record.AnimEventSoundID = entity.AnimEventSoundID;
        record.Flags = entity.Flags;
        record.MissileAttachment = entity.MissileAttachment;
        record.MissileDestinationAttachment = entity.MissileDestinationAttachment;
        record.MissileCastPositionerID = entity.MissileCastPositionerID;
        record.MissileImpactPositionerID = entity.MissileImpactPositionerID;
        record.MissileTargetingKit = entity.MissileTargetingKit;
        record.HostileSpellVisualID = entity.HostileSpellVisualID;
        record.CasterSpellVisualID = entity.CasterSpellVisualID;
        record.SpellVisualMissileSetID = entity.SpellVisualMissileSetID;
        record.DamageNumberDelay = entity.DamageNumberDelay;
        record.LowViolenceSpellVisualID = entity.LowViolenceSpellVisualID;
        record.RaidSpellVisualMissileSetID = entity.RaidSpellVisualMissileSetID;
        record.ReducedUnexpectedCameraMovementSpellVisualID = entity.ReducedUnexpectedCameraMovementSpellVisualID;
        record.AreaModel = entity.AreaModel;
        record.HasMissile = entity.HasMissile;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellVisual e) =>
    [
        F.Float("Missile Cast Offset1", () => e.MissileCastOffset1, v => e.MissileCastOffset1 = v),
        F.Float("Missile Cast Offset2", () => e.MissileCastOffset2, v => e.MissileCastOffset2 = v),
        F.Float("Missile Cast Offset3", () => e.MissileCastOffset3, v => e.MissileCastOffset3 = v),
        F.Float("Missile Impact Offset1", () => e.MissileImpactOffset1, v => e.MissileImpactOffset1 = v),
        F.Float("Missile Impact Offset2", () => e.MissileImpactOffset2, v => e.MissileImpactOffset2 = v),
        F.Float("Missile Impact Offset3", () => e.MissileImpactOffset3, v => e.MissileImpactOffset3 = v),
        F.Int("Anim Event Sound ID", () => e.AnimEventSoundID, v => e.AnimEventSoundID = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
        F.Int("Missile Attachment", () => e.MissileAttachment, v => e.MissileAttachment = v),
        F.Int("Missile Destination Attachment", () => e.MissileDestinationAttachment, v => e.MissileDestinationAttachment = v),
        F.Int("Missile Cast Positioner ID", () => e.MissileCastPositionerID, v => e.MissileCastPositionerID = v),
        F.Int("Missile Impact Positioner ID", () => e.MissileImpactPositionerID, v => e.MissileImpactPositionerID = v),
        F.Int("Missile Targeting Kit", () => e.MissileTargetingKit, v => e.MissileTargetingKit = v),
        F.Link("Hostile Spell Visual ID", () => e.HostileSpellVisualID, v => e.HostileSpellVisualID = v, "Spell Visual"),
        F.Link("Caster Spell Visual ID", () => e.CasterSpellVisualID, v => e.CasterSpellVisualID = v, "Spell Visual"),
        F.Int("Spell Visual Missile Set ID", () => e.SpellVisualMissileSetID, v => e.SpellVisualMissileSetID = v),
        F.Int("Damage Number Delay", () => e.DamageNumberDelay, v => e.DamageNumberDelay = v),
        F.Link("Low Violence Spell Visual ID", () => e.LowViolenceSpellVisualID, v => e.LowViolenceSpellVisualID = v, "Spell Visual"),
        F.Int("Raid Spell Visual Missile Set ID", () => e.RaidSpellVisualMissileSetID, v => e.RaidSpellVisualMissileSetID = v),
        F.Int("Reduced Unexpected Camera Movement Spell Visual ID", () => e.ReducedUnexpectedCameraMovementSpellVisualID, v => e.ReducedUnexpectedCameraMovementSpellVisualID = v),
        F.Int("Area Model", () => e.AreaModel, v => e.AreaModel = v),
        F.Int("Has Missile", () => e.HasMissile, v => e.HasMissile = v),
    ];
}
