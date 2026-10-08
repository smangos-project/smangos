using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`broadcast_text` as a browsable catalog, searchable by id or text.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class BroadcastTextFactory : ThinDbcFactory<BroadcastText, BroadcastTextRecord>
{
    public BroadcastTextFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Broadcast Text";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(BroadcastTextRecord record) => record.MaleText ?? record.FemaleText ?? string.Empty;

    protected override Expression<Func<BroadcastTextRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.MaleText, $"%{term}%") || EF.Functions.Like(record.FemaleText, $"%{term}%");

    protected override BroadcastText ToEntity(BroadcastTextRecord record) => new()
    {
        ID = record.Entry,
        MaleText = record.MaleText ?? string.Empty,
        FemaleText = record.FemaleText ?? string.Empty,
        ChatType = record.ChatType,
        SoundId = record.SoundId,
        LanguageId = record.LanguageId,
        EmoteId1 = record.EmoteId1,
        EmoteId2 = record.EmoteId2,
        EmoteId3 = record.EmoteId3,
        EmoteDelay1 = record.EmoteDelay1,
        EmoteDelay2 = record.EmoteDelay2,
        EmoteDelay3 = record.EmoteDelay3,
        IsSaved = true,
    };

    protected override void Write(BroadcastText entity, BroadcastTextRecord record)
    {
        record.Entry = entity.ID;
        record.MaleText = entity.MaleText.Length == 0 ? null : entity.MaleText;
        record.FemaleText = entity.FemaleText.Length == 0 ? null : entity.FemaleText;
        record.ChatType = entity.ChatType;
        record.SoundId = entity.SoundId;
        record.LanguageId = entity.LanguageId;
        record.EmoteId1 = entity.EmoteId1;
        record.EmoteId2 = entity.EmoteId2;
        record.EmoteId3 = entity.EmoteId3;
        record.EmoteDelay1 = entity.EmoteDelay1;
        record.EmoteDelay2 = entity.EmoteDelay2;
        record.EmoteDelay3 = entity.EmoteDelay3;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(BroadcastText e) =>
    [
        F.TextMultiline("Male Text", () => e.MaleText, v => e.MaleText = v),
        F.TextMultiline("Female Text", () => e.FemaleText, v => e.FemaleText = v),
        F.Int("Chat Type", () => e.ChatType, v => e.ChatType = v),
        F.Int("Sound Id", () => e.SoundId, v => e.SoundId = v),
        F.Int("Language Id", () => e.LanguageId, v => e.LanguageId = v),
        F.Link("Emote 1", () => e.EmoteId1, v => e.EmoteId1 = v, "Emote"),
        F.Int("Emote Delay 1", () => e.EmoteDelay1, v => e.EmoteDelay1 = v),
        F.Link("Emote 2", () => e.EmoteId2, v => e.EmoteId2 = v, "Emote"),
        F.Int("Emote Delay 2", () => e.EmoteDelay2, v => e.EmoteDelay2 = v),
        F.Link("Emote 3", () => e.EmoteId3, v => e.EmoteId3 = v, "Emote"),
        F.Int("Emote Delay 3", () => e.EmoteDelay3, v => e.EmoteDelay3 = v),
    ];

    internal BroadcastText CallToEntity(BroadcastTextRecord record) => ToEntity(record);

    internal void CallWrite(BroadcastText entity, BroadcastTextRecord record) => Write(entity, record);
}

/// <summary>`npc_text` as a browsable catalog; each slot links to a broadcast text.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class NpcTextFactory : ThinDbcFactory<NpcText, NpcTextRecord>
{
    public NpcTextFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Npc Text";

    protected override NpcText ToEntity(NpcTextRecord r)
    {
        var e = new NpcText
        {
            ID = r.ID,
            IsSaved = true,
        };
        int[] ids = [r.BroadcastTextID0, r.BroadcastTextID1, r.BroadcastTextID2, r.BroadcastTextID3,
            r.BroadcastTextID4, r.BroadcastTextID5, r.BroadcastTextID6, r.BroadcastTextID7];
        float[] probs = [r.Probability0, r.Probability1, r.Probability2, r.Probability3,
            r.Probability4, r.Probability5, r.Probability6, r.Probability7];
        ids.CopyTo(e.BroadcastTextIds, 0);
        probs.CopyTo(e.Probabilities, 0);
        return e;
    }

    protected override void Write(NpcText e, NpcTextRecord r)
    {
        r.ID = e.ID;
        r.BroadcastTextID0 = e.BroadcastTextIds[0];
        r.BroadcastTextID1 = e.BroadcastTextIds[1];
        r.BroadcastTextID2 = e.BroadcastTextIds[2];
        r.BroadcastTextID3 = e.BroadcastTextIds[3];
        r.BroadcastTextID4 = e.BroadcastTextIds[4];
        r.BroadcastTextID5 = e.BroadcastTextIds[5];
        r.BroadcastTextID6 = e.BroadcastTextIds[6];
        r.BroadcastTextID7 = e.BroadcastTextIds[7];
        r.Probability0 = e.Probabilities[0];
        r.Probability1 = e.Probabilities[1];
        r.Probability2 = e.Probabilities[2];
        r.Probability3 = e.Probabilities[3];
        r.Probability4 = e.Probabilities[4];
        r.Probability5 = e.Probabilities[5];
        r.Probability6 = e.Probabilities[6];
        r.Probability7 = e.Probabilities[7];
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(NpcText e)
    {
        for (int i = 0; i < NpcText.Slots; i++)
        {
            int slot = i;
            yield return F.Link($"Broadcast Text {slot}", () => e.BroadcastTextIds[slot], v => e.BroadcastTextIds[slot] = v, "Broadcast Text");
            yield return F.Float($"Probability {slot}", () => e.Probabilities[slot], v => e.Probabilities[slot] = v);
        }
    }

    internal NpcText CallToEntity(NpcTextRecord record) => ToEntity(record);

    internal void CallWrite(NpcText entity, NpcTextRecord record) => Write(entity, record);
}
