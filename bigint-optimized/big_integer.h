#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <gmp.h>
#include <iosfwd>
#include <cstdint>
#include "uint_vector.h"

struct big_integer {
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    big_integer(uint32_t a);
    explicit big_integer(std::string const& str);
    ~big_integer();

    big_integer& operator=(big_integer const& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);

private:
    bool negative;
    uint_vector num;

    size_t length() const;
    big_integer abs() const;
    void expand(size_t len);
    big_integer binpow2(size_t n);
    uint32_t get_byte(size_t i) const;
    void shrink();
    friend uint32_t trial(__uint128_t a, __uint128_t b, __uint128_t c, __uint128_t d, __uint128_t e);
    friend bool smaller(big_integer const &r, big_integer const &dq, size_t k, size_t m);
    friend big_integer quotient(big_integer const& y, uint32_t k);
    void difference(big_integer &r, big_integer const &dq, size_t k, size_t m);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif // BIG_INTEGER_H
