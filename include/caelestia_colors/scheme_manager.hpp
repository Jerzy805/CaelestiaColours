#pragma once

#include "scheme.hpp"
#include "scheme_paths.hpp"

namespace fs = std::filesystem;

class SchemeManager
{
    static void refresh_theme()
    {
        // load the active scheme and apply its colours without regeneration
        constexpr const char* command =
            "python -c '"
            "from caelestia.utils.scheme import get_scheme; "
            "from caelestia.utils.theme import apply_colours; "
            "s = get_scheme(); "
            "apply_colours(s.colours, s.mode)"
            "'";

        // execute the Caelestia theme update
        const int result = std::system(command);

        if (result != 0)
        {
            throw std::runtime_error("Failed to refresh Caelestia theme");
        }
    }

    public:
    SchemeManager() = delete;

    /**
     * Applies a scheme as Caelestia's active scheme.
     *
     * The new scheme is first copied to a temporary file so that the
     * currently active scheme remains untouched if preparing the new
     * scheme fails. Once the temporary file is ready, the current active
     * scheme is backed up as "previous.json". Finally, the temporary file
     * replaces Caelestia's active scheme.
     *
     * This allows the previous active scheme to be restored if needed.
    */
    static void apply_scheme(const fs::path& scheme_path)
    {
        const fs::path active_scheme = SchemePaths::caelestia_scheme();
        const fs::path backup_scheme =
            SchemePaths::backups_directory() / "previous.json";

        // temporary files are created in the same directories
        // as their final destinations
        const fs::path temporary_scheme =
            active_scheme.string() + ".tmp";

        const fs::path temporary_backup =
            backup_scheme.string() + ".tmp";

        // make sure the backup directory exists
        fs::create_directories(SchemePaths::backups_directory());

        try
        {
            // prepare the new scheme without modifying the active one
            // if copying fails, the active scheme remains unchanged
            fs::copy_file(
                scheme_path,
                temporary_scheme,
                fs::copy_options::overwrite_existing
            );

            // prepare a backup of the currently active scheme
            // the existing previous.json is not overwritten yet
            fs::copy_file(
                active_scheme,
                temporary_backup,
                fs::copy_options::overwrite_existing
            );

            // replace the previous backup with the prepared backup
            fs::rename(temporary_backup, backup_scheme);

            // atomically replace the active scheme with the new one
            // both files are in the same directory
            fs::rename(temporary_scheme, active_scheme);
        }
        catch (...)
        {
            // cleanup must not throw another exception while
            // the original exception is being handled
            std::error_code ec;

            fs::remove(temporary_scheme, ec);

            ec.clear();
            fs::remove(temporary_backup, ec);

            // propagate the original exception to the caller
            throw;
        }

        refresh_theme();
    }

    /**
     * Restores the scheme that was active before the last scheme was applied.
     *
     * The previous active scheme is stored as "previous.json". It is first
     * copied to a temporary file so that the current active scheme remains
     * untouched if preparing the restore fails. Once the temporary file is
     * ready, it replaces Caelestia's active scheme.
    */
    static void restore_previous()
    {
        const fs::path active_scheme = SchemePaths::caelestia_scheme();
        const fs::path backup_scheme =
            SchemePaths::backups_directory() / "previous.json";

        if (!fs::exists(backup_scheme))
        {
            throw std::runtime_error("There's no previous scheme");
        }

        if (!fs::exists(active_scheme))
        {
            throw std::runtime_error("Active scheme does not exist");
        }

        // prepare the temporary file in the same directory as the active scheme
        const fs::path temporary_scheme =
            active_scheme.string() + ".tmp";

        // validate both schemes before modifying anything
        Scheme::from_file(active_scheme);
        Scheme::from_file(backup_scheme);

        // copy the previous scheme to a temporary file
        fs::copy_file(
            backup_scheme,
            temporary_scheme,
            fs::copy_options::overwrite_existing
        );

        try
        {
            // replace previous.json with the current active scheme
            fs::rename(active_scheme, backup_scheme);

            try
            {
                // activate the previously backed-up scheme
                fs::rename(temporary_scheme, active_scheme);
            }
            catch (...)
            {
                // restore the original active scheme if the second rename fails
                fs::rename(backup_scheme, active_scheme);
                throw;
            }
        }
        catch (...)
        {
            std::error_code ec;
            fs::remove(temporary_scheme, ec);
            throw;
        }

        refresh_theme();
    }
};