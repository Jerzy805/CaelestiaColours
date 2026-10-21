#pragma once

#include "flavour.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

class Scheme
{
    std::string name;
    std::string flavour;
    std::string mode;
    std::string variant;
    Flavour colour_data; // now here are stored key-value elements, depends on the implementation of Flavour, now eg. background-#FF0000


    public:

    const std::string& get_name() const { return name; }
    const std::string& get_flavour() const { return flavour; }
    const std::string& get_mode() const { return mode; }
    const std::string& get_variant() const { return variant; }

    Flavour& get_colour_data()
    {
        return colour_data;
    }

    const Flavour& get_colour_data() const
    {
        return colour_data;
    }

    nl::json to_json() const
    {
        nl::json data;

        data["name"] = name;
        data["flavour"] = flavour;
        data["mode"] = mode;
        data["variant"] = variant;
        data["colours"] = colour_data.to_json();

        return data;
    }

    void save_to_file(const fs::path& path) const
    {
        std::ofstream file(path);

        if (!file)
        {
            throw std::runtime_error("Failed to open output file");
        }

        file << to_json().dump(4);
        file.flush(); // just to make sure our stream checking makes sense

        if (!file) // checking if stream is correct after saving data
        {
            throw std::runtime_error("Failed to save data");
        }
    }

    static Scheme from_file(const fs::path& path)
    {
        std::ifstream file(path);

        if (!file)
        {
            throw std::runtime_error("Failed to open input file");
        }

        nl::json data;
        file >> data;
    
        // JSON structure validation
        if (!data.is_object() || !data.contains("name") || !data["name"].is_string()
            || !data.contains("flavour") || !data["flavour"].is_string()
            || !data.contains("mode") || !data["mode"].is_string()
            || !data.contains("variant") || !data["variant"].is_string()
            || !data.contains("colours") || !data["colours"].is_object())
        {
            throw std::runtime_error("Invalid scheme file structure");
        }

        Scheme scheme;

        scheme.name = data["name"].get<std::string>();
        scheme.flavour = data["flavour"].get<std::string>();
        scheme.mode = data["mode"].get<std::string>();
        scheme.variant = data["variant"].get<std::string>();
        
        for (const auto& [color_name, color_value] : data["colours"].items())
        {
            scheme.colour_data.add_colour(color_name, Colour::from_hex(color_value)); // using Flavour class API
        }

        return scheme;
    }

    bool operator==(const Scheme& other) const
    {
        return name == other.name &&
            flavour == other.flavour &&
            mode == other.mode &&
            colour_data == other.colour_data &&
            variant == other.variant;
    }

    bool operator!=(const Scheme& other) const
    {
        return !(*this == other);
    }
};