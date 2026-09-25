#ifndef MANGOS_PACKETS_CHAT_H
#define MANGOS_PACKETS_CHAT_H

#include "Packet.h"
#include "SharedDefines.h"

namespace WorldPackets { namespace Chat
{
    // Normalized fields every CMSG_CHAT_MESSAGE_* opcode (Packets/ChatMessages.h) is unpacked into
    // before reaching WorldSession::HandleChatMessageOpcode, whose routing logic keys off `type`
    // and never needed to change when the wire opcodes did -- only how these get populated did.
    struct ChatMessage
    {
        uint32 type = 0;
        uint32 lang = 0;
        std::string whisperTargetOrChannel;
        std::string message;
    };
    // --- Server Packets ---

    class ChatWrongFaction final : public ServerPacket
    {
    public:
        explicit ChatWrongFaction() : ServerPacket(SMSG_CHAT_WRONG_FACTION) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_8_4
    class ChatRestricted final : public ServerPacket
    {
    public:
        explicit ChatRestricted() : ServerPacket(SMSG_CHAT_RESTRICTED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };
#endif

    class ChatPlayerNotFound final : public ServerPacket
    {
    public:
        std::string name;

        explicit ChatPlayerNotFound() : ServerPacket(SMSG_CHAT_PLAYER_NOT_FOUND) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Chat

#endif // MANGOS_PACKETS_CHAT_H
