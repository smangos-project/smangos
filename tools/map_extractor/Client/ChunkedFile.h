#ifndef MAP_EXTRACTOR_CLIENT_CHUNKEDFILE_H
#define MAP_EXTRACTOR_CLIENT_CHUNKEDFILE_H

#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct Chunk
{
    uint8_t const* data = nullptr; // payload, after the 8 byte header
    uint32_t size = 0;
    std::vector<Chunk> subChunks;  // only filled for MCNK

    template <typename T>
    T const* As() const { return size >= sizeof(T) ? reinterpret_cast<T const*>(data) : nullptr; }

    Chunk const* Sub(char const* fourcc) const;
};

// IFF-like file: fourcc is stored reversed on disk, so "MCNK" is looked up as written.
class ChunkedFile
{
public:
    // False if the data is empty or no chunk could be read.
    bool Parse(std::vector<uint8_t> data);

    Chunk const* Get(char const* fourcc) const;
    std::vector<Chunk const*> GetAll(char const* fourcc) const;

private:
    std::vector<uint8_t> m_data;
    std::multimap<std::string, Chunk> m_chunks;
};

#endif
