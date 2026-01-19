#pragma once

#include <string>
#include <unordered_map>

namespace client::systems {

enum class Language {
    English = 0,
    Spanish = 1,
    French = 2
};

class Localization {
public:
    static void setLanguage(Language lang);
    static Language getLanguage();
    static std::string get(const std::string& key);

private:
    static Language current_language_;
    static const std::unordered_map<std::string, std::string> english_strings_;
    static const std::unordered_map<std::string, std::string> spanish_strings_;
    static const std::unordered_map<std::string, std::string> french_strings_;
};

}  // namespace client::systems
