
cmake_minimum_required(VERSION 3.25)

project(CaelestiaColours
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CTest)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(nlohmann_json 3.11 REQUIRED)

qt_standard_project_setup()

# GUI application
qt_add_executable(caelestia_colours
    app/main.cpp
    app/windows/main_window.cpp
    app/windows/main_window.hpp
    app/widgets/clickable_frame.hpp
)

target_include_directories(caelestia_colours
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/app
)

target_link_libraries(caelestia_colours
    PRIVATE
        Qt6::Widgets
        nlohmann_json::nlohmann_json
)

target_compile_options(caelestia_colours
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# Install executable
install(TARGETS caelestia_colours
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES packaging/caelestia-colours.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# Install application icon
install(
    FILES assets/caelestia-colours-512.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
    RENAME caelestia-colours.png
)

# Backend tests
if(BUILD_TESTING)
    add_executable(scheme_editor_test
        tests/scheme_editor_test.cpp
    )

    target_include_directories(scheme_editor_test
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(scheme_editor_test
        PRIVATE
            nlohmann_json::nlohmann_json
    )

    target_compile_options(scheme_editor_test
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

    add_test(
        NAME scheme_editor_test
        COMMAND scheme_editor_test
    )
endif()