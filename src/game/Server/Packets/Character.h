#ifndef MANGOS_PACKETS_CHARACTER_H
#define MANGOS_PACKETS_CHARACTER_H

#include "Packet.h"
#include "ObjectGuid.h"
#include "SharedDefines.h"

#include <string>
#include <vector>

namespace WorldPackets { namespace Character
{
    static constexpr uint8 CHAR_ENUM_EQUIPMENT_SLOTS = 20; // INVENTORY_SLOT_BAG_START + 1

    struct CharEnumEquipmentSlot
    {
        uint32 displayInfoId = 0;
        uint8  inventoryType = 0;
    };

    struct CharEnumData
    {
        ObjectGuid guid;
        std::string name;
        uint8  race = 0;
        uint8  class_ = 0;
        uint8  gender = 0;
        uint8  skin = 0;
        uint8  face = 0;
        uint8  hairStyle = 0;
        uint8  hairColor = 0;
        uint8  facialHair = 0;
        uint8  level = 0;
        uint32 zone = 0;
        uint32 map = 0;
        float  x = 0.f;
        float  y = 0.f;
        float  z = 0.f;
        uint32 guildId = 0;
        uint32 charFlags = 0;
        uint8  firstLogin = 0;
        uint32 petDisplayId = 0;
        uint32 petLevel = 0;
        uint32 petFamily = 0;
        CharEnumEquipmentSlot equipment[CHAR_ENUM_EQUIPMENT_SLOTS] = {};
    };

    class CharRename final : public ClientPacket
    {
    public:
        ObjectGuid  guid;
        std::string newname;

        explicit CharRename() : ClientPacket(CMSG_CHAR_RENAME) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };
    // --- Server Packets ---

    class CharRenameResponse final : public ServerPacket
    {
    public:
        uint8 result = 0;
        ObjectGuid renamedCharacterGuid;
        std::string newName;

        CharRenameResponse() : ServerPacket(SMSG_CHAR_RENAME) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Character

#endif // MANGOS_PACKETS_CHARACTER_H
