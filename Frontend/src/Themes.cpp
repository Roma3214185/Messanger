#include "interfaces/ITheme.h"

QString LightTheme::getStyleSheet() {
  return QString(R"(
        QWidget {
            background-color: #eaf4ff;
            color: #1b1b1b;
            font-family: "Inter", "Arial";
        }

        QListView {
            background-color: #f5faff;
            border: none;
            outline: 0;
            selection-background-color: #b3d4ff;
            selection-color: #1b1b1b;
            show-decoration-selected: 0;
        }

        QListView::item {
            color: #1b1b1b;
            padding: 6px 8px;
        }

        QListView::item:selected {
            background-color: #b3d4ff;
            color: #1b1b1b !important;
        }

        QListView::item:hover {
            background: transparent;
            color: #1b1b1b;
        }

        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #c7dfff;
            border-radius: 8px;
            padding: 6px 10px;
            color: #1b1b1b;
        }

        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #c7dfff;
            color: #1b1b1b;
        }

        QPushButton {
            background-color: #d0e7ff;
            border: 1px solid #b3d4ff;
            border-radius: 4px;
            padding: 4px 8px;
            color: #1b1b1b;
        }

        QPushButton:hover, QPushButton:pressed, QPushButton:checked {
            background-color: #d0e7ff;
            border: 1px solid #b3d4ff;
            color: #1b1b1b;
        }

        QLabel {
            background: transparent;
            color: #1b1b1b;
        }

        QScrollBar:vertical {
            background: #f5faff;
            width: 8px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background: #c2dcff;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #b3d4ff;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
    )");
}

QString DarkTheme::getStyleSheet() {
  return QString(R"(
        QWidget {
            background-color: #0f1c2b;
            color: #e0e6f0;
            font-family: "Inter", "Arial";
        }

        QListView {
            background-color: #142133;
            border: 1px solid #0f2a45;
            outline: 0;
            selection-background-color: #1f4a7a;
            selection-color: #ffffff;
            show-decoration-selected: 0;
        }

        QListView::item {
            color: #e0e6f0;
            padding: 6px 8px;
        }


        QListView::item:selected,
        QListView::item:selected:focus,
        QListView::item:selected:!focus {
            background-color: #1f4a7a;
            color: #ffffff !important;
        }

        QListView::item:hover {
            background: transparent;
            color: #e0e6f0;
        }

        QPushButton {
            background-color: #22385b;
            border: 1px solid #3a547d;
            color: #e0e6f0;
            padding: 4px 8px;
            border-radius: 4px;
        }

        QPushButton:hover, QPushButton:pressed, QPushButton:checked {
            background-color: #22385b;
            border: 1px solid #3a547d;
            color: #e0e6f0;
        }

        QLabel {
            background: transparent;
            color: #e0e6f0;
        }

        QScrollBar:vertical {
            background: #142133;
            width: 8px;
            border-radius: 4px;
        }

        QTextEdit {
            background-color: #142133;
            border: 1px solid #0f2a45;
            color: #e0e6f0;
        }

        QLineEdit {
            background-color: #142133;
            border: 1px solid #0f2a45;
            color: #e0e6f0;
        }

        QScrollBar::handle:vertical {
            background: #2a3b55;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #3a547d;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
    )");
}
