#include "localization.hpp"

namespace client::systems {

Language Localization::current_language_ = Language::English;

const std::unordered_map<std::string, std::string> Localization::english_strings_ = {
    // Settings Menu
    {"settings", "SETTINGS"},
    {"audio", "Audio"},
    {"gameplay", "Gameplay"},
    {"graphics", "Graphics"},
    {"network", "Network"},
    {"accessibility", "Accessibility"},
    
    // Audio Settings
    {"master_volume", "Master Volume"},
    {"music_volume", "Music Volume"},
    {"sfx_volume", "SFX Volume"},
    
    // Gameplay Settings
    {"difficulty", "Difficulty"},
    {"player_lives", "Player Lives"},
    {"enemies_per_wave", "Enemies / Wave"},
    {"kills_per_wave", "Kills / Wave"},
    {"infinite_lives", "Infinite Lives"},
    {"difficulty_easy", "Easy"},
    {"difficulty_normal", "Normal"},
    {"difficulty_hard", "Hard"},
    {"difficulty_hardcore", "Hardcore"},
    
    // Graphics Settings
    {"fullscreen", "Fullscreen"},
    {"vsync", "VSync"},
    {"show_fps", "Show FPS"},
    {"screen_shake", "Screen Shake"},
    {"target_fps", "Target FPS"},
    
    // Network Settings
    {"player_name", "Player Name"},
    {"server_ip", "Server IP"},
    {"server_port", "Server Port"},
    
    // Accessibility Settings
    {"high_contrast", "High Contrast"},
    {"font_scale", "Font Scale"},
    {"language", "Language"},
    {"language_english", "English"},
    {"language_spanish", "Español"},
    {"language_french", "Français"},
    
    // Common
    {"on", "On"},
    {"off", "Off"},
    {"apply", "Apply"},
    {"back", "Back"},
    
    // Main Menu / Lobby
    {"connect", "CONNECT"},
    {"host", "Host"},
    {"port", "Port"},
    {"select_level", "SELECT LEVEL:"},
    {"waiting_players", "Waiting for players..."},
    {"players", "Players"},
    
    // Main Menu Buttons
    {"play", ">> PLAY <<"},
    {"scores", ">> SCORES <<"},
    {"help", ">> HELP <<"},
    {"quit", ">> QUIT <<"},
    
    // Help Screen
    {"help_title", "HOW TO PLAY"},
    {"help_controls", "CONTROLS"},
    {"help_movement", "Arrow Keys / WASD - Move your ship"},
    {"help_shoot", "SPACE - Shoot"},
    {"help_ultimate", "E - Ultimate Attack (when charged)"},
    {"help_settings_title", "SETTINGS"},
    {"help_settings_desc", "Press ESC in the main menu to access settings. You can adjust audio, gameplay difficulty, graphics, and accessibility options including language."},
    {"help_ultimate_title", "ULTIMATE ABILITY"},
    {"help_ultimate_desc", "Your ultimate charges by dealing damage to enemies. When fully charged, press E to unleash a devastating attack. Use it wisely during tough situations!"},
    {"help_levels_title", "LEVELS"},
    {"help_level1", "Level 1: Basic enemies, learn the controls"},
    {"help_level2", "Level 2: Faster enemies, more aggressive"},
    {"help_level3", "Level 3: Multiple enemy types"},
    {"help_level4", "Level 4: Advanced patterns"},
    {"help_level5", "Level 5: Boss battle!"},
    {"help_tips_title", "TIPS"},
    {"help_tip1", "- Stay mobile to avoid enemy fire"},
    {"help_tip2", "- Save your ultimate for critical moments"},
    {"help_tip3", "- Higher difficulty means better rewards"},
    {"press_back", "Press ESC or click BACK to return"},
    {"back_to_menu", ">> BACK TO MENU <<"}
};

const std::unordered_map<std::string, std::string> Localization::spanish_strings_ = {
    // Settings Menu
    {"settings", "CONFIGURACION"},
    {"audio", "Audio"},
    {"gameplay", "Jugabilidad"},
    {"graphics", "Graficos"},
    {"network", "Red"},
    {"accessibility", "Accesibilidad"},
    
    // Audio Settings
    {"master_volume", "Volumen General"},
    {"music_volume", "Volumen Musica"},
    {"sfx_volume", "Volumen Efectos"},
    
    // Gameplay Settings
    {"difficulty", "Dificultad"},
    {"player_lives", "Vidas del Jugador"},
    {"enemies_per_wave", "Enemigos / Oleada"},
    {"kills_per_wave", "Muertes / Oleada"},
    {"infinite_lives", "Vidas Infinitas"},
    {"difficulty_easy", "Facil"},
    {"difficulty_normal", "Normal"},
    {"difficulty_hard", "Dificil"},
    {"difficulty_hardcore", "Extremo"},
    
    // Graphics Settings
    {"fullscreen", "Pantalla Completa"},
    {"vsync", "VSync"},
    {"show_fps", "Mostrar FPS"},
    {"screen_shake", "Vibracion Pantalla"},
    {"target_fps", "FPS Objetivo"},
    
    // Network Settings
    {"player_name", "Nombre del Jugador"},
    {"server_ip", "IP del Servidor"},
    {"server_port", "Puerto del Servidor"},
    
    // Accessibility Settings
    {"high_contrast", "Alto Contraste"},
    {"font_scale", "Escala de Fuente"},
    {"language", "Idioma"},
    {"language_english", "English"},
    {"language_spanish", "Español"},
    {"language_french", "Français"},
    
    // Common
    {"on", "Activado"},
    {"off", "Desactivado"},
    {"apply", "Aplicar"},
    {"back", "Atras"},
    
    // Main Menu / Lobby
    {"connect", "CONECTAR"},
    {"host", "Host"},
    {"port", "Puerto"},
    {"select_level", "SELECCIONAR NIVEL:"},
    {"waiting_players", "Esperando jugadores..."},
    {"players", "Jugadores"},
    
    // Main Menu Buttons
    {"play", ">> JUGAR <<"},
    {"scores", ">> PUNTUACIONES <<"},
    {"help", ">> AYUDA <<"},
    {"quit", ">> SALIR <<"},
    
    // Help Screen
    {"help_title", "CÓMO JUGAR"},
    {"help_controls", "CONTROLES"},
    {"help_movement", "Flechas / WASD - Mover tu nave"},
    {"help_shoot", "ESPACIO - Disparar"},
    {"help_ultimate", "E - Ataque Ultimate (cuando esté cargado)"},
    {"help_settings_title", "CONFIGURACIÓN"},
    {"help_settings_desc", "Presiona ESC en el menú principal para acceder a la configuración. Puedes ajustar audio, dificultad de juego, gráficos y opciones de accesibilidad incluyendo el idioma."},
    {"help_ultimate_title", "HABILIDAD ULTIMATE"},
    {"help_ultimate_desc", "Tu ultimate se carga causando daño a los enemigos. Cuando esté completamente cargado, presiona E para desatar un ataque devastador. ¡Úsalo sabiamente en situaciones difíciles!"},
    {"help_levels_title", "NIVELES"},
    {"help_level1", "Nivel 1: Enemigos básicos, aprende los controles"},
    {"help_level2", "Nivel 2: Enemigos más rápidos y agresivos"},
    {"help_level3", "Nivel 3: Múltiples tipos de enemigos"},
    {"help_level4", "Nivel 4: Patrones avanzados"},
    {"help_level5", "Nivel 5: ¡Batalla contra el jefe!"},
    {"help_tips_title", "CONSEJOS"},
    {"help_tip1", "- Mantente en movimiento para evitar el fuego enemigo"},
    {"help_tip2", "- Guarda tu ultimate para momentos críticos"},
    {"help_tip3", "- Mayor dificultad significa mejores recompensas"},
    {"press_back", "Presiona ESC o haz clic en ATRÁS para volver"},
    {"back_to_menu", ">> VOLVER AL MENÚ <<"}
};

const std::unordered_map<std::string, std::string> Localization::french_strings_ = {
    // Settings Menu
    {"settings", "PARAMETRES"},
    {"audio", "Audio"},
    {"gameplay", "Gameplay"},
    {"graphics", "Graphiques"},
    {"network", "Reseau"},
    {"accessibility", "Accessibilite"},
    
    // Audio Settings
    {"master_volume", "Volume Principal"},
    {"music_volume", "Volume Musique"},
    {"sfx_volume", "Volume Effets"},
    
    // Gameplay Settings
    {"difficulty", "Difficulte"},
    {"player_lives", "Vies du Joueur"},
    {"enemies_per_wave", "Ennemis / Vague"},
    {"kills_per_wave", "Éliminations / Vague"},
    {"infinite_lives", "Vies Infinies"},
    {"difficulty_easy", "Facile"},
    {"difficulty_normal", "Normal"},
    {"difficulty_hard", "Difficile"},
    {"difficulty_hardcore", "Extreme"},
    
    // Graphics Settings
    {"fullscreen", "Plein Ecran"},
    {"vsync", "VSync"},
    {"show_fps", "Afficher FPS"},
    {"screen_shake", "Tremblement Ecran"},
    {"target_fps", "FPS Cible"},
    
    // Network Settings
    {"player_name", "Nom du Joueur"},
    {"server_ip", "IP du Serveur"},
    {"server_port", "Port du Serveur"},
    
    // Accessibility Settings
    {"high_contrast", "Contraste Eleve"},
    {"font_scale", "Echelle de Police"},
    {"language", "Langue"},
    {"language_english", "English"},
    {"language_spanish", "Español"},
    {"language_french", "Français"},
    
    // Common
    {"on", "Active"},
    {"off", "DDesactive"},
    {"apply", "Appliquer"},
    {"back", "Retour"},
    
    // Main Menu / Lobby
    {"connect", "CONNECTER"},
    {"host", "Hote"},
    {"port", "Port"},
    {"select_level", "SELECTIONNER NIVEAU:"},
    {"waiting_players", "En attente de joueurs..."},
    {"players", "Joueurs"},
    
    // Main Menu Buttons
    {"play", ">> JOUER <<"},
    {"scores", ">> SCORES <<"},
    {"help", ">> AIDE <<"},
    {"quit", ">> QUITTER <<"},
    
    // Help Screen
    {"help_title", "COMMENT JOUER"},
    {"help_controls", "CONTRÔLES"},
    {"help_movement", "Flèches / WASD - Déplacer votre vaisseau"},
    {"help_shoot", "ESPACE - Tirer"},
    {"help_ultimate", "E - Attaque Ultimate (quand chargé)"},
    {"help_settings_title", "PARAMÈTRES"},
    {"help_settings_desc", "Appuyez sur ESC dans le menu principal pour accéder aux paramètres. Vous pouvez ajuster l'audio, la difficulté du jeu, les graphiques et les options d'accessibilité y compris la langue."},
    {"help_ultimate_title", "CAPACITÉ ULTIMATE"},
    {"help_ultimate_desc", "Votre ultimate se charge en infligeant des dégâts aux ennemis. Lorsqu'il est complètement chargé, appuyez sur E pour déclencher une attaque dévastatrice. Utilisez-le judicieusement dans les situations difficiles!"},
    {"help_levels_title", "NIVEAUX"},
    {"help_level1", "Niveau 1: Ennemis basiques, apprenez les contrôles"},
    {"help_level2", "Niveau 2: Ennemis plus rapides et agressifs"},
    {"help_level3", "Niveau 3: Plusieurs types d'ennemis"},
    {"help_level4", "Niveau 4: Motifs avancés"},
    {"help_level5", "Niveau 5: Combat de boss!"},
    {"help_tips_title", "CONSEILS"},
    {"help_tip1", "- Restez mobile pour éviter les tirs ennemis"},
    {"help_tip2", "- Gardez votre ultimate pour les moments critiques"},
    {"help_tip3", "- Plus de difficulté signifie de meilleures récompenses"},
    {"press_back", "Appuyez sur ESC ou cliquez sur RETOUR pour revenir"},
    {"back_to_menu", ">> RETOUR AU MENU <<"}
};

void Localization::setLanguage(Language lang) {
    current_language_ = lang;
}

Language Localization::getLanguage() {
    return current_language_;
}

std::string Localization::get(const std::string& key) {
    const std::unordered_map<std::string, std::string>* strings = nullptr;
    
    switch (current_language_) {
        case Language::Spanish:
            strings = &spanish_strings_;
            break;
        case Language::French:
            strings = &french_strings_;
            break;
        case Language::English:
        default:
            strings = &english_strings_;
            break;
    }
    
    auto it = strings->find(key);
    if (it != strings->end()) {
        return it->second;
    }
    
    // Fallback to English if key not found
    auto fallback = english_strings_.find(key);
    if (fallback != english_strings_.end()) {
        return fallback->second;
    }
    
    // Return key itself if not found anywhere
    return key;
}

}  // namespace client::systems
