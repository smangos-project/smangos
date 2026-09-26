#ifndef UPDATES_UPDATEOBJECTWRITER_H
#define UPDATES_UPDATEOBJECTWRITER_H

class ByteBuffer;
class Object;
class Player;
class UpdateData;

// Serializes a create block for `object` into `data`, the way the modern client wants it:
// [update type][packed guid128][object type][movement frame][size]{[field flags][entity
// fragments][indirect-fragment byte][per-struct WriteCreate payloads]}. `target` is the receiving
// player -- when it is `object` itself, the block includes ActivePlayerData and the owner-only
// fields (decision 5's "Owner visibility"). Every other object type (game objects, corpses, other
// players' fields visible to a bystander, ...) still writes an empty ObjectData/UnitData struct
// until its fill function and viewer gating land (bot-phase-4.md slice C5).
void BuildCreateUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target);

#endif
