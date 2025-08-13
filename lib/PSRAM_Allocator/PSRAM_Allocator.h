#pragma once

#include <cstdlib>
#include "esp_heap_caps.h"

/**
 * @brief A C++ STL allocator that uses PSRAM.
 *
 * This allocator can be used with STL containers like std::map and std::string
 * to force them to allocate their memory from the external PSRAM, freeing up
 * precious internal RAM for the heap.
 */
template <class T>
struct PSRAM_Allocator {
    typedef T value_type;

    PSRAM_Allocator() = default;
    template <class U> constexpr PSRAM_Allocator(const PSRAM_Allocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        if (n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();
        if (auto p = static_cast<T*>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM))) return p;
        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }
};

template <class T, class U>
bool operator==(const PSRAM_Allocator<T>&, const PSRAM_Allocator<U>&) { return true; }

template <class T, class U>
bool operator!=(const PSRAM_Allocator<T>&, const PSRAM_Allocator<U>&) { return false; }