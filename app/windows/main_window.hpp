#pragma once

#include <QMainWindow>
#include <QLabel>

#include <caelestia_colors/scheme_paths.hpp>
#include <caelestia_colors/scheme_editor.hpp>
#include <optional>

class ClickableFrame;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    ClickableFrame* m_selectedCard = nullptr;
    std::optional<SchemeEditor> m_scheme_editor;
    QLabel* m_app_title = nullptr; // "title" label, we're using it to display whether the state of scheme is changed
    void populate_scheme_list();
    QListWidget* m_scheme_list{nullptr};
    void load_scheme(const fs::path& path);
    void rebuild_colour_card();
};