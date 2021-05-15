#include "datastream.h"

namespace {
bool isLittleEndian()
{
    const union {
        uint32_t value;
        char bytes[4];
    } bits = { 0x12345678 };
    return bits.bytes[0] == 0x78;
}

bool needSwap()
{
    return !isLittleEndian();
}

uint16_t byteSwap16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

uint32_t byteSwap32(uint32_t value)
{
    return (byteSwap16(value) << 16) | byteSwap16(value >> 16);
}
} // namespace

DataStream::DataStream(const char *path)
    : m_in(fopen(path, "rb"))
    , m_error(m_in == nullptr)
    , m_needSwap(needSwap())
{
}

size_t DataStream::readBytes(char *buf, std::size_t size)
{
    if (m_error)
        return 0;
    const auto readResult = fread(buf, 1, size, m_in);
    if (readResult != size)
        m_error = true;
    return readResult;
}

DataStream &DataStream::operator>>(int8_t &value)
{
    readBytes(reinterpret_cast<char *>(&value), 1);
    return *this;
}

DataStream &DataStream::operator>>(int16_t &value)
{
    if (readBytes(reinterpret_cast<char *>(&value), 2) == 2) {
        if (m_needSwap)
            value = byteSwap16(value);
    }
    return *this;
}

DataStream &DataStream::operator>>(int32_t &value)
{
    if (readBytes(reinterpret_cast<char *>(&value), 4) == 4) {
        if (m_needSwap)
            value = byteSwap32(value);
    }
    return *this;
}
