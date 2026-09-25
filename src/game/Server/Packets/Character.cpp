#include "Character.h"
#include "SharedDefines.h"

void WorldPackets::Character::CharRename::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
    recv_data >> newname;
}

size_t WorldPackets::Character::CharRenameResponse::EstimateFinalSize() const
{
    if (result != RESPONSE_SUCCESS)
        return sizeof(result);

    return sizeof(result) +
           sizeof(renamedCharacterGuid) +
           newName.size() + sizeof(char) /*null terminator*/;
}

void WorldPackets::Character::CharRenameResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << result;
    if (result == RESPONSE_SUCCESS)
    {
        buffer << renamedCharacterGuid;
        buffer << newName;
    }
}

