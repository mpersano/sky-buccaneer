#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class DataStream
{
public:
    explicit DataStream(const char *path);

    size_t readBytes(char *buf, std::size_t size);

    DataStream &operator>>(char &value);
    DataStream &operator>>(int8_t &value);
    DataStream &operator>>(uint8_t &value);
    DataStream &operator>>(int16_t &value);
    DataStream &operator>>(uint16_t &value);
    DataStream &operator>>(int32_t &value);
    DataStream &operator>>(uint32_t &value);
    DataStream &operator>>(float &value);

    operator bool() const { return !m_error; }

private:
    FILE *m_in;
    bool m_error;
};

inline DataStream &DataStream::operator>>(char &value)
{
    return *this >> reinterpret_cast<int8_t &>(value);
}

inline DataStream &DataStream::operator>>(uint8_t &value)
{
    return *this >> reinterpret_cast<int8_t &>(value);
}

inline DataStream &DataStream::operator>>(uint16_t &value)
{
    return *this >> reinterpret_cast<int16_t &>(value);
}

inline DataStream &DataStream::operator>>(uint32_t &value)
{
    return *this >> reinterpret_cast<int32_t &>(value);
}

inline DataStream &DataStream::operator>>(float &value)
{
    return *this >> reinterpret_cast<int32_t &>(value);
}

namespace detail {

template<typename SizeT, typename Container>
DataStream &readContainer(DataStream &ds, Container &c)
{
    c.clear();
    if (ds) {
        SizeT n;
        ds >> n;
        if (ds) {
            c.reserve(n);
            for (SizeT i = 0; i < n; ++i) {
                typename Container::value_type t;
                ds >> t;
                if (!ds) {
                    c.clear();
                    break;
                }
                c.push_back(t);
            }
        }
    }
    return ds;
}

} // namespace detail

template<typename T>
DataStream &operator>>(DataStream &ds, std::vector<T> &v)
{
    return detail::readContainer<uint32_t>(ds, v);
}

template<typename T>
DataStream &operator>>(DataStream &ds, std::basic_string<T> &s)
{
    return detail::readContainer<uint8_t>(ds, s);
}

template<typename T, glm::qualifier Q>
DataStream &operator>>(DataStream &ds, glm::vec<2, T, Q> &v)
{
    ds >> v.x;
    ds >> v.y;
    return ds;
}

template<typename T, glm::qualifier Q>
DataStream &operator>>(DataStream &ds, glm::vec<3, T, Q> &v)
{
    ds >> v.x;
    ds >> v.y;
    ds >> v.z;
    return ds;
}

template<typename T, glm::qualifier Q>
DataStream &operator>>(DataStream &ds, glm::vec<4, T, Q> &v)
{
    ds >> v.x;
    ds >> v.y;
    ds >> v.z;
    ds >> v.w;
    return ds;
}

template<typename T, glm::qualifier Q>
DataStream &operator>>(DataStream &ds, glm::qua<T, Q> &q)
{
    ds >> q.x;
    ds >> q.y;
    ds >> q.z;
    ds >> q.w;
    return ds;
}
