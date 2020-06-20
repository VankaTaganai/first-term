#include "uint_vector.h"

uint_vector::uint_vector() {
    is_small = false;
    empty = true;
}

uint_vector::uint_vector(const uint_vector& other) {
    is_small = other.is_small;
    empty = other.empty;
    if (is_small) {
        number.value = other.number.value;
    } else if (!empty) {
        number.data = other.number.data;
        ++number.data->ref_counter;
    }
}

uint_vector::~uint_vector() {
    if (!is_small && !empty) {
        --number.data->ref_counter;
        if (number.data->ref_counter == 0) {
            delete number.data;
        }
    }
}

uint_vector& uint_vector::operator=(uint_vector other) {
    swap(other);
    return *this;
}

void uint_vector::push_back(uint32_t x) {
    if (empty) {
        empty = false;
        is_small = true;
        number.value = x;
    } else if (is_small) {
        is_small = false;

        vector<uint32_t> tmp;
        tmp.push_back(number.value);
        tmp.push_back(x);

        number.data = new shared_pointer(tmp);
    } else {
        if (!number.data->unique()) {
            unshare();
        }
        number.data->ints.push_back(x);
    }
}

void uint_vector::pop_back() {
    if (!is_small && !empty) {
        if (number.data->ref_counter == 2) {
            uint32_t tmp = (*this)[1];
            --number.data->ref_counter;
            if (number.data->ref_counter == 0) {
                delete number.data;
            }
            number.value = tmp;
            is_small = true;
        } else {
            if (!number.data->unique()) {
                unshare();
            }
            number.data->ints.pop_back();
        }
    } else {
        empty = true;
    }
}

size_t uint_vector::size() const {
    if (empty) {
        return 0;
    } else if (is_small) {
        return 1;
    } else {
        return number.data->ints.size();
    }
}

uint32_t const& uint_vector::operator[](size_t index) const {
    return is_small ? number.value : number.data->ints[index];
}

uint32_t& uint_vector::operator[](size_t index) {
    if (is_small) {
        return number.value;
    } else {
        if (!number.data->unique()) {
            unshare();
        }
        return number.data->ints[index];
    }
}

uint32_t uint_vector::back() const {
    return is_small ? number.value : number.data->ints.back();
}

void uint_vector::swap(uint_vector &other) {
    if (empty) {
        if (other.is_small) {
            number.value = other.number.value;
        } else if (!other.empty) {
            number.data = other.number.data;
        }
    } else if (is_small) {
        if (other.empty) {
            other.number.value = number.value;
        } else if (other.is_small) {
            std::swap(number.value, other.number.value);
        } else {
            uint32_t tmp = number.value;
            number.data = other.number.data;
            other.number.value = tmp;
        }
    } else {
        if (other.empty) {
            other.number.data = number.data;
        } else if (other.is_small) {
            uint32_t tmp = other.number.value;
            other.number.data = number.data;
            number.value = tmp;
        } else {
            std::swap(number.data, other.number.data);
        }
    }
    std::swap(empty, other.empty);
    std::swap(is_small, other.is_small);
}

void uint_vector::unshare() {
    --number.data->ref_counter;
    number.data = new shared_pointer(number.data->ints);
}