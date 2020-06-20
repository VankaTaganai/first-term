#ifndef BIGINT_UINT_VECTOR_H
#define BIGINT_UINT_VECTOR_H

#include <memory>
#include <vector>
#include <cstdint>
#include "shared_pointer.h"

struct uint_vector {
    uint_vector();

    uint_vector(uint_vector const& other);

    ~uint_vector();

    uint_vector& operator=(uint_vector other);

    void push_back(uint32_t x);

    void pop_back();

    size_t size() const ;

    uint32_t const& operator[](size_t index) const;

    uint32_t& operator[](size_t index);

    uint32_t back() const;

    void swap(uint_vector &other);

private:
    bool is_small;
    bool empty;
    union {
        uint32_t value;
        shared_pointer* data;
    } number;

    void unshare();
};


#endif //BIGINT_UINT_VECTOR_H
