// API class for all data operations

#pragma once

#include "scheme.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class SchemeEditor
{
    Scheme original; // originally loaded scheme
    Scheme current; // current data, potentially modified by the user
    fs::path path; // path of the file from which the original data was loaded

    public:
    SchemeEditor(const Scheme& scheme, const fs::path& path) : original(scheme), 
        current(scheme), path(path) {} // original and current are initially always identical

    const Scheme& get_original() const { return original; } // modification of the original is not allowed
    const Scheme& get_current() const { return current; } // modifications to the current state are performed explicitly through the API

    const fs::path& get_path() const { return path; } // returns the path, which is not necessarily the original path

    void reset() // reverts all modifications
    {
        current = original;
    }

    void set_colour(const std::string& key, const Colour& colour)
    {
        current.get_colour_data().set_colour(key, colour);
        // validation of this operation is handled by the Flavour class; if it throws an exception, it is propagated here
    }

    bool is_modified() const { return original != current; } // uses the comparison operators of the Scheme class

    static SchemeEditor from_file(const fs::path& path)
    {
        auto scheme = Scheme::from_file(path); // if loading fails, the thrown exception is propagated

        return SchemeEditor(scheme, path);
    }

    void save() // saves current to the original path; original is overwritten, which may be reconsidered later
    {
        current.save_to_file(path);
        original = current;
    }

    void save_as(const fs::path& new_path)
    {
        current.save_to_file(new_path);
        path = new_path;
        original = current; 
    }

    void reload() // reloads the editor from the current path
    {
        original = Scheme::from_file(path);
        current = original;
    }
};