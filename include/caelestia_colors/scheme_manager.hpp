#pragma once

#include "scheme.hpp"
#include "scheme_paths.hpp"

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

class SchemeManager
{
    static void refresh_theme()
    {
        constexpr const char* command =
            "python -c '"
            "from caelestia.utils.scheme import get_scheme; "
            "from caelestia.utils.theme import apply_colours; "
            "s = get_scheme(); "
            "apply_colours(s.colours, s.mode)"
            "'";

        if (std::system(command) != 0)
        {
            throw std::runtime_error(
                "Scheme files were updated, but Caelestia theme refresh failed");
        }
    }

    static void remove_noexcept(const fs::path& path) noexcept
    {
        std::error_code ec;
        fs::remove(path, ec);
    }

    static void validate_scheme(const fs::path& path)
    {
        (void)Scheme::from_file(path);
    }

public:
    SchemeManager() = delete;

    static void apply_scheme(const fs::path& scheme_path)
    {
        const fs::path active = SchemePaths::caelestia_scheme();
        const fs::path previous =
            SchemePaths::backups_directory() / "previous.json";
        const fs::path next_active = active.string() + ".apply.tmp";
        const fs::path next_previous = previous.string() + ".apply.tmp";

        if (fs::equivalent(scheme_path, active))
            throw std::runtime_error("Cannot apply the active scheme to itself");

        validate_scheme(scheme_path);
        validate_scheme(active);
        fs::create_directories(previous.parent_path());

        // Staging files must not be shared with another running instance.
        try
        {
            fs::copy_file(scheme_path, next_active,
                          fs::copy_options::overwrite_existing);
            fs::copy_file(active, next_previous,
                          fs::copy_options::overwrite_existing);

            // Publish the backup first: the current active scheme is still intact.
            fs::rename(next_previous, previous);
            fs::rename(next_active, active);
        }
        catch (...)
        {
            remove_noexcept(next_active);
            remove_noexcept(next_previous);
            throw;
        }

        refresh_theme();
    }

    static void restore_previous()
    {
        const fs::path active = SchemePaths::caelestia_scheme();
        const fs::path previous =
            SchemePaths::backups_directory() / "previous.json";
        const fs::path next_active = active.string() + ".restore.tmp";
        const fs::path next_previous = previous.string() + ".restore.tmp";
        const fs::path old_previous = previous.string() + ".restore.rollback.tmp";

        if (!fs::exists(previous))
            throw std::runtime_error("There's no previous scheme");

        validate_scheme(active);
        validate_scheme(previous);

        try
        {
            // Prepare both new versions without touching the originals.
            fs::copy_file(previous, next_active,
                          fs::copy_options::overwrite_existing);
            fs::copy_file(active, next_previous,
                          fs::copy_options::overwrite_existing);
            fs::copy_file(previous, old_previous,
                          fs::copy_options::overwrite_existing);

            // First replace the backup; active remains readable throughout.
            fs::rename(next_previous, previous);

            try
            {
                // Publish the previously backed-up scheme as active.
                fs::rename(next_active, active);
            }
            catch (...)
            {
                // Restore the old backup if publishing the active file failed.
                try
                {
                    fs::rename(old_previous, previous);
                }
                catch (...)
                {
                    // Preserve rollback data on disk for manual recovery.
                    remove_noexcept(next_active);
                    throw std::runtime_error(
                        "Restore failed; previous.json may have changed. "
                        "Original backup is in: " + old_previous.string());
                }
                throw;
            }
        }
        catch (...)
        {
            remove_noexcept(next_active);
            remove_noexcept(next_previous);
            // Keep old_previous if rollback itself failed.
            throw;
        }

        remove_noexcept(old_previous);
        refresh_theme();
    }
};