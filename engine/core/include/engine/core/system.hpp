#pragma once

#include <memory>
#include <vector>
#include <typeindex>
#include <algorithm>

#include "ISystem.hpp"

namespace rtype::ecs {

/**
 * @brief Manages and executes all game systems in a deterministic order
 * 
 * The SystemScheduler maintains a collection of systems and executes them
 * each frame. Systems can be dynamically added, removed, enabled, or disabled.
 * Execution order is determined by the order systems are added.
 */
class SystemScheduler {
public:
    SystemScheduler() = default;
    ~SystemScheduler() = default;

    // Non-copyable
    SystemScheduler(const SystemScheduler&) = delete;
    SystemScheduler& operator=(const SystemScheduler&) = delete;

    // Movable
    SystemScheduler(SystemScheduler&&) noexcept = default;
    SystemScheduler& operator=(SystemScheduler&&) noexcept = default;

    /**
     * @brief Add a new system to the scheduler
     * @tparam T System type (must inherit from ISystem)
     * @tparam Args Constructor argument types
     * @param args Arguments forwarded to the system constructor
     * @return Reference to the added system
     */
    template <typename T, typename... Args>
    T& add_system(Args&&... args);

    /**
     * @brief Remove a system from the scheduler
     * @tparam T System type to remove
     * @return true if system was removed, false if not found
     */
    template <typename T>
    bool remove_system();

    /**
     * @brief Enable a system (it will run during run_frame)
     * @tparam T System type to enable
     * @return true if system was enabled, false if not found
     */
    template <typename T>
    bool enable_system();

    /**
     * @brief Disable a system (it will not run during run_frame)
     * @tparam T System type to disable
     * @return true if system was disabled, false if not found
     */
    template <typename T>
    bool disable_system();

    /**
     * @brief Check if a system is currently enabled
     * @tparam T System type to check
     * @return true if enabled, false if disabled or not found
     */
    template <typename T>
    bool is_enabled() const;

    /**
     * @brief Execute all enabled systems for one frame
     * @param reg The ECS registry
     * @param dt Delta time in seconds
     */
    void run_frame(registry& reg, float dt);

    /**
     * @brief Get the number of systems (enabled and disabled)
     * @return Total system count
     */
    size_t system_count() const { return _systems.size(); }

private:
    struct SystemEntry {
        std::unique_ptr<ISystem> system;
        std::type_index type;
        bool enabled;

        SystemEntry(std::unique_ptr<ISystem> sys, std::type_index t)
            : system(std::move(sys)), type(t), enabled(true) {}
    };

    std::vector<SystemEntry> _systems;

    // Helper to find system by type
    auto find_system(std::type_index type) {
        return std::find_if(_systems.begin(), _systems.end(),
            [type](const SystemEntry& entry) { return entry.type == type; });
    }

    auto find_system(std::type_index type) const {
        return std::find_if(_systems.begin(), _systems.end(),
            [type](const SystemEntry& entry) { return entry.type == type; });
    }
};

// ================= IMPLEMENTATION =================

template <typename T, typename... Args>
T& SystemScheduler::add_system(Args&&... args) {
    static_assert(std::is_base_of_v<ISystem, T>, "T must inherit from ISystem");

    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = system.get();

    _systems.emplace_back(std::move(system), std::type_index(typeid(T)));

    return *ptr;
}

template <typename T>
bool SystemScheduler::remove_system() {
    auto it = find_system(std::type_index(typeid(T)));
    if (it != _systems.end()) {
        _systems.erase(it);
        return true;
    }
    return false;
}

template <typename T>
bool SystemScheduler::enable_system() {
    auto it = find_system(std::type_index(typeid(T)));
    if (it != _systems.end()) {
        it->enabled = true;
        return true;
    }
    return false;
}

template <typename T>
bool SystemScheduler::disable_system() {
    auto it = find_system(std::type_index(typeid(T)));
    if (it != _systems.end()) {
        it->enabled = false;
        return true;
    }
    return false;
}

template <typename T>
bool SystemScheduler::is_enabled() const {
    auto it = find_system(std::type_index(typeid(T)));
    return it != _systems.end() && it->enabled;
}

inline void SystemScheduler::run_frame(registry& reg, float dt) {
    for (auto& entry : _systems) {
        if (entry.enabled && entry.system) {
            entry.system->run(reg, dt);
        }
    }
}

}  // namespace rtype::ecs

