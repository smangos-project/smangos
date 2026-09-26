#ifndef MANGOS_PACKETS_QUEST_H
#define MANGOS_PACKETS_QUEST_H

#include "Packet.h"
#include "ObjectGuid.h"
#include "SharedDefines.h"
#include <string>
#include <vector>

class Quest;

// The rest of the vanilla quest wire is ported to Packets/QuestMessages.h; what's left here has no
// modern binding yet (failure-reason toasts, and the standalone quest-query-by-id lookup).
namespace WorldPackets { namespace Quest
{
    class QueryQuest final : public ClientPacket
    {
    public:
        uint32 questEntry; // db entry id

        explicit QueryQuest() : ClientPacket(CMSG_QUEST_QUERY), questEntry(0) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    // --- Server Packets ---

    class QuestGiverQuestFailed final : public ServerPacket
    {
    public:
        uint32 questId = 0;
        uint32 reason = 0;

        explicit QuestGiverQuestFailed() : ServerPacket(SMSG_QUESTGIVER_QUEST_FAILED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class QuestUpdateFailed final : public ServerPacket
    {
    public:
        uint32 questId = 0;

        explicit QuestUpdateFailed() : ServerPacket(SMSG_QUESTUPDATE_FAILED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class QuestUpdateFailedTimer final : public ServerPacket
    {
    public:
        uint32 questId = 0;

        explicit QuestUpdateFailedTimer() : ServerPacket(SMSG_QUESTUPDATE_FAILEDTIMER) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class QuestGiverQuestInvalid final : public ServerPacket
    {
    public:
        uint32 msg = 0;

        explicit QuestGiverQuestInvalid() : ServerPacket(SMSG_QUESTGIVER_QUEST_INVALID) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class QuestQueryResponse final : public ServerPacket
    {
    public:
        int sessionDbLocaleIndex = -1;
        ::Quest const* quest = nullptr;

        explicit QuestQueryResponse() : ServerPacket(SMSG_QUEST_QUERY_RESPONSE) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Quest

#endif // MANGOS_PACKETS_QUEST_H
