#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace client {
namespace systems {

class LobbyUI {
public:
    enum class State {
        InputPlayerName,
        InputServerAddr,
        InputServerPort,
        Connecting,
        Connected,
        Error
    };

    enum class ActiveField {
        PlayerName,
        ServerAddr,
        ServerPort
    };

    explicit LobbyUI(sf::Font& font);

    void handle_event(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    // Getters for connection
    [[nodiscard]] bool is_ready_to_connect() const { return ready_to_connect_; }
    [[nodiscard]] const std::string& player_name() const { return player_name_; }
    [[nodiscard]] const std::string& server_addr() const { return server_addr_; }
    [[nodiscard]] unsigned short server_port() const;

    // State management
    void set_state(State state) { current_state_ = state; }
    void set_error(const std::string& error);
    void reset();

private:
    void build_ui();
    void handle_text_input(char32_t unicode);
    void handle_backspace();
    void handle_tab();
    void handle_enter();
    void cycle_field();
    void update_colors();
    [[nodiscard]] bool validate_port() const;

    sf::Font& font_;
    State current_state_;
    ActiveField active_field_;
    bool ready_to_connect_;

    // Input data
    std::string player_name_;
    std::string server_addr_;
    std::string server_port_;
    std::string error_message_;

    // UI elements - SFML 3 requires Font in constructor
    sf::Text title_;
    sf::Text label_name_;
    sf::Text input_name_;
    sf::Text label_addr_;
    sf::Text input_addr_;
    sf::Text label_port_;
    sf::Text input_port_;
    sf::Text status_text_;
    sf::Text hint_text_;
    sf::Text connect_button_text_;

    // UI layout
    sf::RectangleShape input_box_name_;
    sf::RectangleShape input_box_addr_;
    sf::RectangleShape input_box_port_;
    sf::RectangleShape connect_button_;

    // Colors
    sf::Color active_color_;
    sf::Color inactive_color_;
    sf::Color bg_color_;
};

}  // namespace systems
}  // namespace client
