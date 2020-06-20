#ifndef BIGINT_SHARED_POINTER_H
#define BIGINT_SHARED_POINTER_H

#include <cstddef>
#include <cstdint>
#include "vector.h"

struct shared_pointer {
    shared_pointer();

    explicit shared_pointer(vector<uint32_t> const& vec);

    bool unique() const;

    size_t ref_counter;
    vector<uint32_t> ints;
};


#endif //BIGINT_SHARED_POINTER_H
