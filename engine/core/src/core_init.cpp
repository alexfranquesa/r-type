#include "engine/core/engine_core.hpp"

#include <iostream>
#include <mutex>

namespace {
std::once_flag g_coreInitFlag;
}

namespace engine::core {

std::string_view module_name() {
    return "engine::core";
}

void initialize() {
    std::call_once(g_coreInitFlag, [] {
        std::cout << "[" << module_name() << "] ECS core initialized." << std::endl;
    });
}

}  // namespace engine::core
