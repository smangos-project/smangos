#include "Misc.h"

#include "SpellEntry.h"

void WorldPackets::Misc::WorldTeleport::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> timeMs;
    recv_data >> location.mapId;
    recv_data >> location.x;
    recv_data >> location.y;
    recv_data >> location.z;
    recv_data >> location.o;
}

void WorldPackets::Misc::SetTarget::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Misc::ZoneUpdate::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> newZone;
}

void WorldPackets::Misc::ChatIgnored::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Misc::MeetingStoneJoin::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Misc::TeleportToUnit::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> playerName;
}

void WorldPackets::Misc::Who::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> levelMin;
    recv_data >> levelMax;
    recv_data >> playerName;
    recv_data >> guildName;
    recv_data >> raceMask;
    recv_data >> classMask;

    uint32 zonesCount;
    recv_data >> zonesCount;
    for (uint32 i = 0; i < zonesCount; ++i)
    {
        uint32 zone;
        recv_data >> zone;
        zoneIds.push_back(zone);
    }

    uint32 strCount;
    recv_data >> strCount;
    for (uint32 i = 0; i < strCount; ++i)
    {
        std::string term;
        recv_data >> term;
        searchTerms.push_back(std::move(term));
    }
}

void WorldPackets::Misc::Bug::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> suggestion;
    recv_data.read_skip<uint32>(); // contentLen
    recv_data >> content;
    recv_data.read_skip<uint32>(); // typeLen
    recv_data >> type;
}

// --- Server Packets ---

size_t WorldPackets::Misc::LogoutComplete::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Misc::LogoutComplete::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Misc::LogoutCancelAck::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Misc::LogoutCancelAck::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Misc::PlayTimeWarning::EstimateFinalSize() const
{
    return sizeof(flag) +
           sizeof(timeLeftInSeconds);
}
void WorldPackets::Misc::PlayTimeWarning::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << flag;
    buffer << timeLeftInSeconds;
}

size_t WorldPackets::Misc::LogoutResponse::EstimateFinalSize() const
{
    return sizeof(reason) +
           sizeof(instant);
}

void WorldPackets::Misc::LogoutResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << reason;
    buffer << instant;
}


size_t WorldPackets::Misc::ServerMessage::EstimateFinalSize() const
{
    return sizeof(messageType) +
           text.size() + sizeof(char) /*null terminator*/;
}

void WorldPackets::Misc::ServerMessage::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << messageType;
    buffer << text;
}

size_t WorldPackets::Misc::MeetingstoneJoinFailed::EstimateFinalSize() const
{
    return sizeof(reason);
}

void WorldPackets::Misc::MeetingstoneJoinFailed::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << reason;
}

size_t WorldPackets::Misc::MeetingstoneSetQueue::EstimateFinalSize() const
{
    return
           sizeof(areaId) +
           sizeof(status);
}

void WorldPackets::Misc::MeetingstoneSetQueue::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << areaId;
    buffer << status;
}

size_t WorldPackets::Misc::MeetingstoneMemberAdded::EstimateFinalSize() const
{
    return sizeof(playerGuid);
}

void WorldPackets::Misc::MeetingstoneMemberAdded::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << playerGuid;
}

size_t WorldPackets::Misc::MeetingstoneInProgress::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Misc::MeetingstoneInProgress::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Misc::MeetingstoneComplete::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Misc::MeetingstoneComplete::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Misc::SetForcedReactions::EstimateFinalSize() const
{
    return sizeof(uint32) /*size*/ +
           forcedReactions.size() * sizeof(ForcedReactionEntry);
}

void WorldPackets::Misc::SetForcedReactions::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << static_cast<uint32>(forcedReactions.size());
    for (const auto& reaction : forcedReactions)
    {
        buffer << reaction.factionId;       // faction_id (Faction.dbc)
        buffer << reaction.reputationRank;  // reputation rank
    }
}

size_t WorldPackets::Misc::SetFactionVisible::EstimateFinalSize() const
{
    return sizeof(reputationListId);
}

void WorldPackets::Misc::SetFactionVisible::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << reputationListId;
}

size_t WorldPackets::Misc::PlayMusic::EstimateFinalSize() const
{
    return sizeof(musicId);
}

void WorldPackets::Misc::PlayMusic::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << musicId;
}

size_t WorldPackets::Misc::PlaySound::EstimateFinalSize() const
{
    return sizeof(soundId);
}

void WorldPackets::Misc::PlaySound::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << soundId;
}

size_t WorldPackets::Misc::Notification::EstimateFinalSize() const
{
    return message.size() + sizeof(char) /*null terminator*/;
}

void WorldPackets::Misc::Notification::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << message;
}

size_t WorldPackets::Misc::InvalidatePlayer::EstimateFinalSize() const
{
    return sizeof(playerGuid);
}
void WorldPackets::Misc::InvalidatePlayer::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << playerGuid;
}

size_t WorldPackets::Misc::ZoneUnderAttack::EstimateFinalSize() const
{
    return sizeof(areaId);
}

void WorldPackets::Misc::ZoneUnderAttack::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << areaId;
}

size_t WorldPackets::Misc::PlayObjectSound::EstimateFinalSize() const
{
    return sizeof(soundId) +
           sizeof(sourceGuid);
}

void WorldPackets::Misc::PlayObjectSound::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << soundId;
    buffer << sourceGuid;
}

size_t WorldPackets::Misc::TriggerCinematic::EstimateFinalSize() const
{
    return sizeof(cinematicSequenceId);
}

void WorldPackets::Misc::TriggerCinematic::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << cinematicSequenceId;
}

size_t WorldPackets::Misc::FactionAtWarChange::EstimateFinalSize() const
{
    return sizeof(reputationId) +
           sizeof(flags);
}
void WorldPackets::Misc::FactionAtWarChange::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << reputationId;
    buffer << flags;
}

size_t WorldPackets::Misc::InstanceReset::EstimateFinalSize() const
{
    return sizeof(mapId);
}
void WorldPackets::Misc::InstanceReset::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << mapId;
}

size_t WorldPackets::Misc::InstanceResetFailed::EstimateFinalSize() const
{
    return sizeof(reason) +
           sizeof(mapId);
}
void WorldPackets::Misc::InstanceResetFailed::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << reason;
    buffer << mapId;
}

size_t WorldPackets::Misc::RaidGroupOnly::EstimateFinalSize() const
{
    return sizeof(timer) +
           sizeof(errorCode);
}

void WorldPackets::Misc::RaidGroupOnly::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << timer;
    buffer << errorCode;
}

size_t WorldPackets::Misc::PlayerBound::EstimateFinalSize() const
{
    return sizeof(binderGuid) +
           sizeof(areaId);
}

void WorldPackets::Misc::PlayerBound::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << binderGuid;
    buffer << areaId;
}


size_t WorldPackets::Misc::RaidInstanceMessage::EstimateFinalSize() const
{
    return sizeof(messageType) +
           sizeof(mapId) +
           sizeof(resetTime);
}
void WorldPackets::Misc::RaidInstanceMessage::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << messageType;
    buffer << mapId;
    buffer << resetTime;
}

size_t WorldPackets::Misc::UpdateInstanceOwnership::EstimateFinalSize() const
{
    return sizeof(hasBeenSaved);
}
void WorldPackets::Misc::UpdateInstanceOwnership::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << hasBeenSaved;
}

size_t WorldPackets::Misc::UpdateLastInstance::EstimateFinalSize() const
{
    return sizeof(mapId);
}
void WorldPackets::Misc::UpdateLastInstance::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << mapId;
}

size_t WorldPackets::Misc::UpdateWorldState::EstimateFinalSize() const
{
    return sizeof(field) +
           sizeof(value);
}

void WorldPackets::Misc::UpdateWorldState::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << field;
    buffer << value;
}
