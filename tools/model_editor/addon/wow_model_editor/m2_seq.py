"""Sequence playback: assigns a sequence's action to every animated object and keeps the global loops running."""
import math

import bpy

from . import m2_import


def _objects(top):
    return [o for o in top.all_objects if o.animation_data is not None or o.type == "ARMATURE"]


def _slot_for(action, ob):
    return next((s for s in action.slots if s.identifier == "OB" + ob.name), None)


def assign(ob, action):
    ad = ob.animation_data or ob.animation_data_create()
    if action is None:
        ad.action = None
        return
    slot = _slot_for(action, ob)
    ad.action = action if slot else None
    if slot:
        ad.action_slot = slot


def _loop_track(ob):
    ad = ob.animation_data or ob.animation_data_create()
    track = ad.nla_tracks.get("wow_loops")
    if track is not None:
        ad.nla_tracks.remove(track)
    return ad.nla_tracks.new()


def rebuild_loops(top, length):
    """Repeating NLA strips for every global loop action; `length` = frames to cover."""
    tracks = {}
    for k, loop in enumerate(top.wow_m2.loops):
        if loop.action is None or loop.timestamp <= 0:
            continue
        for ob in _objects(top):
            slot = _slot_for(loop.action, ob)
            if slot is None:
                continue
            tr = tracks.get(ob.name)
            if tr is None:
                tr = tracks[ob.name] = _loop_track(ob)
                tr.name = "wow_loops"
            strip = tr.strips.new(f"loop{k}", 0, loop.action)
            strip.action_slot = slot
            strip.repeat = max(1.0, math.ceil(length / loop.timestamp) + 1)


def play(top, index, start=True):
    """Makes sequence `index` active and sets the scene range to its duration."""
    seqs = top.wow_m2.sequences
    if not 0 <= index < len(seqs):
        return False
    seq = seqs[index]
    for ob in _objects(top):
        assign(ob, seq.action)
    scene = bpy.context.scene
    scene.frame_start, scene.frame_end = 0, max(int(seq.duration), 1)
    scene.frame_set(0)
    rebuild_loops(top, scene.frame_end)
    return True


def on_active(props):
    for c in bpy.data.collections:
        if c.wow_m2.as_pointer() == props.as_pointer():
            play(c, props.active_sequence)
            return


class WOW_OT_m2_play_sequence(bpy.types.Operator):
    bl_idname = "wow.m2_play_sequence"
    bl_label = "Play sequence"

    index: bpy.props.IntProperty(default=-1)

    @classmethod
    def poll(cls, context):
        return m2_import.root_collection(context.active_object) is not None

    def execute(self, context):
        top = m2_import.root_collection(context.active_object)
        i = self.index if self.index >= 0 else top.wow_m2.active_sequence
        if not play(top, i):
            return {"CANCELLED"}
        if not context.screen.is_animation_playing:
            bpy.ops.screen.animation_play()
        return {"FINISHED"}


classes = (WOW_OT_m2_play_sequence,)
