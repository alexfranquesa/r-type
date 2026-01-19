#include "lobby_ui_system.hpp"
#include <algorithm>
#include <cctype>

namespace client {
namespace systems {

LobbyUI::LobbyUI(sf::Font& font)
    : font_(font)
    , current_state_(State::InputPlayerName)
    , active_field_(ActiveField::PlayerName)
    , ready_to_connect_(false)
    , player_name_()
    , server_addr_("127.0.0.1")
    , server_port_("4242")
    , error_message_()
    , title_(font)
    , label_name_(font)
    , input_name_(font)
    , label_addr_(font)
    , input_addr_(font)
    , label_port_(font)
    , input_port_(font)
    , status_text_(font)
    , hint_text_(font)
    , connect_button_text_(font)
    , active_color_(100, 150, 255)
    , inactive_color_(180, 180, 180)
    , bg_color_(40, 44, 52) {
    build_ui();
}

void LobbyUI::build_ui() {
    // Title
    title_.setString("R-TYPE Multiplayer Lobby");
    title_.setCharacterSize(48);
    title_.setFillColor(sf::Color::White);
    title_.setPosition(sf::Vector2f(320.0f, 100.0f));

    // Player Name
    label_name_.setString("Player Name:");
    label_name_.setCharacterSize(24);
    label_name_.setFillColor(sf::Color::White);
    label_name_.setPosition(sf::Vector2f(200.0f, 220.0f));

    input_name_.setCharacterSize(24);
    input_name_.setPosition(sf::Vector2f(210.0f, 260.0f));

    input_box_name_.setSize(sf::Vector2f(400.0f, 50.0f));
    input_box_name_.setPosition(sf::Vector2f(200.0f, 250.0f));
    input_box_name_.setFillColor(bg_color_);
    input_box_name_.setOutlineThickness(2.0f);

    // Server Address
    label_addr_.setString("Server Address:");
    label_addr_.setCharacterSize(24);
    label_addr_.setFillColor(sf::Color::White);
    label_addr_.setPosition(sf::Vector2f(200.0f, 340.0f));

    input_addr_.setString(server_addr_);
    input_addr_.setCharacterSize(24);
    input_addr_.setPosition(sf::Vector2f(210.0f, 380.0f));

    input_box_addr_.setSize(sf::Vector2f(400.0f, 50.0f));
    input_box_addr_.setPosition(sf::Vector2f(200.0f, 370.0f));
    input_box_addr_.setFillColor(bg_color_);
    input_box_addr_.setOutlineThickness(2.0f);

    // Server Port
    label_port_.setString("Port:");
    label_port_.setCharacterSize(24);
    label_port_.setFillColor(sf::Color::White);
    label_port_.setPosition(sf::Vector2f(200.0f, 460.0f));

    input_port_.setString(server_port_);
    input_port_.setCharacterSize(24);
    input_port_.setPosition(sf::Vector2f(210.0f, 500.0f));

    input_box_port_.setSize(sf::Vector2f(200.0f, 50.0f));
    input_box_port_.setPosition(sf::Vector2f(200.0f, 490.0f));
    input_box_port_.setFillColor(bg_color_);
    input_box_port_.setOutlineThickness(2.0f);

    // Connect button
    connect_button_.setSize(sf::Vector2f(200.0f, 60.0f));
    connect_button_.setPosition(sf::Vector2f(440.0f, 580.0f));
    connect_button_.setFillColor(sf::Color(50, 150, 50));
    connect_button_.setOutlineThickness(2.0f);
    connect_button_.setOutlineColor(sf::Color(70, 200, 70));

    connect_button_text_.setString("Connect");
    connect_button_text_.setCharacterSize(28);
    connect_button_text_.setFillColor(sf::Color::White);
    connect_button_text_.setPosition(sf::Vector2f(490.0f, 595.0f));

    // Status text
    status_text_.setCharacterSize(20);
    status_text_.setFillColor(sf::Color::Yellow);
    status_text_.setPosition(sf::Vector2f(200.0f, 660.0f));

    // Hint text
    hint_text_.setString("Tab: Next field | Enter: Connect | ESC: Quit");
    hint_text_.setCharacterSize(18);
    hint_text_.setFillColor(sf::Color(150, 150, 150));
    hint_text_.setPosition(sf::Vector2f(320.0f, 690.0f));

    update_colors();
}

void LobbyUI::handle_event(const sf::Event& event, sf::RenderWindow& window) {
    if (current_state_ == State::Connecting || current_state_ == State::Connected) {
        return;
    }

    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>()) {
        if (key_pressed->code == sf::Keyboard::Key::Tab) {
            handle_tab();
        } else if (key_pressed->code == sf::Keyboard::Key::Enter) {
            handle_enter();
        } else if (key_pressed->code == sf::Keyboard::Key::Backspace) {
            handle_backspace();
        }
    }

    if (const auto* text_entered = event.getIf<sf::Event::TextEntered>()) {
        if (text_entered->unicode >= 32 && text_entered->unicode < 127) {
            handle_text_input(static_cast<char32_t>(text_entered->unicode));
        }
    }

    if (const auto* mouse_clicked = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse_clicked->button == sf::Mouse::Button::Left) {
            // Map mouse coordinates from window to world coordinates
            sf::Vector2i pixel_pos(mouse_clicked->position.x, mouse_clicked->position.y);
            sf::Vector2f mouse_pos = window.mapPixelToCoords(pixel_pos);
            
            if (input_box_name_.getGlobalBounds().contains(mouse_pos)) {
                active_field_ = ActiveField::PlayerName;
                update_colors();
            } else if (input_box_addr_.getGlobalBounds().contains(mouse_pos)) {
                active_field_ = ActiveField::ServerAddr;
                update_colors();
            } else if (input_box_port_.getGlobalBounds().contains(mouse_pos)) {
                active_field_ = ActiveField::ServerPort;
                update_colors();
            } else if (connect_button_.getGlobalBounds().contains(mouse_pos)) {
                handle_enter();
            }
        }
    }
}

void LobbyUI::handle_text_input(char32_t unicode) {
    char c = static_cast<char>(unicode);

    switch (active_field_) {
        case ActiveField::PlayerName:
            if (player_name_.size() < 16) {
                player_name_ += c;
                input_name_.setString(player_name_);
            }
            break;
        case ActiveField::ServerAddr:
            if (server_addr_.size() < 45) {
                server_addr_ += c;
                input_addr_.setString(server_addr_);
            }
            break;
        case ActiveField::ServerPort:
            if (std::isdigit(c) && server_port_.size() < 5) {
                server_port_ += c;
                input_port_.setString(server_port_);
            }
            break;
    }
}

void LobbyUI::handle_backspace() {
    switch (active_field_) {
        case ActiveField::PlayerName:
            if (!player_name_.empty()) {
                player_name_.pop_back();
                input_name_.setString(player_name_);
            }
            break;
        case ActiveField::ServerAddr:
            if (!server_addr_.empty()) {
                server_addr_.pop_back();
                input_addr_.setString(server_addr_);
            }
            break;
        case ActiveField::ServerPort:
            if (!server_port_.empty()) {
                server_port_.pop_back();
                input_port_.setString(server_port_);
            }
            break;
    }
}

void LobbyUI::handle_tab() {
    cycle_field();
}

void LobbyUI::handle_enter() {
    if (player_name_.empty()) {
        set_error("Please enter your name");
        return;
    }
    if (server_addr_.empty()) {
        set_error("Please enter server address");
        return;
    }
    if (!validate_port()) {
        set_error("Invalid port number");
        return;
    }

    ready_to_connect_ = true;
    current_state_ = State::Connecting;
    status_text_.setString("Connecting...");
    status_text_.setFillColor(sf::Color::Yellow);
}

void LobbyUI::cycle_field() {
    switch (active_field_) {
        case ActiveField::PlayerName:
            active_field_ = ActiveField::ServerAddr;
            break;
        case ActiveField::ServerAddr:
            active_field_ = ActiveField::ServerPort;
            break;
        case ActiveField::ServerPort:
            active_field_ = ActiveField::PlayerName;
            break;
    }
    update_colors();
}

void LobbyUI::update_colors() {
    input_box_name_.setOutlineColor(active_field_ == ActiveField::PlayerName ? active_color_ : inactive_color_);
    input_box_addr_.setOutlineColor(active_field_ == ActiveField::ServerAddr ? active_color_ : inactive_color_);
    input_box_port_.setOutlineColor(active_field_ == ActiveField::ServerPort ? active_color_ : inactive_color_);

    input_name_.setFillColor(active_field_ == ActiveField::PlayerName ? sf::Color::White : sf::Color(200, 200, 200));
    input_addr_.setFillColor(active_field_ == ActiveField::ServerAddr ? sf::Color::White : sf::Color(200, 200, 200));
    input_port_.setFillColor(active_field_ == ActiveField::ServerPort ? sf::Color::White : sf::Color(200, 200, 200));
}

bool LobbyUI::validate_port() const {
    if (server_port_.empty()) {
        return false;
    }
    try {
        int port = std::stoi(server_port_);
        return port > 0 && port <= 65535;
    } catch (...) {
        return false;
    }
}

unsigned short LobbyUI::server_port() const {
    return static_cast<unsigned short>(std::stoi(server_port_));
}

void LobbyUI::set_error(const std::string& error) {
    error_message_ = error;
    current_state_ = State::Error;
    status_text_.setString("Error: " + error);
    status_text_.setFillColor(sf::Color::Red);
    ready_to_connect_ = false;
}

void LobbyUI::reset() {
    ready_to_connect_ = false;
    current_state_ = State::InputPlayerName;
    active_field_ = ActiveField::PlayerName;
    error_message_.clear();
    player_name_.clear();
    input_name_.setString("");
    status_text_.setString("");
    update_colors();
}

void LobbyUI::update(float dt) {
    // Future: add animations, cursor blinking, etc.
    (void)dt;
}

void LobbyUI::draw(sf::RenderWindow& window) {
    window.draw(title_);

    window.draw(input_box_name_);
    window.draw(label_name_);
    window.draw(input_name_);

    window.draw(input_box_addr_);
    window.draw(label_addr_);
    window.draw(input_addr_);

    window.draw(input_box_port_);
    window.draw(label_port_);
    window.draw(input_port_);

    window.draw(connect_button_);
    window.draw(connect_button_text_);

    window.draw(status_text_);
    window.draw(hint_text_);
}

}  // namespace systems
}  // namespace client
