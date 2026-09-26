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
// fields (decision 5's "Owner visibility"). Values that depend on which player is looking (lootable
// flags, quest-activated game objects, ...) still send their unconditional default until the
// viewer-dependent hooks land (bot-phase-4.md slice C6).
void BuildCreateUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target);

#endif
