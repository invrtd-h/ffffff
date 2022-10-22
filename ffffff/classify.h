#ifndef UNDERSCORE_CPP_CLASSIFY_H
#define UNDERSCORE_CPP_CLASSIFY_H

#include <concepts>

namespace fff {
    
    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;
}

namespace fff {
    
    template<arithmetic T>
    class Wrap {
        T data;
    public:
        constexpr explicit Wrap(const T &data) : data(data) {}
        constexpr Wrap &operator=(const T &data_) {
            data = data_;
            return *this;
        }
        
        constexpr operator T() const noexcept {
            return data;
        }
        constexpr operator bool() const noexcept {
            return static_cast<bool>(data);
        }
    
        template<arithmetic U>
        constexpr auto operator<=>(const U &r) const noexcept {
            return data <=> r;
        }
    
        template<arithmetic U>
        constexpr auto operator+(const U &r) const noexcept {
            return Wrap<decltype(data + r)>(data + r);
        }
        template<arithmetic U>
        constexpr auto operator-(const U &r) const noexcept {
            return Wrap<decltype(data - r)>(data - r);
        }
        template<arithmetic U>
        constexpr auto operator*(const U &r) const noexcept {
            return Wrap<decltype(data * r)>(data * r);
        }
        template<arithmetic U>
        constexpr auto operator/(const U &r) const noexcept {
            return Wrap<decltype(data / r)>(data / r);
        }
        template<arithmetic U>
        constexpr auto operator%(const U &r) const noexcept {
            return Wrap<decltype(data % r)>(data % r);
        }
        template<arithmetic U>
        constexpr auto operator&(const U &r) const noexcept {
            return Wrap<decltype(data & r)>(data & r);
        }
        template<arithmetic U>
        constexpr auto operator|(const U &r) const noexcept {
            return Wrap<decltype(data | r)>(data | r);
        }
        template<arithmetic U>
        constexpr auto operator^(const U &r) const noexcept {
            return Wrap<decltype(data ^ r)>(data ^ r);
        }
        template<std::integral U>
        constexpr auto operator<<(const U &r) const noexcept {
            return Wrap<decltype(data << r)>(data << r);
        }
        template<std::integral U>
        constexpr auto operator>>(const U &r) const noexcept {
            return Wrap<decltype(data >> r)>(data >> r);
        }
    
        template<arithmetic U>
        constexpr auto &operator+=(const U &r) noexcept {
            data += r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator-=(const U &r) noexcept {
            data -= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator*=(const U &r) noexcept {
            data *= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator/=(const U &r) noexcept {
            data /= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator%=(const U &r) noexcept {
            data %= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator&=(const U &r) noexcept {
            data &= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator|=(const U &r) noexcept {
            data |= r; return *this;
        }
        template<arithmetic U>
        constexpr auto &operator^=(const U &r) noexcept {
            data ^= r; return *this;
        }
    
        template<std::integral U>
        constexpr auto &operator<<=(const U &r) noexcept {
            data <<= r; return *this;
        }
        template<std::integral U>
        constexpr auto &operator>>=(const U &r) noexcept {
            data >>= r; return *this;
        }
        
        constexpr auto &operator++() noexcept {
            ++data; return *this;
        }
        constexpr auto &operator--() noexcept {
            --data; return *this;
        }
        constexpr auto operator++(int) noexcept {
            auto temp = *this;
            ++data; return temp;
        }
        constexpr auto operator--(int) noexcept {
            auto temp = *this;
            --data; return temp;
        }
    };
}


#endif //UNDERSCORE_CPP_CLASSIFY_H
