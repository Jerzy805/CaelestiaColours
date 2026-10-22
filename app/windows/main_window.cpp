#include "windows/main_window.hpp"
#include "widgets/clickable_frame.hpp"

#include <QColor>
#include <QColorDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <QSignalBlocker>
#include <QInputDialog>
#include <QRegularExpression>

#include <exception>
#include <utility>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_scheme_editor = SchemeEditor::from_file(SchemePaths::caelestia_scheme());

    setWindowTitle("CaelestiaColours");
    resize(1280, 800);
    setMinimumSize(950, 600);

    auto* root = new QWidget(this);
    root->setObjectName("root");
    setCentralWidget(root);

    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(16);

    // Top bar
    auto* header = new QHBoxLayout;
    header->setSpacing(10);

    m_app_title = new QLabel("CaelestiaColours");
    m_app_title->setObjectName("appTitle");

    auto* subtitle = new QLabel("SCHEME EDITOR");
    subtitle->setObjectName("subtitle");

    auto* titleGroup = new QVBoxLayout;
    titleGroup->setSpacing(2);
    titleGroup->addWidget(m_app_title);
    titleGroup->addWidget(subtitle);

    auto* search = new QLabel("⌕   Search schemes...");
    search->setObjectName("searchPlaceholder");

    auto* newSchemeButton = new QPushButton("+  New scheme");
    newSchemeButton->setObjectName("primaryButton");
    newSchemeButton->setCursor(Qt::PointingHandCursor);

    connect(newSchemeButton, &QPushButton::clicked,
        this, &MainWindow::create_scheme);

    header->addLayout(titleGroup);
    header->addStretch();
    header->addWidget(search);
    header->addWidget(newSchemeButton);
    rootLayout->addLayout(header);

    // Main workspace
    auto* workspace = new QHBoxLayout;
    workspace->setSpacing(14);

    // Sidebar
    auto* sidebar = new QFrame;
    sidebar->setObjectName("panel");
    sidebar->setMinimumWidth(190);
    sidebar->setMaximumWidth(240);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(14, 16, 14, 14);
    sidebarLayout->setSpacing(12);

    auto* schemesTitle = new QLabel("YOUR SCHEMES");
    schemesTitle->setObjectName("sectionTitle");

    m_scheme_list = new QListWidget;
    m_scheme_list->setObjectName("schemeList");
    populate_scheme_list();
    m_scheme_list->setCurrentRow(0);

    connect(m_scheme_list, &QListWidget::itemClicked, this,
            [this](QListWidgetItem* item)
    {
        const auto path = fs::path(item->data(Qt::UserRole).toString().toStdString());
        load_scheme(path);
    });

    auto* saveButton = new QPushButton("Save");
    saveButton->setObjectName("secondaryButton");

    connect(saveButton, &QPushButton::clicked, this, [this]
    {
        try
        {
            m_scheme_editor->save();
            update_modified_state();
        }
        catch (const std::exception& e)
        {
            QMessageBox::warning(this, "Failed to save scheme", QString::fromUtf8(e.what()));
        }
    });

    sidebarLayout->addWidget(schemesTitle);
    sidebarLayout->addWidget(m_scheme_list, 1);
    sidebarLayout->addWidget(saveButton);

    // Palette preview
    auto* editor = new QFrame;
    editor->setObjectName("panel");

    auto* editorLayout = new QVBoxLayout(editor);
    editorLayout->setContentsMargins(22, 20, 22, 22);
    editorLayout->setSpacing(18);

    auto* paletteDescription = new QLabel("Your colours, organised in one place.");
    paletteDescription->setObjectName("mutedText");

    auto* paletteHeader = new QVBoxLayout;
    paletteHeader->setSpacing(5);
    paletteHeader->addWidget(paletteDescription);
    editorLayout->addLayout(paletteHeader);

    auto* colorsWidget = new QWidget;
    m_colour_grid = new QGridLayout(colorsWidget);
    m_colour_grid->setSpacing(12);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidget(colorsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("colorScrollArea");
    editorLayout->addWidget(scrollArea, 1);

    // Colour inspector
    auto* inspector = new QFrame;
    inspector->setObjectName("panel");
    inspector->setMinimumWidth(180);
    inspector->setMaximumWidth(250);

    auto* inspectorLayout = new QVBoxLayout(inspector);
    inspectorLayout->setContentsMargins(16, 18, 16, 16);
    inspectorLayout->setSpacing(14);

    auto* inspectorTitle = new QLabel("Colour details");
    inspectorTitle->setObjectName("panelTitle");

    auto* colorPickerButton = new QPushButton("Choose colour...");
    colorPickerButton->setObjectName("secondaryButton");
    colorPickerButton->setStyleSheet(
        "QPushButton { background-color: #313244; color: #cdd6f4;"
        " border: 1px solid #45475a; border-radius: 10px; padding: 10px 12px; }"
        "QPushButton:hover { background-color: #45475a; border-color: #cba6f7; }"
        "QPushButton:pressed { background-color: #585b70; }");

    auto* hexLabel = new QLabel("HEX VALUE");
    hexLabel->setObjectName("sectionTitle");

    m_selected_colour = new QFrame;
    m_selected_colour->setMinimumHeight(110);

    m_selected_name = new QLabel;
    m_selected_name->setObjectName("panelTitle");

    m_hex_value = new QLineEdit;

    auto* info = new QLabel("Select a colour from the palette to inspect its properties.");
    info->setObjectName("mutedText");
    info->setWordWrap(true);

    inspectorLayout->addWidget(inspectorTitle);
    inspectorLayout->addWidget(m_selected_colour);
    inspectorLayout->addWidget(m_selected_name);
    inspectorLayout->addWidget(hexLabel);
    inspectorLayout->addWidget(m_hex_value);
    inspectorLayout->addWidget(colorPickerButton);
    inspectorLayout->addWidget(info);
    inspectorLayout->addStretch();

    // Apply the HEX value on Enter or when the input loses focus
    connect(m_hex_value, &QLineEdit::editingFinished, this, [this]
    {
        if (!m_selectedCard)
            return;

        if (!update_card_colour(m_selectedCard, m_hex_value->text().trimmed()))
            m_hex_value->setText(m_selectedCard->hex());
    });

    connect(colorPickerButton, &QPushButton::clicked, this, [this]
    {
        if (!m_selectedCard)
            return;

        QColorDialog dialog(QColor(m_selectedCard->hex()), this);
        dialog.setWindowTitle("Choose colour");
        dialog.setOption(QColorDialog::DontUseNativeDialog, true);
            dialog.setStyleSheet(R"(
                QColorDialog {
                    background-color: #191b22;
                    color: #e2e2eb;
                }

                QLabel {
                    color: #e2e2eb;
                }

                QPushButton {
                    background-color: #313244;
                    color: #cdd6f4;
                    border: 1px solid #45475a;
                    border-radius: 8px;
                    padding: 7px 12px;
                }

                QPushButton:hover {
                    background-color: #45475a;
                    border-color: #cba6f7;
                }

                QPushButton:pressed {
                    background-color: #585b70;
                }

                QLineEdit,
                QSpinBox,
                QDoubleSpinBox {
                    background-color: #22242d;
                    color: #cdd6f4;
                    border: 1px solid #45475a;
                    border-radius: 6px;
                    padding: 5px;
                    selection-background-color: #cba6f7;
                    selection-color: #1e1e2e;
                }

                QComboBox {
                    background-color: #22242d;
                    color: #cdd6f4;
                    border: 1px solid #45475a;
                    border-radius: 6px;
                    padding: 5px;
                }

                QComboBox QAbstractItemView {
                    background-color: #22242d;
                    color: #cdd6f4;
                    selection-background-color: #45475a;
                    selection-color: #e2e2eb;
                }

                QTabWidget::pane {
                    border: 1px solid #343642;
                    background-color: #191b22;
                }

                QTabBar::tab {
                    background-color: #22242d;
                    color: #cdd6f4;
                    padding: 7px 12px;
                    border: 1px solid #343642;
                }

                QTabBar::tab:selected {
                    background-color: #373044;
                    color: #e6dcff;
                    border-color: #cba6f7;
                }
            )");

        if (dialog.exec() == QDialog::Accepted)
        {
            const QColor chosenColor = dialog.selectedColor();
            if (chosenColor.isValid())
                update_card_colour(m_selectedCard, chosenColor.name(QColor::HexRgb));
        }
    });

    workspace->addWidget(sidebar);
    workspace->addWidget(editor, 1);
    workspace->addWidget(inspector);
    rootLayout->addLayout(workspace, 1);

    // Caelestia-inspired styling
    setStyleSheet(R"(
        QWidget#root {
            background-color: #111318;
            color: #e2e2eb;
            font-family: "Inter", "Noto Sans", sans-serif;
            font-size: 13px;
        }

        QScrollArea#colorScrollArea {
            background-color: #191b22;
            border: none;
        }

        QScrollArea#colorScrollArea QWidget {
            background-color: transparent;
        }

        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 2px 0 2px 0;
        }

        QScrollBar::handle:vertical {
            background: #45475a;
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: #585b70;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: transparent;
        }

        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
            margin: 0 2px 0 2px;
        }

        QScrollBar::handle:horizontal {
            background: #45475a;
            border-radius: 4px;
            min-width: 30px;
        }

        QScrollBar::handle:horizontal:hover {
            background: #585b70;
        }

        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0;
        }

        QScrollBar::add-page:horizontal,
        QScrollBar::sub-page:horizontal {
            background: transparent;
        }

        QFrame#colorCard[selected="true"] {
            border: 2px solid #cba6f7;
        }

        QLabel#appTitle {
            color: #e6e0f5;
            font-size: 21px;
            font-weight: 650;
        }

        QLabel#subtitle, QLabel#sectionTitle {
            color: #a9a4bb;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1px;
        }

        QLabel#panelTitle {
            color: #e6e0f5;
            font-size: 17px;
            font-weight: 600;
        }

        QLabel#mutedText {
            color: #a4a2b3;
        }

        QFrame#panel {
            background-color: #191b22;
            border: 1px solid #2b2d38;
            border-radius: 16px;
        }

        QFrame#colorCard {
            background-color: #20222b;
            border: 1px solid #2b2d38;
            border-radius: 12px;
        }

        QLabel#colorName {
            color: #e2e0ec;
            font-weight: 600;
        }

        QLabel#colorHex {
            color: #a9a6b9;
            font-size: 11px;
        }

        QLabel#searchPlaceholder {
            color: #aaa6ba;
            background-color: #22242d;
            border: 1px solid #30323e;
            border-radius: 9px;
            padding: 10px;
        }

        QListWidget#schemeList {
            background: transparent;
            border: none;
            outline: none;
        }

        QListWidget#schemeList::item {
            color: #c4c1d2;
            padding: 11px 10px;
            border-radius: 9px;
            margin: 2px 0;
        }

        QListWidget#schemeList::item:selected {
            background-color: #373044;
            color: #e6dcff;
        }

        QListWidget#schemeList::item:hover:!selected {
            background-color: #252630;
        }

        QPushButton#primaryButton {
            background-color: #cba6f7;
            color: #211b2b;
            border: none;
            border-radius: 10px;
            padding: 11px 15px;
            font-weight: 650;
        }

        QPushButton#primaryButton:hover {
            background-color: #d8baff;
        }

        QPushButton#secondaryButton {
            background-color: #252630;
            color: #e6e0f5;
            border: 1px solid #3a3c48;
            border-radius: 10px;
            padding: 10px 16px;
            font-weight: 600;
        }

        QPushButton#secondaryButton:hover {
            background-color: #30313d;
            border-color: #cba6f7;
            color: #f1eaff;
        }

        QPushButton#secondaryButton:pressed {
            background-color: #373044;
            border-color: #cba6f7;
        }

        QPushButton#secondaryButton:disabled {
            background-color: #20222b;
            color: #6f6c79;
            border-color: #2b2d38;
        }
    )");
    rebuild_colour_cards();
    update_modified_state();
}

void MainWindow::update_modified_state()
{
    if (m_app_title && m_scheme_editor)
        m_app_title->setText(m_scheme_editor->is_modified()
            ? "CaelestiaColours*" : "CaelestiaColours");
}

void MainWindow::populate_scheme_list()
{
    try
    {
        const auto directory = SchemePaths::schemes_directory();
        fs::create_directories(directory);

        m_scheme_list->clear();

        // active Caelestia scheme
        const auto activePath = SchemePaths::caelestia_scheme();

        auto* activeItem = new QListWidgetItem(
            "● Active Caelestia scheme", m_scheme_list);

        activeItem->setData(
            Qt::UserRole,
            QString::fromStdString(activePath.string())
        );

        // user-created schemes
        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (!entry.is_regular_file() ||
                entry.path().extension() != ".json")
                continue;

            const auto path = entry.path();

            auto* item = new QListWidgetItem(
                QString::fromStdString(path.stem().string()),
                m_scheme_list
            );

            item->setData(
                Qt::UserRole,
                QString::fromStdString(path.string())
            );
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(
            this,
            "Failed to list schemes",
            QString::fromUtf8(e.what())
        );
    }
}

void MainWindow::load_scheme(const fs::path& path)
{
    if (m_scheme_editor && m_scheme_editor->is_modified())
    {
        const auto answer = QMessageBox::question(
            this, "Unsaved changes",
            "Discard unsaved changes and open another scheme?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (answer != QMessageBox::Yes)
            return;
    }

    try
    {
        auto editor = SchemeEditor::from_file(path);
        m_scheme_editor = std::move(editor);
        rebuild_colour_cards();
        update_modified_state();
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "Failed to load scheme", QString::fromUtf8(e.what()));
    }
}

void MainWindow::clear_colour_cards()
{
    m_selectedCard = nullptr;

    if (!m_colour_grid)
        return;

    while (auto* item = m_colour_grid->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    m_colour_cards.clear();
}

void MainWindow::rebuild_colour_cards()
{
    clear_colour_cards();

    if (!m_scheme_editor)
    {
        update_inspector();
        return;
    }

    const auto& colourData = m_scheme_editor->get_current().get_colour_data();
    const auto colourNames = colourData.get_colour_names();

    int i = 0;
    for (const auto& colourName : colourNames)
    {
        const auto& colour = colourData.get_colour(colourName);
        const auto name = QString::fromStdString(colourName);
        const auto hex = "#" + QString::fromStdString(colour.to_hex());

        auto* card = new ClickableFrame(name, hex);
        card->setObjectName("colorCard");
        card->setCursor(Qt::PointingHandCursor);

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(8, 8, 8, 10);
        cardLayout->setSpacing(8);

        auto* swatch = new QFrame;
        swatch->setMinimumHeight(76);
        swatch->setObjectName("colorSwatch");
        swatch->setStyleSheet(QString(
            "background-color: %1; border-radius: 9px;").arg(hex));

        auto* nameLabel = new QLabel(name);
        nameLabel->setObjectName("colorName");

        auto* hexLabel = new QLabel(hex.toUpper());
        hexLabel->setObjectName("colorHex");

        cardLayout->addWidget(swatch);
        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(hexLabel);

        m_colour_grid->addWidget(card, i / 4, i % 4);
        m_colour_cards.append(card);

        connect(card, &ClickableFrame::clicked, this, [this, card]
        {
            select_colour_card(card);
        });

        ++i;
    }

    select_colour_card(m_colour_cards.isEmpty() ? nullptr : m_colour_cards.front());
}

void MainWindow::select_colour_card(ClickableFrame* card)
{
    m_selectedCard = card;

    for (auto* other : m_colour_cards)
        other->setSelected(other == card);

    update_inspector();
}

void MainWindow::update_inspector()
{
    if (!m_selected_name || !m_hex_value || !m_selected_colour)
        return;

    if (!m_selectedCard)
    {
        m_selected_name->clear();
        m_hex_value->clear();
        m_hex_value->setEnabled(false);
        m_selected_colour->setStyleSheet(QString());
        return;
    }

    m_hex_value->setEnabled(true);
    m_selected_name->setText(m_selectedCard->name());
    m_hex_value->setText(m_selectedCard->hex().toUpper());
    m_selected_colour->setStyleSheet(QString(
        "background-color: %1; border-radius: 12px;").arg(m_selectedCard->hex()));
}

bool MainWindow::update_card_colour(ClickableFrame* card, const QString& value)
{
    if (!card || !m_scheme_editor)
        return false;

    const QColor color(value);
    if (!color.isValid())
        return false;

    const QString normalizedHex = color.name(QColor::HexRgb).toUpper();

    try
    {
        const auto newColour = Colour::from_hex(normalizedHex.toStdString());
        m_scheme_editor->set_colour(card->name().toStdString(), newColour);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "Failed to change colour", QString::fromUtf8(e.what()));
        return false;
    }

    card->setHex(normalizedHex);

    if (auto* swatch = card->findChild<QFrame*>("colorSwatch"))
        swatch->setStyleSheet(QString(
            "background-color: %1; border-radius: 9px;").arg(normalizedHex));

    if (auto* label = card->findChild<QLabel*>("colorHex"))
        label->setText(normalizedHex);

    update_inspector();
    update_modified_state();
    return true;
}

void MainWindow::create_scheme()
{
    if (!m_scheme_editor)
        return;

    QInputDialog dialog(this);
    dialog.setWindowTitle("New scheme");
    dialog.setLabelText("Scheme name:");
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue("");

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #171923;
            color: #E8EAF0;
        }

        QLabel {
            color: #E8EAF0;
            font-size: 13px;
        }

        QLineEdit {
            background-color: #252938;
            color: #FFFFFF;
            border: 1px solid #454B60;
            border-radius: 7px;
            padding: 8px;
            min-width: 260px;
        }

        QPushButton {
            background-color: #30364A;
            color: #FFFFFF;
            border: 1px solid #454B60;
            border-radius: 7px;
            padding: 7px 16px;
        }

        QPushButton:hover {
            background-color: #454D68;
        }
    )");

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString name = dialog.textValue().trimmed();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Invalid name", "Scheme name cannot be empty");
        return;
    }

    // allow only safe filename characters
    static const QRegularExpression validName(R"(^[A-Za-z0-9 _-]+$)");

    if (!validName.match(name).hasMatch())
    {
        QMessageBox::warning(
            this,
            "Invalid name",
            "Use only letters, numbers, spaces, underscores and hyphens"
        );
        return;
    }

    try
    {
        const auto directory = SchemePaths::schemes_directory();
        fs::create_directories(directory);

        const auto path = directory/(name.toStdString() + ".json"); 

        if (fs::exists(path))
        {
            QMessageBox::warning(this, "Scheme already exists", "A scheme with this name already exists");
            return;
        }

        // copy current scheme, including unsaved colour changes
        auto scheme = m_scheme_editor->get_current();
        scheme.set_name(name.toStdString());

        scheme.save_to_file(path);

        // switch editor to the newly created file
        m_scheme_editor = SchemeEditor::from_file(path);

        populate_scheme_list();

        // select the new scheme in the sidebar
        for (int i = 0; i < m_scheme_list->count(); i++)
        {
            auto* item = m_scheme_list->item(i);

            if (item->data(Qt::UserRole).toString() == QString::fromStdString(path.string()))
            {
                m_scheme_list->setCurrentItem(item);
                break;
            }
        }

        rebuild_colour_cards();
        update_modified_state();
    }
    catch(const std::exception& e)
    {
        QMessageBox::warning(this, "Failed to create scheme", QString::fromUtf8(e.what()));
    }
}