#pragma once

#include <QMainWindow>
#include <QVector>
#include <caelestia_colors/scheme_paths.hpp>
#include <caelestia_colors/scheme_editor.hpp>
#include <optional>

class ClickableFrame;
class QFrame;
class QGridLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void populate_scheme_list();
    void select_current_scheme_in_list();
    void update_action_buttons();
    bool is_editing_active_scheme() const;
    void create_scheme();
    void apply_current_scheme();
    void restore_previous_scheme();
    void delete_current_scheme();
    void load_scheme(const fs::path& path);
    void rebuild_colour_cards();
    void clear_colour_cards();
    void select_colour_card(ClickableFrame* card);
    bool update_card_colour(ClickableFrame* card, const QString& value);
    void update_inspector();
    void update_modified_state();

    std::optional<SchemeEditor> m_scheme_editor;
    QListWidget* m_scheme_list{nullptr};
    QPushButton* m_save_button{nullptr};
    QPushButton* m_apply_button{nullptr};
    QPushButton* m_restore_button{nullptr};
    QPushButton* m_delete_button{nullptr};
    QGridLayout* m_colour_grid{nullptr};
    QVector<ClickableFrame*> m_colour_cards;
    ClickableFrame* m_selectedCard{nullptr};

    QLabel* m_app_title{nullptr};
    QLabel* m_read_only_label{nullptr};
    QLabel* m_selected_name{nullptr};
    QLineEdit* m_hex_value{nullptr};
    QFrame* m_selected_colour{nullptr};
};