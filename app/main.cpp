#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPen>
#include <QIcon>

#include "windows/main_window.hpp"

class CaelestiaStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(
        StyleHint hint,
        const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr,
        QStyleHintReturn* returnData = nullptr
    ) const override
    {
        // keep dialog buttons text-only, but preserve Qt keyboard mnemonics
        if (hint == QStyle::SH_DialogButtonBox_ButtonsHaveIcons)
            return 0;

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }

    void drawPrimitive(
        PrimitiveElement element,
        const QStyleOption* option,
        QPainter* painter,
        const QWidget* widget = nullptr
    ) const override
    {
        if (option && (element == QStyle::PE_IndicatorSpinUp ||
                       element == QStyle::PE_IndicatorSpinDown))
        {
            // draw high-contrast spinbox arrows on dark backgrounds
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(QPen(
                option->state & QStyle::State_Enabled
                    ? QColor("#E8EAF0")
                    : QColor("#777B8A"),
                1.6,
                Qt::SolidLine,
                Qt::RoundCap,
                Qt::RoundJoin
            ));
            painter->setBrush(Qt::NoBrush);

            const QPointF c = option->rect.center();
            const qreal halfWidth = 3.0;
            const qreal halfHeight = 1.8;

            QPolygonF arrow;
            if (element == QStyle::PE_IndicatorSpinUp)
            {
                arrow << QPointF(c.x() - halfWidth, c.y() + halfHeight)
                      << QPointF(c.x(), c.y() - halfHeight)
                      << QPointF(c.x() + halfWidth, c.y() + halfHeight);
            }
            else
            {
                arrow << QPointF(c.x() - halfWidth, c.y() - halfHeight)
                      << QPointF(c.x(), c.y() + halfHeight)
                      << QPointF(c.x() + halfWidth, c.y() - halfHeight);
            }

            painter->drawPolyline(arrow);
            painter->restore();
            return;
        }

        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

void setupApplicationTheme(QApplication& app)
{
    app.setStyle(new CaelestiaStyle("Fusion"));

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

    app.setWindowIcon(QIcon::fromTheme("caelestia-colours"));

    setupApplicationTheme(app);

    MainWindow window;
    window.show();

    return app.exec();
}
