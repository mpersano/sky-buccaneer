#include "datastream.h"

DataStream::DataStream(const char *path)
    : m_is(path, std::ios::binary)
{
}

void DataStream::readBytes(char *buf, std::size_t size)
{
    m_is.read(buf, size);
    // TODO error checking
}

namespace {

template<typename T>
void readPrimitive(DataStream &ds, T &value)
{
    ds.readBytes(reinterpret_cast<char *>(&value), sizeof(T));
    // TODO error checking, endianness conversion
}

} // namespace

DataStream &DataStream::operator>>(int8_t &value)
{
    readPrimitive(*this, value);
    return *this;
}

DataStream &DataStream::operator>>(int16_t &value)
{
    readPrimitive(*this, value);
    return *this;
}

DataStream &DataStream::operator>>(int32_t &value)
{
    readPrimitive(*this, value);
    return *this;
}
