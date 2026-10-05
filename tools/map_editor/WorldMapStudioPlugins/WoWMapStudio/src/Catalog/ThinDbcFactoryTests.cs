using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="ThinDbcFactory{TEntity,TRecord}"/>'s database-free surface — filter
/// classification, the create-time registry guard, and the <see cref="ThinDbcFactory{TEntity,TRecord}"/>
/// <c>ToEntity</c>/<c>Write</c> mapping pair — through a fake in-memory subclass. Search execution and
/// the create popup are DB-/UI-bound and untested, matching the other catalog factories here.</summary>
public static class ThinDbcFactoryTests
{
    private sealed class ProbeRecord
    {
        public int ID { get; set; }

        public string Name { get; set; } = string.Empty;

        public int Count { get; set; }
    }

    private sealed class ProbeEntity : CatalogEntity, IDbcCatalogEntity
    {
        public int ID { get; set; }

        public string Name { get; set; } = string.Empty;

        public int Count { get; set; }

        public bool IsSaved { get; set; }

        public override string DisplayName => $"Probe {ID}";
    }

    private sealed class Probe : ThinDbcFactory<ProbeEntity, ProbeRecord>
    {
        private readonly bool _textSearchable;

        public Probe(bool textSearchable = true) : base(null!) => _textSearchable = textSearchable;

        public override string CatalogName => "Probe";

        protected override string DisplayText(ProbeRecord record) => record.Name;

        protected override Expression<Func<ProbeRecord, bool>>? TextFilter(string term) =>
            _textSearchable ? record => record.Name.Contains(term) : null;

        protected override ProbeEntity ToEntity(ProbeRecord record) => new()
        {
            ID = record.ID,
            Name = record.Name,
            Count = record.Count,
            IsSaved = true,
        };

        protected override void Write(ProbeEntity entity, ProbeRecord record)
        {
            record.Name = entity.Name;
            record.Count = entity.Count;
        }

        protected override IEnumerable<CatalogFieldSpec> Fields(ProbeEntity e) =>
        [
            F.Text("Name", () => e.Name, v => e.Name = v),
            F.Int("Count", () => e.Count, v => e.Count = v),
        ];

        internal ProbeEntity CallToEntity(ProbeRecord record) => ToEntity(record);

        internal void CallWrite(ProbeEntity entity, ProbeRecord record) => Write(entity, record);

        internal IReadOnlyList<string?> FieldLabels(ProbeEntity entity) =>
            Fields(entity).Select(spec => spec.Label).ToList();
    }

    private sealed class KeyedProbeRecord
    {
        public int Code { get; set; }

        public string Name { get; set; } = string.Empty;
    }

    // Mirrors DbcSpellspecialuniteffectRecord's "EnumID" key: the reflected id column isn't "ID".
    private sealed class KeyedProbe : ThinDbcFactory<ProbeEntity, KeyedProbeRecord>
    {
        public KeyedProbe() : base(null!)
        {
        }

        public override string CatalogName => "Keyed Probe";

        protected override string KeyPropertyName => "Code";

        protected override ProbeEntity ToEntity(KeyedProbeRecord record) => new()
        {
            ID = record.Code,
            Name = record.Name,
            IsSaved = true,
        };

        protected override void Write(ProbeEntity entity, KeyedProbeRecord record)
        {
            record.Name = entity.Name;
        }

        protected override IEnumerable<CatalogFieldSpec> Fields(ProbeEntity e) => [];

        internal ProbeEntity CallToEntity(KeyedProbeRecord record) => ToEntity(record);
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void Blank_filter_matches_everything()
    {
        var probe = new Probe();

        Assert.AreEqual(Probe.FilterKind.All, probe.ClassifyFilter("", out _));
        Assert.AreEqual(Probe.FilterKind.All, probe.ClassifyFilter("   ", out _));
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void A_numeric_filter_is_an_id_lookup()
    {
        var probe = new Probe();

        Assert.AreEqual(Probe.FilterKind.ById, probe.ClassifyFilter("133", out int id));
        Assert.AreEqual(133, id);
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void A_text_filter_needs_a_text_predicate()
    {
        Assert.AreEqual(Probe.FilterKind.ByText, new Probe(textSearchable: true).ClassifyFilter("fire", out _));
        Assert.AreEqual(Probe.FilterKind.None, new Probe(textSearchable: false).ClassifyFilter("fire", out _),
            "a non-numeric term on an id-only catalog matches nothing rather than everything");
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void Create_guard_rejects_an_id_already_open()
    {
        var registry = new CatalogEntityRegistry();
        registry.Add(new ProbeEntity { ID = 5 });
        var probe = new Probe();

        Assert.Throws<InvalidOperationException>(() => probe.GuardCreatable(registry, 5));
        probe.GuardCreatable(registry, 6);
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void To_entity_and_write_round_trip_the_modelled_columns()
    {
        var probe = new Probe();
        var source = new ProbeRecord { ID = 7, Name = "Alpha", Count = 3 };

        ProbeEntity entity = probe.CallToEntity(source);
        Assert.AreEqual(7, entity.ID);
        Assert.AreEqual("Alpha", entity.Name);
        Assert.AreEqual(3, entity.Count);
        Assert.IsTrue(entity.IsSaved);

        entity.Name = "Beta";
        entity.Count = 9;

        var dest = new ProbeRecord { ID = 7 };
        probe.CallWrite(entity, dest);
        Assert.AreEqual("Beta", dest.Name);
        Assert.AreEqual(9, dest.Count);
    }

    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void Field_specs_carry_their_labels_in_order()
    {
        var probe = new Probe();

        IReadOnlyList<string?> labels = probe.FieldLabels(new ProbeEntity());

        Assert.AreEqual(2, labels.Count);
        Assert.AreEqual("Name", labels[0]);
        Assert.AreEqual("Count", labels[1]);
    }

    // DescribeAsync's registry-first path (an open, possibly-unsaved entity) never touches Storage, so
    // this exercises it without a live database — the same reason Probe's own constructor default is
    // storage: null. The DB fallback for a key nothing has open is DB-bound and untested here, matching
    // this file's own doc comment about what stays untested.
    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void Describe_reflects_an_unsaved_edit_on_an_open_entity()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_thin_dbc_describe_test__" });
        var entity = new ProbeEntity { ID = 7, Name = "Alpha", IsSaved = true };
        context.Catalog.Add(entity);
        entity.Name = "Beta";

        var probe = new Probe();
        IReadOnlyDictionary<string, string> described = BlockingWork.Run(() => probe.DescribeAsync(context, ["7"]));

        Assert.IsTrue(described.TryGetValue("7", out string? text));
        Assert.AreEqual("Beta", text);
    }

    // KeyPropertyName's whole job is picking which TRecord property the reflected expression builder
    // binds to; a subclass overriding it away from "ID" without throwing at construction, and still
    // reading through ToEntity, is the observable proof it worked — the queries it feeds are DB-bound
    // and untested here like the rest of this file.
    [EditorTest(Category = "WoW Spell", Thread = TestThread.Background)]
    public static void Key_property_name_override_resolves_a_non_id_column()
    {
        var probe = new KeyedProbe();
        var record = new KeyedProbeRecord { Code = 9, Name = "Gamma" };

        ProbeEntity entity = probe.CallToEntity(record);

        Assert.AreEqual(9, entity.ID);
        Assert.AreEqual("Gamma", entity.Name);
    }
}
