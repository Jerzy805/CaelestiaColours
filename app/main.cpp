#include <QApplication>
#include <QPalette>
#include <QColor>

#include "windows/main_window.hpp"

void setupApplicationTheme(QApplication& app)
{
    app.setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#171923"));
    palette.setColor(QPalette::WindowText, QColor("#E8EAF0"));
    palette.setColor(QPalette::Base, QColor("#252938"));
    palette.setColor(QPalette::AlternateBase, QColor("#30364A"));
    palette.setColor(QPalette::Text, QColor("#FFFFFF"));
    palette.setColor(QPalette::Button, QColor("#30364A"));
    palette.setColor(QPalette::ButtonText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Highlight, QColor("#626B94"));
    palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));

    app.setPalette(palette);

    app.setStyleSheet(R"(
        QMessageBox {
            background-color: #171923;
        }

        QMessageBox QLabel {
            color: #E8EAF0;
            font-size: 13px;
        }

        QMessageBox QPushButton {
            background-color: #30364A;
            color: #FFFFFF;
            border: 1px solid #454B60;
            border-radius: 7px;
            padding: 7px 16px;
            min-width: 75px;
        }

        QMessageBox QPushButton:hover {
            background-color: #454D68;
        }

        QMessageBox QPushButton:pressed {
            background-color: #252938;
        }
    )");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    setupApplicationTheme(app);

    MainWindow window;
    window.show();

    return app.exec();
}
