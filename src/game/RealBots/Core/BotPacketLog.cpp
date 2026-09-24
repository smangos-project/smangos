#include "BotPacketLog.h"
#include "ClassicOpcodes.h"
#include <cstdio>
#include <filesystem>

BotPacketLog::BotPacketLog(std::string const& dir, std::string const& botName)
{
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    m_file.open(dir + "/" + botName + ".log", std::ios::out | std::ios::app);
}

void BotPacketLog::Write(uint32 timeMs, BotPacketDir dir, BotConnectionKind connection, uint32 opcode, ByteBuffer const& data)
{
    if (!m_file.is_open())
        return;

    char header[112];
    std::snprintf(header, sizeof(header), "[%10u] %-3s %-8s %-28s 0x%06X size=%-6zu ",
        timeMs, dir == BotPacketDir::Incoming ? "IN" : "OUT", connection == BotConnectionKind::Realm ? "realm" : "instance",
        ClassicOpcodeName(opcode), opcode, data.size());
    m_file << header;

    uint8 const* bytes = data.contents();
    char hex[4];
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::snprintf(hex, sizeof(hex), "%02X ", bytes[i]);
        m_file << hex;
    }
    m_file << "\n";
    m_file.flush();
}
