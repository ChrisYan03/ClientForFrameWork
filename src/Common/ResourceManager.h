#pragma once

#include <memory>
#include <functional>
#include <stdexcept>

// Forward declaration for STB image functions
extern "C" {
    void stbi_image_free(void* retval_from_stbi_load);
}

namespace ClientForFrame {

/**
 * @brief Custom deleter for array types
 */
template<typename T>
struct ArrayDeleter {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

/**
 * @brief Custom deleter for STB image data
 */
struct StbImageDeleter {
    void operator()(unsigned char* ptr) const {
        if (ptr) {
            stbi_image_free(ptr);
        }
    }
};

/**
 * @brief RAII resource management template
 *
 * Provides automatic resource management using custom deleters.
 * This ensures proper cleanup even when exceptions occur.
 *
 * @tparam T Resource type
 * @tparam Deleter Custom deleter type
 */
template<typename T, typename Deleter = std::default_delete<T>>
class ManagedResource {
public:
    using pointer = T*;
    using deleter_type = Deleter;

    // Default constructor
    ManagedResource() noexcept : ptr_(nullptr) {}

    // Constructor with pointer
    explicit ManagedResource(pointer p) noexcept : ptr_(p) {}

    // Constructor with pointer and custom deleter
    ManagedResource(pointer p, const Deleter& d) noexcept : ptr_(p), deleter_(d) {}

    // Move constructor
    ManagedResource(ManagedResource&& other) noexcept
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    // Move assignment
    ManagedResource& operator=(ManagedResource&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            deleter_ = std::move(other.deleter_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Delete copy operations
    ManagedResource(const ManagedResource&) = delete;
    ManagedResource& operator=(const ManagedResource&) = delete;

    // Destructor
    ~ManagedResource() {
        reset();
    }

    // Release ownership of the pointer
    pointer release() noexcept {
        pointer tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    // Reset the pointer (deletes current if any)
    void reset(pointer p = nullptr) noexcept {
        if (ptr_ != p) {
            if (ptr_) {
                deleter_(ptr_);
            }
            ptr_ = p;
        }
    }

    // Get the raw pointer
    pointer get() const noexcept {
        return ptr_;
    }

    // Check if pointer is not null
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    // Dereference operators
    T& operator*() const {
        if (!ptr_) {
            throw std::runtime_error("Attempting to dereference null pointer");
        }
        return *ptr_;
    }

    pointer operator->() const {
        if (!ptr_) {
            throw std::runtime_error("Attempting to dereference null pointer");
        }
        return ptr_;
    }

    // Swap
    void swap(ManagedResource& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

private:
    pointer ptr_;
    Deleter deleter_;
};

// Type aliases for common use cases
using StbImagePtr = ManagedResource<unsigned char, StbImageDeleter>;

template<typename T>
using ArrayPtr = ManagedResource<T, ArrayDeleter<T>>;

/**
 * @brief Factory function to create managed array resources
 *
 * @tparam T Element type
 * @param size Number of elements to allocate
 * @return ArrayPtr<T> Managed array pointer
 */
template<typename T>
ArrayPtr<T> make_array(size_t size) {
    return ArrayPtr<T>(new T[size]());
}

/**
 * @brief Factory function to create managed array resources with uninitialized memory
 *
 * @tparam T Element type
 * @param size Number of elements to allocate
 * @return ArrayPtr<T> Managed array pointer
 */
template<typename T>
ArrayPtr<T> make_array_no_init(size_t size) {
    return ArrayPtr<T>(new T[size]);
}

} // namespace ClientForFrame