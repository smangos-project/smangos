using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_emotes` as a browsable catalog. Id or `EmoteSlashCommand` substring search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class EmoteFactory : ThinDbcFactory<Emote, DbcEmotesRecord>
{
    private const uint SlashCommandMaxLength = 100;

    public EmoteFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Emote";

    protected override string DisplayText(DbcEmotesRecord record) => record.EmoteSlashCommand;

    protected override Expression<Func<DbcEmotesRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.EmoteSlashCommand, $"%{term}%");

    protected override Emote ToEntity(DbcEmotesRecord record) => new()
    {
        ID = record.ID,
        SlashCommand = record.EmoteSlashCommand,
        AnimID = record.AnimID,
        EmoteFlags = record.EmoteFlags,
        IsSaved = true,
    };

    protected override void Write(Emote entity, DbcEmotesRecord record)
    {
        record.EmoteSlashCommand = entity.SlashCommand;
        record.AnimID = entity.AnimID;
        record.EmoteFlags = entity.EmoteFlags;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(Emote e) =>
    [
        F.Text("Slash Command", () => e.SlashCommand, v => e.SlashCommand = v, SlashCommandMaxLength),
        F.Int("Anim ID", () => e.AnimID, v => e.AnimID = v),
        F.Int("Emote Flags", () => e.EmoteFlags, v => e.EmoteFlags = v),
    ];
}
