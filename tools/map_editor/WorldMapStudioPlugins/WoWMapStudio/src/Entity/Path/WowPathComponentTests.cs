using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A throwaway <see cref="WowPathComponent{TNode}"/> for exercising the base class's order
/// reconciliation without a real database-backed path type.</summary>
internal sealed class TestPathComponent : WowPathComponent<int>
{
    public override string TypeId => "test-path";

    public override string DisplayName => "Test Path";

    protected override int DefaultNode => 0;

    protected override Color MeshColor => Colors.White;

    public override SceneComponent Clone()
    {
        var clone = new TestPathComponent();
        CopyStateInto(clone);
        return clone;
    }
}

public static class WowPathComponentTests
{
    private static TestPathComponent NetworkOf(out int a, out int b, out int c)
    {
        var component = new TestPathComponent();
        var network = new VertexNetwork();
        a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        b = network.AddVertex(new Vector3(1.0f, 0.0f, 0.0f));
        c = network.AddVertex(new Vector3(2.0f, 0.0f, 0.0f));
        network.AddEdge(a, b);
        network.AddEdge(b, c);
        component.ReplaceNetwork(network);
        component.SetOrder(a, b);
        return component;
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void New_vertices_get_the_default_node_and_old_metadata_survives_a_replace()
    {
        TestPathComponent component = NetworkOf(out int a, out _, out int c);
        component.SetNodeData(a, 42);

        VertexNetwork grown = component.Network.Clone();
        int d = grown.AddVertex(new Vector3(3.0f, 0.0f, 0.0f));
        grown.AddEdge(c, d);
        component.ReplaceNetwork(grown);

        Assert.AreEqual(42, component.NodeData(a), "existing metadata must survive a network replace");
        Assert.AreEqual(0, component.NodeData(d), "a brand-new vertex gets DefaultNode");
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Both_anchors_present_just_rewalks()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out int c);

        Assert.IsTrue(component.OrderedNodes.Select(n => n.VertexId).SequenceEqual(new[] { a, b, c }));
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Deleting_the_start_vertex_reanchors_to_the_nearest_endpoint()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out int c);

        VertexNetwork edited = component.Network.Clone();
        edited.RemoveVertex(a);
        component.ReplaceNetwork(edited);

        Assert.AreEqual(b, component.StartVertexId);
        Assert.AreEqual(c, component.SecondVertexId);
        Assert.IsTrue(component.OrderedNodes.Select(n => n.VertexId).SequenceEqual(new[] { b, c }));
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Deleting_the_second_vertex_reanchors_to_a_neighbour_of_start()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out int c);

        VertexNetwork edited = component.Network.Clone();
        edited.RemoveVertex(b);
        edited.AddEdge(a, c);
        component.ReplaceNetwork(edited);

        Assert.AreEqual(a, component.StartVertexId);
        Assert.AreEqual(c, component.SecondVertexId);
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Closing_the_chain_into_a_loop_keeps_the_same_anchors()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out int c);

        VertexNetwork closed = component.Network.Clone();
        closed.AddEdge(c, a);
        component.ReplaceNetwork(closed);

        Assert.AreEqual(a, component.StartVertexId);
        Assert.AreEqual(b, component.SecondVertexId);
        Assert.IsTrue(component.OrderedNodes.Select(n => n.VertexId).SequenceEqual(new[] { a, b, c }));
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Fewer_than_two_vertices_clears_both_anchors()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out _);

        VertexNetwork edited = new VertexNetwork();
        edited.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        component.ReplaceNetwork(edited);

        Assert.AreEqual(0, component.StartVertexId);
        Assert.AreEqual(0, component.SecondVertexId);
        Assert.AreEqual(0, component.OrderedNodes.Count);
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void A_branch_leaves_ordered_nodes_empty_rather_than_guessing()
    {
        TestPathComponent component = NetworkOf(out int a, out int b, out int c);

        VertexNetwork branched = component.Network.Clone();
        int d = branched.AddVertex(new Vector3(1.0f, 0.0f, 1.0f));
        branched.AddEdge(b, d);
        component.ReplaceNetwork(branched);

        Assert.AreEqual(0, component.OrderedNodes.Count);
        Assert.AreEqual(1, component.ValidateTopology("Test Path").Count);
    }

    [EditorTest(Category = "Path", Thread = TestThread.Background)]
    public static void Clone_deep_copies_network_and_metadata()
    {
        TestPathComponent component = NetworkOf(out int a, out _, out _);
        component.SetNodeData(a, 7);

        var clone = (TestPathComponent)component.Clone();
        clone.SetNodeData(a, 9);

        Assert.AreEqual(7, component.NodeData(a));
        Assert.AreEqual(9, clone.NodeData(a));
        Assert.AreEqual(component.StartVertexId, clone.StartVertexId);
        Assert.AreEqual(component.SecondVertexId, clone.SecondVertexId);
    }
}
