#include "Common.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "Packets/Session.h"

void WorldSession::HandleClassicLogoutRequestOpcode(ClassicLogoutRequest const& /*packet*/)
{
    if (ObjectGuid lootGuid = GetPlayer()->GetLootGuid())
        DoLootRelease(lootGuid);

    uint8 reason = 0;

    if (GetPlayer()->IsInCombat())
        reason = 1;
    else if (GetPlayer()->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)))
        reason = 3;                      // is jumping or falling
    else if (GetPlayer()->HasAura(9454)) // frozen by GM via freeze command
        reason = 2;                      // FIXME - Need the correct value

    if (reason)
    {
        SendClassic(LogoutResponse{ int32(reason), false });
        LogoutRequest(0);
        return;
    }

    // instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ||
        GetPlayer()->IsTaxiFlying() ||
        GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))
    {
        SendClassic(LogoutResponse{ 0, true });
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        if (GetPlayer()->GetStandState() == UNIT_STAND_STATE_STAND &&
           !GetPlayer()->IsMounted() &&
           !(GetPlayer()->GetUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_SPLINE_ENABLED)))
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        GetPlayer()->SetRooted(true);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    GetPlayer()->ApplyModByteFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_FLAGS, PLAYER_FIELD_BYTE_LOGGING_OUT, true);

    SendClassic(LogoutResponse{ 0, false });
    LogoutRequest(time(nullptr));
}

void WorldSession::HandleClassicTimeSyncResponseOpcode(TimeSyncResponse const& packet)
{
    auto it = m_pendingTimeSyncRequests.find(packet.SequenceIndex);
    if (it == m_pendingTimeSyncRequests.end())
        return;

    uint32 const roundTripMs = WorldTimer::getMSTimeDiff(it->second, WorldTimer::getMSTime());
    m_pendingTimeSyncRequests.erase(it);

    // Half the round trip approximates the one-way delay to the client, so adding it to ClientTime
    // gives the server's own clock at the moment the client actually sent the response.
    m_timeSyncClockDeltaMs = int32(WorldTimer::getMSTime()) - int32(packet.ClientTime + roundTripMs / 2);
}
