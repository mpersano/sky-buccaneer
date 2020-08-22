#pragma once

#include <type_traits>

enum class InputState : unsigned {
    None = 0,
    Left = 1 << 0,
    Right = 1 << 1,
    Up = 1 << 2,
    Down = 1 << 3,
    Forward = 1 << 4,
    Reverse = 1 << 5,
};

constexpr InputState operator&(InputState x, InputState y)
{
    using UT = typename std::underlying_type_t<InputState>;
    return static_cast<InputState>(static_cast<UT>(x) & static_cast<UT>(y));
}

constexpr InputState operator|(InputState x, InputState y)
{
    using UT = typename std::underlying_type_t<InputState>;
    return static_cast<InputState>(static_cast<UT>(x) | static_cast<UT>(y));
}

constexpr InputState operator^(InputState x, InputState y)
{
    using UT = typename std::underlying_type_t<InputState>;
    return static_cast<InputState>(static_cast<UT>(x) ^ static_cast<UT>(y));
}

constexpr InputState operator~(InputState x)
{
    using UT = typename std::underlying_type_t<InputState>;
    return static_cast<InputState>(~static_cast<UT>(x));
}

inline InputState &operator&=(InputState &x, InputState y)
{
    return x = x & y;
}

inline InputState &operator|=(InputState &x, InputState y)
{
    return x = x | y;
}

inline InputState &operator^=(InputState &x, InputState y)
{
    return x = x ^ y;
}
