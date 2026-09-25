#ifndef MANGOS_PACKETS_CLASSICCLIENT_H
#define MANGOS_PACKETS_CLASSICCLIENT_H

#include "Packet.h"

// Adapts a Classic wire struct (T::Opcode, T::Read) to the handler table's ClientPacket, by
// composition rather than inheritance: T's own members (Opcode, Read, Write, ...) never mix with
// Packet's (opcode, GetOpcode, SetOpcode).
template<typename T>
class ClassicClient final : public ClientPacket
{
public:
    ClassicClient() : ClientPacket(uint32(T::Opcode)) {}

    void ReadFromWorldPacket(WorldPacket& data) override { m_valid = Body.Read(data); }
    bool IsValid() const { return m_valid; }

    T Body;

private:
    bool m_valid = false;
};

#endif // MANGOS_PACKETS_CLASSICCLIENT_H
