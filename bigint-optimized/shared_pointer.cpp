#include "shared_pointer.h"

shared_pointer::shared_pointer() {
    ref_counter = 1;
    ints = vector<uint32_t>();
}

shared_pointer::shared_pointer(vector<uint32_t> const& vec) {
    ref_counter = 1;
    ints = vec;
}

bool shared_pointer::unique() const {
    return ref_counter == 1;
}