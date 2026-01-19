#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <utility>
#include <iostream>

namespace rtype::ecs {

template <typename Component>
class sparse_array {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;

    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

public:
    // Constructors / assignments
    sparse_array() = default;

    sparse_array(sparse_array const &) = default;
    sparse_array(sparse_array &&) noexcept = default;
    ~sparse_array() = default;

    sparse_array & operator=(sparse_array const &) = default;
    sparse_array & operator=(sparse_array &&) noexcept = default;

    // Access by index (slot)
    reference_type operator[](size_type idx) {
        if (idx >= _data.size()) {
            _data.resize(idx + 1);
        }
        return _data[idx];
    }

    const_reference_type operator[](size_type idx) const {
        return _data[idx];
    }

    // Iterators
    iterator begin() {
        return _data.begin();
    }

    const_iterator begin() const {
        return _data.begin();
    }

    const_iterator cbegin() const {
        return _data.cbegin();
    }

    iterator end() {
        return _data.end();
    }

    const_iterator end() const {
        return _data.end();
    }

    const_iterator cend() const {
        return _data.cend();
    }

    // Size of the storage (number of slots)
    size_type size() const {
        return _data.size();
    }

    // Insert component at index (copy)
    reference_type insert_at(size_type pos, Component const &value) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = value;
        return _data[pos];
    }

    // Insert component at index (move)
    reference_type insert_at(size_type pos, Component &&value) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = std::move(value);
        return _data[pos];
    }

    // Construct component in-place at index
    template <class... Params>
    reference_type emplace_at(size_type pos, Params &&...params) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(std::forward<Params>(params)...);
        return _data[pos];
    }

    // Remove component at index (slot remains but is empty)
    void erase(size_type pos) {
        if (pos < _data.size()) {
            _data[pos].reset();
        }
    }

    // Get index of a slot (given the optional reference)
    size_type get_index(value_type const &value) const {
        auto ptr = std::addressof(value);   // address of this slot
        auto base = _data.data();           // address of first slot
        return static_cast<size_type>(ptr - base);
    }

    // Clear all stored components and release memory
    void clear() {
        size_type old_size = _data.size();
        _data.clear();
        std::cout << "[SPARSE_ARRAY::CLEAR] Cleared " << old_size << " slots" << std::endl;
    }

private:
    container_t _data;
};

}  // namespace rtype::ecs
