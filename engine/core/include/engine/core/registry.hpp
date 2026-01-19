#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <functional>
#include <vector>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <tuple>
#include <limits>
#include <iostream>

#include "entity.hpp"
#include "sparse_array.hpp"

namespace rtype::ecs {

class registry {
public:
    // COMPONENT TYPE REGISTRATION

    // Creates (if needed) and returns the storage for this component type
    template <class Component>
    sparse_array<Component> & register_component();

    // Returns the storage (non-const); creates if doesn't exist
    template <class Component>
    sparse_array<Component> & get_components();

    // Returns the storage (const); throws if doesn't exist
    template <class Component>
    sparse_array<Component> const & get_components() const;

    // ENTITY MANAGEMENT

    // Creates a new entity with a unique ID
    entity_t spawn_entity();

    // Creates an entity from an index (useful for converting index -> entity_t)
    entity_t entity_from_index(entity_id_t idx);

    // Removes all components from an entity (in all storages)
    void kill_entity(entity_t const &e);

    // COMPONENT MANAGEMENT

    // Adds a pre-constructed component to an entity
    template <typename Component>
    typename sparse_array<Component>::reference_type
    add_component(entity_t const &to, Component &&c);

    // Constructs the component "in place" in the entity
    template <typename Component, typename... Params>
    typename sparse_array<Component>::reference_type
    emplace_component(entity_t const &to, Params &&...p);

    // Removes a specific component from an entity
    template <typename Component>
    void remove_component(entity_t const &from);

    // Try to get a component pointer (returns nullptr if not present)
    template <typename Component>
    Component * try_get(entity_t const &e);

    template <typename Component>
    Component const * try_get(entity_t const &e) const;

    // Iterate over entities that have all listed components and invoke the callback
    template <typename... Components, typename Func>
    void view(Func &&func);

    template <typename... Components, typename Func>
    void view(Func &&func) const;

    // Remove all components from all registered storages
    void clear();

private:
    using component_array_any = std::any;
    using eraser_t = std::function<void(registry &, entity_t const &)>;
    using clearer_t = std::function<void(registry &)>;

    // type_index (component type) -> std::any containing sparse_array<Component>
    std::unordered_map<std::type_index, component_array_any> _components_arrays;

    // List of functions that know how to erase an entity in each storage
    std::vector<eraser_t> _component_erasers;

    // List of functions that know how to clear each storage
    std::vector<clearer_t> _component_clearers;

    // Next free ID for new entities
    entity_id_t _next_entity_id{0};
};

// ================= IMPLEMENTATION =================

template <class Component>
sparse_array<Component> & registry::register_component() {
    std::type_index key{typeid(Component)};
    auto it = _components_arrays.find(key);

    if (it == _components_arrays.end()) {
        // Create a new storage for this component type
        auto [inserted_it, inserted] =
            _components_arrays.emplace(key, sparse_array<Component>{});

        // Register a function that knows how to delete this component for a given entity
        if (inserted) {
            _component_erasers.emplace_back(
                [](registry &reg, entity_t const &e) {
                    auto &array = reg.get_components<Component>();
                    entity_id_t idx = static_cast<entity_id_t>(e);
                    if (idx < array.size()) {
                        array.erase(idx);
                    }
                }
            );
            _component_clearers.emplace_back(
                [](registry &reg) {
                    reg.get_components<Component>().clear();
                }
            );
        }

        it = inserted_it;
    }

    return std::any_cast<sparse_array<Component> &>(it->second);
}

template <class Component>
sparse_array<Component> & registry::get_components() {
    std::type_index key{typeid(Component)};
    auto it = _components_arrays.find(key);

    if (it == _components_arrays.end()) {
        // If not registered, register it now
        return register_component<Component>();
    }

    return std::any_cast<sparse_array<Component> &>(it->second);
}

template <class Component>
sparse_array<Component> const & registry::get_components() const {
    std::type_index key{typeid(Component)};
    auto it = _components_arrays.find(key);

    if (it == _components_arrays.end()) {
        throw std::runtime_error("Component not registered in registry");
    }

    return std::any_cast<sparse_array<Component> const &>(it->second);
}

inline entity_t registry::spawn_entity() {
    return entity_t{_next_entity_id++};
}

inline entity_t registry::entity_from_index(entity_id_t idx) {
    return entity_t{idx};
}

inline void registry::kill_entity(entity_t const &e) {
    // Call all functions that erase components
    for (auto &eraser : _component_erasers) {
        eraser(*this, e);
    }
}

template <typename Component>
typename sparse_array<Component>::reference_type
registry::add_component(entity_t const &to, Component &&c) {
    auto &array = get_components<Component>();
    entity_id_t idx = static_cast<entity_id_t>(to);
    return array.insert_at(idx, std::forward<Component>(c));
}

template <typename Component, typename... Params>
typename sparse_array<Component>::reference_type
registry::emplace_component(entity_t const &to, Params &&...p) {
    auto &array = get_components<Component>();
    entity_id_t idx = static_cast<entity_id_t>(to);
    return array.emplace_at(idx, std::forward<Params>(p)...);
}

template <typename Component>
void registry::remove_component(entity_t const &from) {
    auto &array = get_components<Component>();
    entity_id_t idx = static_cast<entity_id_t>(from);
    if (idx < array.size()) {
        array.erase(idx);
    }
}

template <typename Component>
Component * registry::try_get(entity_t const &e) {
    auto &array = get_components<Component>();
    entity_id_t idx = static_cast<entity_id_t>(e);
    
    if (idx >= array.size()) {
        return nullptr;
    }
    
    auto &opt = array[idx];
    if (!opt.has_value()) {
        return nullptr;
    }
    
    return &(opt.value());
}

template <typename Component>
Component const * registry::try_get(entity_t const &e) const {
    auto const &array = get_components<Component>();
    entity_id_t idx = static_cast<entity_id_t>(e);
    
    if (idx >= array.size()) {
        return nullptr;
    }
    
    auto const &opt = array[idx];
    if (!opt.has_value()) {
        return nullptr;
    }
    
    return &(opt.value());
}

// View implementation: iterate entities that have all components in Components...
template <typename... Components, typename Func>
void registry::view(Func &&func) {
    // Ensure storages exist
    (get_components<Components>(), ...);

    const std::size_t max_size = std::max({get_components<Components>().size()...});

    for (std::size_t idx = 0; idx < max_size; ++idx) {
        bool has_all = ((idx < get_components<Components>().size() &&
                         get_components<Components>()[idx].has_value()) && ...);
        if (!has_all) {
            continue;
        }
        func(entity_t{static_cast<entity_id_t>(idx)}, (*get_components<Components>()[idx])...);
    }
}

template <typename... Components, typename Func>
void registry::view(Func &&func) const {
    const std::size_t max_size = std::max({get_components<Components>().size()...});

    for (std::size_t idx = 0; idx < max_size; ++idx) {
        bool has_all = ((idx < get_components<Components>().size() &&
                         get_components<Components>()[idx].has_value()) && ...);
        if (!has_all) {
            continue;
        }
        func(entity_t{static_cast<entity_id_t>(idx)}, (*get_components<Components>()[idx])...);
    }
}

// ================= VIEW API IMPLEMENTATION =================

inline void registry::clear() {
    std::cout << "[REGISTRY::CLEAR] Starting clear, _component_clearers.size()=" << _component_clearers.size() << std::endl;
    std::cout << "[REGISTRY::CLEAR] _next_entity_id before=" << _next_entity_id << std::endl;
    
    for (size_t i = 0; i < _component_clearers.size(); ++i) {
        std::cout << "[REGISTRY::CLEAR] Calling clearer #" << i << std::endl;
        _component_clearers[i](*this);
    }
    
    _next_entity_id = 0;  // Reset entity ID counter to sync with server
    std::cout << "[REGISTRY::CLEAR] Finished clear, _next_entity_id=" << _next_entity_id << std::endl;
}

}  // namespace rtype::ecs
