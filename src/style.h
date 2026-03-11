#pragma once
#include <QString>

// ─── Color Palette (GitHub-Dark inspired) ─────────────────────────────────────

namespace Style {

// Base colors
constexpr auto BG_MAIN     = "#0d1117";
constexpr auto BG_CARD     = "#161b22";
constexpr auto BG_SIDEBAR  = "#010409";
constexpr auto BORDER      = "#30363d";
constexpr auto BORDER_LIGHT= "#21262d";

// Accent colors
constexpr auto BLUE        = "#58a6ff";
constexpr auto GREEN       = "#3fb950";
constexpr auto RED         = "#f85149";
constexpr auto YELLOW      = "#d29922";
constexpr auto PURPLE      = "#bc8cff";
constexpr auto ORANGE      = "#ffa657";

// Text
constexpr auto TEXT_PRIMARY   = "#f0f6fc";
constexpr auto TEXT_SECONDARY = "#8b949e";
constexpr auto TEXT_MUTED     = "#484f58";

// Button hover
constexpr auto BTN_PRIMARY    = "#1f6feb";
constexpr auto BTN_PRIMARY_H  = "#388bfd";

inline QString appStyleSheet()
{
    return R"(
        QWidget {
            background-color: #0d1117;
            color: #f0f6fc;
            font-family: "Segoe UI", "Arial", sans-serif;
            font-size: 13px;
        }
        QMainWindow, QDialog {
            background-color: #0d1117;
        }
        /* ─── Sidebar ─── */
        #sidebar {
            background-color: #010409;
            border-right: 1px solid #21262d;
        }
        #sidebarBtn {
            background: transparent;
            color: #8b949e;
            border: none;
            text-align: left;
            padding: 10px 20px;
            font-size: 13px;
            border-radius: 0;
        }
        #sidebarBtn:hover  { background: #161b22; color: #f0f6fc; }
        #sidebarBtn:checked {
            background: #1f2937;
            color: #58a6ff;
            border-left: 3px solid #58a6ff;
        }
        /* ─── Cards ─── */
        #card {
            background-color: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
        }
        /* ─── Tables ─── */
        QTableWidget {
            background-color: #0d1117;
            alternate-background-color: #161b22;
            gridline-color: #21262d;
            border: 1px solid #30363d;
            border-radius: 6px;
            selection-background-color: #1f6feb33;
        }
        QTableWidget::item { padding: 6px 10px; border: none; }
        QTableWidget::item:selected { background: #1f6feb44; color: #f0f6fc; }
        QHeaderView::section {
            background-color: #161b22;
            color: #8b949e;
            font-weight: 600;
            font-size: 12px;
            padding: 8px 10px;
            border: none;
            border-bottom: 1px solid #30363d;
            text-transform: uppercase;
        }
        /* ─── Buttons ─── */
        QPushButton {
            background-color: #21262d;
            color: #f0f6fc;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 7px 16px;
            font-weight: 500;
        }
        QPushButton:hover  { background-color: #30363d; border-color: #8b949e; }
        QPushButton:pressed { background-color: #161b22; }
        QPushButton#btnPrimary {
            background-color: #1f6feb;
            border-color: #1f6feb;
            color: #ffffff;
        }
        QPushButton#btnPrimary:hover { background-color: #388bfd; }
        QPushButton#btnDanger {
            background-color: #da3633;
            border-color: #da3633;
            color: #ffffff;
        }
        QPushButton#btnDanger:hover { background-color: #f85149; }
        QPushButton#btnSuccess {
            background-color: #238636;
            border-color: #238636;
            color: #ffffff;
        }
        QPushButton#btnSuccess:hover { background-color: #3fb950; }
        /* ─── Inputs ─── */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox {
            background-color: #0d1117;
            color: #f0f6fc;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 6px 10px;
            selection-background-color: #1f6feb;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus,
        QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border-color: #58a6ff;
            outline: none;
        }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox QAbstractItemView {
            background: #161b22;
            border: 1px solid #30363d;
            selection-background-color: #1f6feb44;
        }
        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background: #21262d;
            border: none;
            border-radius: 3px;
        }
        QDateEdit::up-button, QDateEdit::down-button {
            background: #21262d; border: none;
        }
        /* ─── Scrollbars ─── */
        QScrollBar:vertical {
            background: #0d1117; width: 8px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #30363d; border-radius: 4px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #484f58; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal {
            background: #0d1117; height: 8px; margin: 0;
        }
        QScrollBar::handle:horizontal {
            background: #30363d; border-radius: 4px; min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background: #484f58; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        /* ─── Labels ─── */
        QLabel#titleLabel {
            font-size: 22px; font-weight: 700; color: #f0f6fc;
        }
        QLabel#subtitleLabel {
            font-size: 13px; color: #8b949e;
        }
        /* ─── Tabs ─── */
        QTabWidget::pane { border: 1px solid #30363d; border-radius: 6px; }
        QTabBar::tab {
            background: #0d1117; color: #8b949e;
            padding: 8px 18px; border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected { color: #f0f6fc; border-bottom: 2px solid #58a6ff; }
        QTabBar::tab:hover { color: #f0f6fc; }
        /* ─── MessageBox ─── */
        QMessageBox { background: #161b22; }
        QMessageBox QLabel { color: #f0f6fc; }
        /* ─── CheckBox ─── */
        QCheckBox { color: #f0f6fc; spacing: 6px; }
        QCheckBox::indicator {
            width: 16px; height: 16px;
            background: #0d1117; border: 1px solid #30363d; border-radius: 3px;
        }
        QCheckBox::indicator:checked {
            background: #1f6feb; border-color: #1f6feb;
        }
        /* ─── Tooltip ─── */
        QToolTip {
            background: #161b22; color: #f0f6fc;
            border: 1px solid #30363d; padding: 4px 8px; border-radius: 4px;
        }
        /* ─── GroupBox ─── */
        QGroupBox {
            border: 1px solid #30363d; border-radius: 6px;
            margin-top: 14px; padding-top: 10px;
            color: #8b949e; font-weight: 600; font-size: 11px;
        }
        QGroupBox::title { subcontrol-origin: margin; padding: 0 6px; }
    )";
}

} // namespace Style
