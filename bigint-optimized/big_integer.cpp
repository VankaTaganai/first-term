#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>

big_integer::big_integer() {
    negative = false;
    num = uint_vector();
}

big_integer::big_integer(big_integer const& other) {
    negative = other.negative;
    num = other.num;
}

big_integer::big_integer(int a) {
    negative = a < 0;
    num = uint_vector();
    num.push_back(a);
    shrink();
}

big_integer::big_integer(uint32_t a) {
    negative = false;
    num = uint_vector();
    num.push_back(a);
    shrink();
}

big_integer::big_integer(std::string const& str) {
    bool number = str.empty() ? false : (str[0] == '-' && str.size() > 1) || ('0' <= str[0] && str[0] <= '9');
    for (size_t i = 1; i < str.size(); i++) {
        number = ('0' <= str[i] && str[i] <= '9');
    }
    if (!number) {
        throw std::runtime_error("Incorrect string format");
    }

    negative = false;
    (*this) = big_integer();
    for (char symbol : str) {
        if (symbol == '-') {
            continue;
        }
        (*this) = (*this) * 10 + (symbol - '0');
    }
    if (str[0] == '-') {
        (*this) = -(*this);
    }
    shrink();
}

big_integer::~big_integer() = default;

// swap trick
big_integer& big_integer::operator=(big_integer const& other) {
    if (this == &other) {
        return *this;
    }

    big_integer tmp(other);
    std::swap(negative, tmp.negative);
    num.swap(tmp.num);

    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    uint64_t carry = 0;
    expand(std::max(length(), rhs.length()) + 1);
    for (size_t i = 0; i < length(); i++) {
        carry = carry + get_byte(i) + rhs.get_byte(i);
        num[i] = uint32_t(carry);
        carry >>= 32u;
    }

    negative = (num.back() >> 31u);
    shrink();
    return *this;
}


big_integer& big_integer::operator-=(big_integer const& rhs) {
    return (*this += (-rhs));
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    if (*this == 0 || rhs == 0) {
        *this = 0;
        return *this;
    }
    bool sg = negative ^ rhs.negative;
    big_integer a = abs();
    big_integer b = rhs.abs();

    *this = 0;
    expand(a.length() + b.length() + 1);
    for (size_t i = 0; i < b.length(); i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < a.length() || carry != 0; j++) {
            size_t index = i + j;
            carry += (uint64_t)b.num[i] * (j < a.length() ? a.num[j] : 0) + num[index];
            num[index] = (uint32_t)carry;
            carry >>= 32u;
        }
    }

    shrink();
    if (sg) {
        (*this) = -(*this);
    }

    return *this;
}

uint32_t trial(__uint128_t a, __uint128_t b, __uint128_t c, __uint128_t d, __uint128_t e) {
    __uint128_t x = (((a << 32u) + b) << 32u) + c;
    __uint128_t y = (d << 32u) + e;
    return std::min((uint32_t)(x / y), UINT32_MAX);
}

bool smaller(big_integer const &r, big_integer const &dq, size_t k, size_t m) {
    for (size_t i = 0; i <= m; i++) {
        if (r.get_byte(m + k - i) != dq.get_byte(m - i)) {
            return r.get_byte(m + k - i) < dq.get_byte(m - i);
        }
    }
    return false;
}

void big_integer::difference(big_integer &r, const big_integer &dq, size_t k, size_t m) {
    uint32_t borrow = 0;
    size_t start = k;
    for (size_t i = 0; i <= m; i++) {
        uint64_t diff = ((int64_t) r.get_byte(start + i) - dq.get_byte(i) - borrow);
        borrow = (r.get_byte(start + i) < dq.get_byte(i) + borrow);
        r.num[start + i] = (uint32_t) diff;
    }
}

big_integer quotient(big_integer const& y, uint32_t k) {
    uint64_t carry = 0;
    big_integer x;
    x.expand(y.length());
    for (int32_t i = y.length() - 1; i >= 0; i--) {
        uint64_t tmp = (carry << 32u) + y.get_byte(i);
        x.num[i] = (uint32_t)(tmp / k);
        carry = tmp % k;
    }
    x.shrink();
    return x;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    bool sign = negative ^ rhs.negative;
    big_integer divs = abs();
    big_integer divr = rhs.abs();

    if (divs < divr) {
        *this = 0;
        return *this;
    }

    if (divr.length() == 1) {
        *this = quotient(divs, divr.get_byte(0));
        if (sign) {
            *this = -(*this);
        }
        return *this;
    }

    big_integer dq;
    uint32_t f = (uint64_t(UINT32_MAX) + 1) / (uint64_t(divr.num.back()) + 1);
    divs *= f;
    divr *= f;

    divs.num.push_back(0);
    size_t n = divs.length();
    size_t m = divr.length();

    *this = 0;
    expand(n - m);

    for (int32_t k = n - m - 1; k >= 0; k--) {
        uint32_t qt = trial(divs.num[m + k], divs.num[m + k - 1],
                divs.num[m + k - 2], divr.num[m - 1], divr.num[m - 2]);
        dq = divr * qt;
        if (smaller(divs, dq, k, m)) {
            qt--;
            dq -= divr;
        }
        num[k] = qt;
        difference(divs, dq, k, m);
    }

    shrink();
    if (sign) {
        *this = -(*this);
    }

    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer tmp = *this;
    *this -= ((tmp /= rhs) * rhs);
    return *this;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    negative &= rhs.negative;
    expand(rhs.length());

    for (size_t i = 0; i < length(); i++) {
        num[i] &= rhs.get_byte(i);
    }
    shrink();
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    negative |= rhs.negative;
    expand(rhs.length());

    for (size_t i = 0; i < length(); i++) {
        num[i] |= rhs.get_byte(i);
    }
    shrink();
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    negative ^= rhs.negative;
    expand(rhs.length());

    for (size_t i = 0; i < length(); i++) {
        num[i] ^= rhs.get_byte(i);
    }
    shrink();
    return *this;
}

big_integer big_integer::binpow2(size_t n) {
    if (n == 0) {
        return 1;
    }
    if (n % 2 == 0) {
        big_integer tmp = binpow2(n / 2);
        return tmp * tmp;
    } else {
        return binpow2(n - 1) * 2;
    }
}

big_integer& big_integer::operator<<=(int rhs) {
    (*this) *= binpow2(rhs);
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)  {
    bool sign = negative;
    (*this) /= binpow2(rhs);
    if (sign) {
        (*this)--;
    }
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    if ((*this) == 0) {
        return *this;
    }
    big_integer r;
    r = (~(*this)) + 1;
    r.shrink();
    return r;
}

big_integer big_integer::operator~() const {
    big_integer r;
    r.negative = !negative;
    for (size_t i = 0; i < length(); i++) {
        r.num.push_back(~num[i]);
    }
    r.shrink();
    return r;
}

big_integer& big_integer::operator++() {
    (*this) += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    (*this) -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b) {
    big_integer x = a;
    big_integer y = b;
    x.shrink();
    y.shrink();
    if (x.negative != y.negative || x.length() != y.length()) {
        return false;
    }
    for (size_t i = 0; i < x.length(); i++) {
        if (x.num[i] != y.num[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.negative != b.negative) {
        return a.negative;
    }
    if (a.length() < b.length()) {
        return !a.negative;
    }
    if (a.length() > b.length()) {
        return a.negative;
    }

    for (int i = (int)a.length() - 1; i >= 0; i--) {
        if (a.num[i] < b.num[i]) {
            return !a.negative;
        }
        if (a.num[i] > b.num[i]) {
            return a.negative;
        }
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b) {
    return (b < a);
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return (a == b || a < b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return (b <= a);
}


std::string to_string(big_integer const& a) {
    if (a.length() == 0) {
        return (a.negative ? "-1" : "0");
    }
    big_integer tmp = a.abs();
    std::string res;
    while (tmp != 0) {
        res.push_back(char((tmp % 10).get_byte(0) + '0'));
        tmp /= 10;
    }

    if (a.negative) {
        res.push_back('-');
    }

    reverse(res.begin(), res.end());
    return res;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}

size_t big_integer::length() const {
    return num.size();
}

big_integer big_integer::abs() const {
    if (!negative) {
        return (*this);
    }
    return (-(*this));
}

void big_integer::expand(size_t len) {
    uint32_t addition = (negative ? UINT32_MAX : 0);
    while (length() < len) {
        num.push_back(addition);
    }
}

void big_integer::shrink() {
    while (length() > 0 && num.back() == 0 && !negative) {
        num.pop_back();
    }
    while (length() > 0 && num.back() == UINT32_MAX && negative) {
        num.pop_back();
    }
}

uint32_t big_integer::get_byte(size_t i) const {
    if (i < length()) {
        return num[i];
    }
    if (negative) {
        return UINT32_MAX;
    }
    return 0;
}
