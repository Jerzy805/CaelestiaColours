
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
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <QScrollArea>
#include <cstdlib>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_scheme_editor = SchemeEditor::from_file(SchemePaths::caelestia_scheme());

    auto updateModifiedState = [this]()
    {
        m_app_title->setText(m_scheme_editor->is_modified()
            ? "CaelestiaColours*" : "CaelestiaColours");
    };

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

    m_app_title = new QLabel("CaelestiaColours"); // label title
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

    auto* saveButton = new QPushButton("Save");
    saveButton->setObjectName("secondaryButton");

    connect(saveButton, &QPushButton::clicked, this, [this, updateModifiedState]
    {
        m_scheme_editor->save();
        updateModifiedState();
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

    auto* paletteDescription = new QLabel(
        "Your colours, organised in one place."
    );
    paletteDescription->setObjectName("mutedText");

    auto* paletteHeader = new QVBoxLayout;
    paletteHeader->setSpacing(5);
    paletteHeader->addWidget(paletteDescription);

    editorLayout->addLayout(paletteHeader);
    

    auto* colorsWidget = new QWidget;
    auto* colors = new QGridLayout(colorsWidget);

    colors->setSpacing(12);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidget(colorsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("colorScrollArea");

    const auto& colourData = m_scheme_editor->get_current().get_colour_data();
    
    const auto colourNames = colourData.get_colour_names();

    QVector<ClickableFrame*> colourCards;

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

        swatch->setStyleSheet(QString("background-color: %1;"
                "border-radius: 9px;").arg(hex));

        auto* nameLabel = new QLabel(name);
        nameLabel->setObjectName("colorName");

        auto* hexLabel = new QLabel(hex.toUpper());
        hexLabel->setObjectName("colorHex");

        cardLayout->addWidget(swatch);
        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(hexLabel);

        colors->addWidget(card, i / 4, i % 4);
        colourCards.append(card);

        ++i;
    }

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
    "QPushButton {"
    "  background-color: #313244;"
    "  color: #cdd6f4;"
    "  border: 1px solid #45475a;"
    "  border-radius: 10px;"
    "  padding: 10px 12px;"
    "}"
    "QPushButton:hover {"
    "  background-color: #45475a;"
    "  border-color: #cba6f7;"
    "}"
    "QPushButton:pressed {"
    "  background-color: #585b70;"
    "}");

    auto* hexLabel = new QLabel("HEX VALUE");
    hexLabel->setObjectName("sectionTitle");

    auto* selectedColor = new QFrame;
    selectedColor->setMinimumHeight(110);

    auto* selectedName = new QLabel;
    selectedName->setObjectName("panelTitle");

    auto* hexValue = new QLineEdit;

    auto* info = new QLabel(
        "Select a colour from the palette to inspect its properties."
    );
    info->setObjectName("mutedText");
    info->setWordWrap(true);

    inspectorLayout->addWidget(inspectorTitle);
    inspectorLayout->addWidget(selectedColor);
    inspectorLayout->addWidget(selectedName);
    inspectorLayout->addWidget(hexLabel);
    inspectorLayout->addWidget(hexValue);
    inspectorLayout->addWidget(colorPickerButton);
    inspectorLayout->addWidget(info);
    inspectorLayout->addStretch();

        // Set the initially selected colour card.
    m_selectedCard = colourCards.isEmpty() ? nullptr : colourCards.front();

    for (auto* card : colourCards)
    {
        card->setSelected(card == m_selectedCard);
    }

    // Update the selected colour across the GUI.
    auto updateCardColor = [this, selectedName, hexValue, selectedColor, updateModifiedState](
        ClickableFrame* card, const QString& value)
    {
        QColor color(value);

        if (!color.isValid())
        {
            return false;
        }

        const QString normalizedHex = color.name(QColor::HexRgb).toUpper();

        const auto newColour = Colour::from_hex(normalizedHex.toStdString());

        m_scheme_editor->set_colour(card->name().toStdString(), newColour); // colour actually changed but not saved yet

        updateModifiedState();

        card->setHex(normalizedHex);

        if (auto* swatch = card->findChild<QFrame*>("colorSwatch"))
        {
            swatch->setStyleSheet(
                QString(
                    "background-color: %1;"
                    "border-radius: 9px;"
                ).arg(normalizedHex)
            );
        }

        if (auto* hexLabel = card->findChild<QLabel*>("colorHex"))
        {
            hexLabel->setText(normalizedHex);
        }

        selectedName->setText(card->name());
        hexValue->setText(normalizedHex);

        selectedColor->setStyleSheet(
            QString(
                "background-color: %1;"
                "border-radius: 12px;"
            ).arg(normalizedHex)
        );

        return true;
    };

    // Select a colour card and update the inspector.
    for (auto* card : colourCards)
    {
        connect(card, &ClickableFrame::clicked, this,
            [this, card, colourCards, updateCardColor]
            {
                m_selectedCard = card;

                for (auto* otherCard : colourCards)
                {
                    otherCard->setSelected(otherCard == card);
                }

                updateCardColor(card, card->hex());
            }
        );
    }

    // Apply the HEX value when Enter is pressed.
    connect(hexValue, &QLineEdit::returnPressed, this,
        [this, hexValue, updateCardColor]
        {
            if (m_selectedCard == nullptr)
            {
                return;
            }

            if (!updateCardColor(m_selectedCard, hexValue->text().trimmed()))
            {
                hexValue->setText(m_selectedCard->hex());
            }
        }
    );

    // Validate the HEX value when the input loses focus.
    connect(hexValue, &QLineEdit::editingFinished, this,
        [this, hexValue, updateCardColor]
        {
            if (m_selectedCard == nullptr)
            {
                return;
            }

            if (!updateCardColor(m_selectedCard, hexValue->text().trimmed()))
            {
                hexValue->setText(m_selectedCard->hex());
            }
        }
    );

    // Open the colour picker dialog.
    connect(colorPickerButton, &QPushButton::clicked, this,
        [this, updateCardColor]
        {
            if (m_selectedCard == nullptr)
            {
                return;
            }

            QColorDialog dialog(
                QColor(m_selectedCard->hex()),
                this
            );

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
                {
                    updateCardColor(
                        m_selectedCard,
                        chosenColor.name(QColor::HexRgb)
                    );
                }
            }
        }
    );

    // Synchronize the inspector with the initial selection.
    if (m_selectedCard != nullptr)
    {
        updateCardColor(m_selectedCard, m_selectedCard->hex());
    }

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
}

void MainWindow::populate_scheme_list()
{
    const auto directory = SchemePaths::schemes_directory();

    fs::create_directories(directory);

    m_scheme_list->clear();

    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
        {
            continue;
        }

        const auto path = entry.path();

        auto* item = new QListWidgetItem(
            QString::fromStdString(path.stem().string()),
            m_scheme_list
        );

        // store the file path independently of the displayed name
        item->setData(
            Qt::UserRole,
            QString::fromStdString(path.string())
        );
    }
}