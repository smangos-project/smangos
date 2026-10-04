#include "Client/ChunkedFile.h"

namespace
{
    std::string ReadFourcc(uint8_t const* p)
    {
        return { char(p[3]), char(p[2]), char(p[1]), char(p[0]) };
    }

    // MCNK header is 128 bytes, sub chunks follow it.
    constexpr uint32_t MCNK_HEADER_SIZE = 128;
}

Chunk const* Chunk::Sub(char const* fourcc) const
{
    // sub chunks are stored with their fourcc in `data - 8`; see Parse
    for (Chunk const& c : subChunks)
    {
        uint8_t const* hdr = c.data - 8;
        if (ReadFourcc(hdr) == fourcc)
            return &c;
    }
    return nullptr;
}

bool ChunkedFile::Parse(std::vector<uint8_t> data)
{
    m_data = std::move(data);
    m_chunks.clear();

    uint64_t pos = 0;
    while (pos + 8 <= m_data.size())
    {
        uint8_t const* hdr = m_data.data() + pos;
        uint32_t size;
        std::memcpy(&size, hdr + 4, 4);
        std::string fourcc = ReadFourcc(hdr);
        uint64_t end = pos + 8 + size;
        if (end > m_data.size())
            size = uint32_t(m_data.size() - pos - 8), end = m_data.size();

        Chunk chunk;
        chunk.data = hdr + 8;
        chunk.size = size;
        if (fourcc == "MCNK" && size > MCNK_HEADER_SIZE)
        {
            uint64_t sub = pos + 8 + MCNK_HEADER_SIZE;
            while (sub + 8 <= end)
            {
                uint8_t const* shdr = m_data.data() + sub;
                uint32_t ssize;
                std::memcpy(&ssize, shdr + 4, 4);
                uint64_t send = sub + 8 + ssize;
                if (send > end)
                    ssize = uint32_t(end - sub - 8), send = end;
                Chunk s;
                s.data = shdr + 8;
                s.size = ssize;
                chunk.subChunks.push_back(std::move(s));
                sub = send;
            }
        }
        m_chunks.emplace(std::move(fourcc), std::move(chunk));
        pos = end;
    }
    return !m_chunks.empty();
}

Chunk const* ChunkedFile::Get(char const* fourcc) const
{
    auto itr = m_chunks.find(fourcc);
    return itr == m_chunks.end() ? nullptr : &itr->second;
}

std::vector<Chunk const*> ChunkedFile::GetAll(char const* fourcc) const
{
    std::vector<Chunk const*> result;
    auto range = m_chunks.equal_range(fourcc);
    for (auto itr = range.first; itr != range.second; ++itr)
        result.push_back(&itr->second);
    return result;
}
