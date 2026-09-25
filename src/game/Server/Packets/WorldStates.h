#ifndef MANGOS_PACKETS_WORLDSTATES_H
#define MANGOS_PACKETS_WORLDSTATES_H

// Zone-wide state the client shows: world state variables and weather. Light like
// Packets/Session.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include <vector>

struct InitWorldStates
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_INIT_WORLD_STATES;

    struct State
    {
        int32 VariableId = 0;
        int32 Value = 0;
    };

    int32 MapId = 0;
    int32 AreaId = 0;    // the zone
    int32 SubareaId = 0; // the area inside it
    std::vector<State> States;

    void Write(ByteBuffer& data) const
    {
        data << MapId << AreaId << SubareaId;
        data << uint32(States.size());
        for (State const& state : States)
            data << state.VariableId << state.Value;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) * 3 + sizeof(uint32))
            return false;
        uint32 count;
        data >> MapId >> AreaId >> SubareaId >> count;
        if ((data.size() - data.rpos()) / (sizeof(int32) * 2) < count)
            return false;
        States.resize(count);
        for (State& state : States)
            data >> state.VariableId >> state.Value;
        return true;
    }
};

// WeatherId is a weather state (fine, light rain, heavy snow, ...), not the server's weather type.
struct WeatherUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_WEATHER;

    uint32 WeatherId = 0;
    float Intensity = 0.0f;
    bool Abrupt = false;

    void Write(ByteBuffer& data) const
    {
        data << WeatherId << Intensity;
        data.WriteBit(Abrupt);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(float) + 1)
            return false;
        data >> WeatherId >> Intensity;
        Abrupt = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

#endif
