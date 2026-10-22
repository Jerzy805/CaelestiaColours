
#include <caelestia_colors/scheme_editor.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace
{
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string& test_name)
    {
        if (condition)
        {
            ++passed;
            std::cout << "[PASS] " << test_name << '\n';
        }
        else
        {
            ++failed;
            std::cerr << "[FAIL] " << test_name << '\n';
        }
    }

    template <typename Function>
    void check_throws(Function&& function, const std::string& test_name)
    {
        try
        {
            function();
            check(false, test_name);
        }
        catch (const std::exception&)
        {
            check(true, test_name);
        }
    }

    struct TestFiles
    {
        fs::path directory =
            fs::temp_directory_path() / "caelestia_colors_tests";

        fs::path original = directory / "original.json";
        fs::path second = directory / "second.json";
        fs::path invalid = directory / "invalid.json";
        fs::path missing_colours = directory / "missing_colours.json";
        fs::path wrong_type = directory / "wrong_type.json";

        TestFiles()
        {
            fs::create_directories(directory);

            std::ofstream file(original);

            file << R"({
                "name": "Test Scheme",
                "flavour": "test",
                "mode": "dark",
                "variant": "tonalspot",
                "colours": {
                    "background": "#112233",
                    "primary": "#AABBCC"
                }
            })";
        }

        ~TestFiles()
        {
            std::error_code error;
            fs::remove_all(directory, error);
        }
    };
}

int main()
{
    TestFiles files;

    // 1. Loading a valid file
    check_throws(
        [] { SchemeEditor::from_file(fs::path{}); },
        "Empty path causes loading to fail"
    );

    auto editor = SchemeEditor::from_file(files.original);

    check(
        editor.get_current().get_name() == "Test Scheme",
        "Scheme name loaded"
    );

    check(
        !editor.is_modified(),
        "Newly loaded scheme is not modified"
    );

    // 2. Modification and reset
    const auto original_colour =
        editor.get_current()
              .get_colour_data()
              .get_colour("primary");

    editor.set_colour("primary", Colour::from_hex("#010203"));

    check(
        editor.is_modified(),
        "Changing a colour sets is_modified"
    );

    editor.reset();

    check(
        !editor.is_modified(),
        "Reset restores the original state"
    );

    check(
        editor.get_current()
              .get_colour_data()
              .get_colour("primary") == original_colour,
        "Reset restores the original colour"
    );

    // 3. Non-existent colour
    check_throws(
        [&] {
            editor.set_colour(
                "unknown",
                Colour::from_hex("#010203")
            );
        },
        "Cannot change a non-existent colour"
    );

    // 4. Save and reload
    editor.set_colour("primary", Colour::from_hex("#010203"));
    editor.save();

    check(
        !editor.is_modified(),
        "Scheme is not modified after save"
    );

    auto loaded = SchemeEditor::from_file(files.original);

    check(
        loaded.get_current()
              .get_colour_data()
              .get_colour("primary") ==
              Colour::from_hex("#010203"),
        "Saved data can be loaded again"
    );

    // Verify metadata survives serialization
    {
        std::ifstream file(files.original);

        nlohmann::json json;
        file >> json;

        check(
            json.value("name", "") == "Test Scheme",
            "Scheme name preserved after save"
        );

        check(
            json.value("flavour", "") == "test",
            "Scheme flavour preserved after save"
        );

        check(
            json.value("mode", "") == "dark",
            "Scheme mode preserved after save"
        );

        check(
            json.value("variant", "") == "tonalspot",
            "Scheme variant preserved after save"
        );
    }

    // 5. Save as
    editor.set_colour(
        "background",
        Colour::from_hex("#FFFFFF")
    );

    editor.save_as(files.second);

    check(
        editor.get_path() == files.second,
        "save_as updates the path"
    );

    check(
        !editor.is_modified(),
        "Scheme is not modified after save_as"
    );

    check(
        fs::exists(files.second),
        "save_as creates the destination file"
    );

    // Verify the saved copy contains the modified colour
    {
        auto copied = SchemeEditor::from_file(files.second);

        check(
            copied.get_current()
                  .get_colour_data()
                  .get_colour("background") ==
                  Colour::from_hex("#FFFFFF"),
            "save_as preserves modified colours"
        );
    }

    // 6. Reload after an external file change
    {
        std::ofstream file(files.second);

        file << R"({
            "name": "Reloaded",
            "flavour": "test",
            "mode": "light",
            "variant": "tonalspot",
            "colours": {
                "background": "#000000",
                "primary": "#FFFFFF"
            }
        })";
    }

    editor.reload();

    check(
        editor.get_current().get_name() == "Reloaded",
        "reload reads the new name"
    );

    check(
        !editor.is_modified(),
        "State is clean after reload"
    );

    check(
        editor.get_current()
              .get_colour_data()
              .get_colour("primary") ==
              Colour::from_hex("#FFFFFF"),
        "reload reads externally modified colours"
    );

    // 7. Invalid JSON
    {
        std::ofstream file(files.invalid);
        file << "{ invalid json";
    }

    check_throws(
        [&] {
            SchemeEditor::from_file(files.invalid);
        },
        "Invalid JSON throws an exception"
    );

    // 8. Failed reload should preserve state
    editor.set_colour(
        "primary",
        Colour::from_hex("#010203")
    );

    const auto before_failed_reload =
        editor.get_current();

    {
        std::ofstream file(files.second);
        file << "{ invalid json";
    }

    check_throws(
        [&] {
            editor.reload();
        },
        "Failed reload throws an exception"
    );

    check(
        editor.get_current() == before_failed_reload,
        "Failed reload preserves current"
    );

    check(
        editor.is_modified(),
        "Failed reload preserves modification status"
    );

    // 9. Failed save should preserve editor state
    editor.set_colour(
        "primary",
        Colour::from_hex("#ABCDEF")
    );

    const auto before_failed_save =
        editor.get_current();

    const auto path_before_failed_save =
        editor.get_path();

    // A directory cannot be used as an output file.
    check_throws(
        [&] {
            editor.save_as(files.directory);
        },
        "Saving to a directory throws an exception"
    );

    check(
        editor.get_current() == before_failed_save,
        "Failed save_as preserves current"
    );

    check(
        editor.get_path() == path_before_failed_save,
        "Failed save_as preserves the path"
    );

    check(
        editor.is_modified(),
        "Failed save_as preserves modification status"
    );

    // 10. Invalid JSON structure
    {
        std::ofstream file(files.missing_colours);

        file << R"({
            "name": "Broken",
            "flavour": "test",
            "mode": "dark",
            "variant": "tonalspot"
        })";
    }

    check_throws(
        [&] {
            SchemeEditor::from_file(files.missing_colours);
        },
        "Missing colours field throws an exception"
    );

    {
        std::ofstream file(files.wrong_type);

        file << R"({
            "name": "Broken",
            "flavour": "test",
            "mode": "dark",
            "variant": "tonalspot",
            "colours": {
                "primary": 123
            }
        })";
    }

    check_throws(
        [&] {
            SchemeEditor::from_file(files.wrong_type);
        },
        "Invalid colour value type throws an exception"
    );

    // 11. Missing variant field
    {
        std::ofstream file(files.invalid);

        file << R"({
            "name": "Missing Variant",
            "flavour": "test",
            "mode": "dark",
            "colours": {
                "primary": "#AABBCC"
            }
        })";
    }

    check_throws(
        [&] {
            SchemeEditor::from_file(files.invalid);
        },
        "Missing variant field throws an exception"
    );

    // Final summary
    std::cout
        << "\nResult: "
        << passed << " passed, "
        << failed << " failed tests.\n";

    return failed == 0 ? 0 : 1;
}