#pragma once

#include <filesystem>
#include <cstdlib>
#include <stdexcept>

namespace fs = std::filesystem;

/*
    Provides paths used by CaelestiaColours and Caelestia.

    CaelestiaColours stores its own schemes and backups in the user's
    XDG configuration directory, while Caelestia stores the currently
    active scheme in its XDG state directory.
*/

struct SchemePaths
{
    /*
      Returns the path to Caelestia's currently active scheme.
     
        Uses $XDG_STATE_HOME when it is set, otherwise falls back to
        $HOME/.local/state.
    
        Example:
        ~/.local/state/caelestia/scheme.json
    */
    static fs::path caelestia_scheme()
    {
        const auto* xdg_state_home = std::getenv("XDG_STATE_HOME");

        if (xdg_state_home)
        {
            return fs::path(xdg_state_home) / "caelestia" / "scheme.json";
        }

        const auto* home = std::getenv("HOME");

        if (!home)
        {
            throw std::runtime_error("HOME environment variable is not set");
        }

        return fs::path(home) / ".local" / "state" / "caelestia" / "scheme.json";
    }

    /*
        Returns the root configuration directory used by CaelestiaColours.
    
        Uses $XDG_CONFIG_HOME when it is set, otherwise falls back to
        $HOME/.config.
    
        Example:
        ~/.config/caelestia-colours
    */
    static fs::path app_config_directory()
    {
        const auto* xdg_config_home = std::getenv("XDG_CONFIG_HOME");

        if (xdg_config_home)
        {
            return fs::path(xdg_config_home) / "caelestia-colours";
        }

        const char* home = std::getenv("HOME");

        if (!home)
        {
            throw std::runtime_error("HOME environment variable is not set");
        }

        return fs::path(home) / ".config" / "caelestia-colours";
    }

    /*
        Returns the directory containing user-created/imported schemes.
    
        Example:
        ~/.config/caelestia-colours/schemes
    */
    static fs::path schemes_directory()
    {
        return app_config_directory() / "schemes";
    }

    /*
        Returns the directory containing backups of previous active schemes.
    
        A backup is created before Caelestia's active scheme is replaced

        Example:
        ~/.config/caelestia-colours/backups
    */
    static fs::path backups_directory()
    {
        return app_config_directory() / "backups";
    }
};