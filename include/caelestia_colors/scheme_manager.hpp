#pragma once

#include "scheme_paths.hpp"

namespace fs = std::filesystem;

struct SchemeManager
{
    SchemeManager() {}

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
    void apply_scheme(const fs::path& scheme_path)
    {   
        const auto active_scheme = SchemePaths::caelestia_scheme();
        const auto backup_scheme = SchemePaths::backups_directory()/"previous.json";
        const auto temporary_scheme = active_scheme.string() + ".tmp";

        fs::create_directories(SchemePaths::backups_directory());

        // making the temporary file from scheme_path
        auto temporary_created = fs::copy_file(scheme_path, temporary_scheme,
            fs::copy_options::overwrite_existing);

        if (!temporary_created)
        {
            throw std::runtime_error("Applying scheme failed");
            // we didn't change the original file, so the rollback isn't a thing here
        }

        // making sure current scheme is saved and possible to recover
        auto backup_created = fs::copy_file(SchemePaths::caelestia_scheme(),
            SchemePaths::backups_directory()/"previous.json", fs::copy_options::overwrite_existing);

        if (!backup_created)
        {
            fs::remove(temporary_scheme);
            throw std::runtime_error("Making the backup of current scheme failed");
        }        

        try
        {
            fs::rename(temporary_scheme, active_scheme);
        }
        catch(...)
        {
            std::error_code error;
            fs::remove(temporary_scheme, error);
            throw;
        }
    }

    /**
     * Restores the scheme that was active before the last scheme was applied.
     *
     * The previous active scheme is stored as "previous.json". It is first
     * copied to a temporary file so that the current active scheme remains
     * untouched if preparing the restore fails. Once the temporary file is
     * ready, it replaces Caelestia's active scheme.
    */
    void restore_previous()
    {
        const auto backup_scheme = SchemePaths::backups_directory()/"previous.json";

        if (!fs::exists(backup_scheme))
        {
            throw std::runtime_error("There's no backup scheme");
        }

        const auto active_scheme = SchemePaths::caelestia_scheme();
        const auto temporary_scheme = active_scheme.string() + ".tmp";

        auto temporary_created = fs::copy_file(backup_scheme, temporary_scheme, fs::copy_options::overwrite_existing);

        if (!temporary_created)
        {
            throw std::runtime_error("Creating the temporary file failed");
        }

        try
        {
            fs::rename(temporary_scheme, active_scheme);
        }
        catch(...)
        {
            std::error_code error;
            fs::remove(temporary_scheme, error);
            throw;
        }
    }
};