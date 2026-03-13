#include "mainwindow.h"

#include <QApplication>
#include <QString>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString darkAngryStyle = R"(
        /* 1. Global Window & Text */
        QWidget {
            background-color: #121212; /* Deep dark grey background */
            color: #ffffff;            /* Crisp white text */
            font-family: "Segoe UI", Arial, sans-serif;
        }

        /* 2. Text Boxes & Combo Boxes */
        QTextEdit, QComboBox {
            background-color: #1e1e1e;
            border: 2px solid #333333;
            border-radius: 6px;
            padding: 5px;
            color: #ffffff;
        }
        QTextEdit:focus, QComboBox:focus {
            border: 2px solid #d32f2f; /* Angry Red Focus Ring! */
            background-color: #252525;
        }

        /* 3. Buttons */
        QPushButton {
            background-color: #2b2b2b;
            border: 2px solid #d32f2f; /* Red border */
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            color: #ffffff;
        }
        QPushButton:hover {
            background-color: #d32f2f; /* Fills red when hovered */
            color: #ffffff;
        }
        QPushButton:pressed {
            background-color: #b71c1c; /* Darker red when clicked */
            border: 2px solid #b71c1c;
        }

        /* 4. The Progress Bar */
        QProgressBar {
            background-color: #1e1e1e;
            border: 1px solid #333333;
            border-radius: 6px;
            text-align: center; /* Puts the percentage text in the middle */
            color: white;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background-color: #d32f2f; /* Red loading bar */
            border-radius: 5px;
        }

        /* 5. The Tabs */
        QTabWidget::pane {
            border: 1px solid #333333;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #1e1e1e;
            border: 1px solid #333333;
            padding: 10px 25px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        QTabBar::tab:selected {
            background-color: #121212;
            border-bottom: none;
            border-top: 3px solid #d32f2f; /* Cool red stripe on the active tab */
        }
        QTabBar::tab:hover:!selected {
            background-color: #2b2b2b;
        }

        /* 6. Checkbox */
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #333333;
            border-radius: 4px;
            background-color: #1e1e1e;
        }
        QCheckBox::indicator:checked {
            background-color: #d32f2f; /* Red checkbox */
            border: 2px solid #d32f2f;
        }
        QCheckBox::indicator:hover {
            border: 2px solid #d32f2f;
        }
    )";


    a.setStyleSheet(darkAngryStyle);

    MainWindow w;
    w.show();
    return a.exec();
}
