"""Sequence playback: assigns a sequence's action to every animated object and keeps the global loops running."""
import math

import bpy

from . import m2_import, m2_names


def _objects(top):
    return [o for o in top.all_objects if o.animation_data is not None or o.type == "ARMATURE" or o.wow_m2_object.kind]


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


def _top(context):
    return m2_import.root_collection(context.active_object)


def _has_top(cls, context):
    return _top(context) is not None


def _has_sequence(cls, context):
    top = _top(context)
    return top is not None and 0 <= top.wow_m2.active_sequence < len(top.wow_m2.sequences)


def _new_action(top, name, duration):
    act = bpy.data.actions.new(name)
    act.use_fake_user = True
    act.use_frame_range = True
    act.frame_start, act.frame_end = 0, max(duration, 1)
    return act


def _action_name(top, anim_id, variation):
    return f"{top.wow_m2.name.rsplit('.', 1)[0]}_{m2_names.anim_name(anim_id)}_{variation}"


def _drop_action(action):
    if action is not None:
        action.use_fake_user = False


class WOW_OT_m2_sequence_add(bpy.types.Operator):
    bl_idname = "wow.m2_sequence_add"
    bl_label = "Add sequence"
    bl_options = {"REGISTER", "UNDO"}

    anim_id: bpy.props.IntProperty(name="Animation ID", min=0, max=0xFFFF)
    duration: bpy.props.IntProperty(name="Duration (ms)", default=1000, min=1)

    poll = classmethod(_has_top)

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        top = _top(context)
        p = top.wow_m2
        e = p.sequences.add()
        e.anim_id, e.duration = self.anim_id, self.duration
        e.variation_index = sum(1 for s in p.sequences if s.anim_id == self.anim_id) - 1
        e.variation_next, e.alias_next = -1, len(p.sequences) - 1
        e.action = _new_action(top, _action_name(top, self.anim_id, e.variation_index), self.duration)
        p.sequence_count = len(p.sequences)
        p.active_sequence = len(p.sequences) - 1
        return {"FINISHED"}


class WOW_OT_m2_sequence_duplicate(bpy.types.Operator):
    bl_idname = "wow.m2_sequence_duplicate"
    bl_label = "Duplicate sequence"
    bl_options = {"REGISTER", "UNDO"}

    poll = classmethod(_has_sequence)

    def execute(self, context):
        p = _top(context).wow_m2
        src = p.sequences[p.active_sequence]
        e = p.sequences.add()
        for name in ("anim_id", "flags", "duration", "frequency", "blend_time_in", "blend_time_out", "movespeed", "replay_min", "replay_max",
                     "bounds_min", "bounds_max", "bounds_radius", "padding"):
            setattr(e, name, getattr(src, name))
        e.variation_index = sum(1 for s in p.sequences if s.anim_id == e.anim_id) - 1
        e.variation_next, e.alias_next = -1, len(p.sequences) - 1
        if src.action is not None:
            e.action = src.action.copy()
            e.action.name = _action_name(_top(context), e.anim_id, e.variation_index)
            e.action.use_fake_user = True
        p.sequence_count = len(p.sequences)
        p.active_sequence = len(p.sequences) - 1
        return {"FINISHED"}


class WOW_OT_m2_sequence_remove(bpy.types.Operator):
    bl_idname = "wow.m2_sequence_remove"
    bl_label = "Remove sequence"
    bl_options = {"REGISTER", "UNDO"}
    poll = classmethod(_has_sequence)

    def execute(self, context):
        p = _top(context).wow_m2
        i = p.active_sequence
        _drop_action(p.sequences[i].action)
        p.sequences.remove(i)
        p.sequence_count = len(p.sequences)
        p.active_sequence = min(i, len(p.sequences) - 1)
        return {"FINISHED"}


class WOW_OT_m2_sequence_key(bpy.types.Operator):
    bl_idname = "wow.m2_sequence_key"
    bl_label = "Key selected bones"
    bl_options = {"REGISTER", "UNDO"}
    poll = classmethod(_has_sequence)
    bl_description = "Insert location, rotation and scale keys for the selected bones at the current frame of the active sequence"

    def execute(self, context):
        p = _top(context).wow_m2
        action = p.sequences[p.active_sequence].action
        arm = context.active_object
        if action is None or arm is None or arm.type != "ARMATURE":
            return {"CANCELLED"}
        slot = _slot_for(action, arm) or action.slots.new("OBJECT", arm.name)
        ad = arm.animation_data or arm.animation_data_create()
        ad.action = action
        ad.action_slot = slot
        frame = context.scene.frame_current
        for pb in context.selected_pose_bones or ():
            for path in ("location", "rotation_quaternion", "scale"):
                pb.keyframe_insert(path, frame=frame, group=pb.name)
        return {"FINISHED"}


class WOW_OT_m2_loop_add(bpy.types.Operator):
    bl_idname = "wow.m2_loop_add"
    bl_label = "Add global loop"
    bl_options = {"REGISTER", "UNDO"}

    timestamp: bpy.props.IntProperty(name="Length (ms)", default=1000, min=1)

    poll = classmethod(_has_top)

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        top = _top(context)
        p = top.wow_m2
        e = p.loops.add()
        e.timestamp = self.timestamp
        e.action = _new_action(top, f"{top.name.rsplit('.', 1)[0]}_loop{len(p.loops) - 1}", self.timestamp)
        return {"FINISHED"}


class WOW_OT_m2_loop_remove(bpy.types.Operator):
    bl_idname = "wow.m2_loop_remove"
    bl_label = "Remove last global loop"
    bl_description = "Removes the last loop; tracks bound to it must not reference it"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        top = _top(context)
        return top is not None and len(top.wow_m2.loops) > 0

    def execute(self, context):
        p = _top(context).wow_m2
        _drop_action(p.loops[-1].action)
        p.loops.remove(len(p.loops) - 1)
        return {"FINISHED"}


classes = (WOW_OT_m2_play_sequence, WOW_OT_m2_sequence_add, WOW_OT_m2_sequence_duplicate, WOW_OT_m2_sequence_remove, WOW_OT_m2_sequence_key,
           WOW_OT_m2_loop_add, WOW_OT_m2_loop_remove)
