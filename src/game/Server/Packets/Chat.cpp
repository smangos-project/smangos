#include "Chat.h"

// --- Server Packets ---

size_t WorldPackets::Chat::ChatWrongFaction::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Chat::ChatWrongFaction::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Chat::ChatRestricted::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Chat::ChatRestricted::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Chat::ChatPlayerNotFound::EstimateFinalSize() const
{
    return name.size() + sizeof(char) /*null terminator*/;
}

void WorldPackets::Chat::ChatPlayerNotFound::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << name;
}
