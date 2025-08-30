#ifndef UISTYLEMANAGER_H
#define UISTYLEMANAGER_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QApplication>
#include <QScreen>

class UIStyleManager : public QObject
{
    Q_OBJECT

public:
    explicit UIStyleManager(QObject *parent = nullptr);
    ~UIStyleManager();

    // 应用全局样式表
    static void applyGlobalStyleSheet(QApplication *app);
    
    // 跨平台DPI优化
    static void optimizeForHighDPI(QApplication *app);
    
    // 跨平台统一颜色方案
    struct ColorScheme {
        QString primary = "#007ACC";        // 主色调 - 现代蓝色
        QString primaryDark = "#005A9E";    // 深蓝色
        QString secondary = "#6C757D";      // 次要色 - 中性灰
        QString background = "#F8F9FA";     // 背景色 - 极浅灰
        QString surface = "#FFFFFF";       // 表面色 - 纯白
        QString text = "#212529";          // 文字色 - 深灰
        QString textSecondary = "#6C757D";  // 次要文字色 - 中灰
        QString success = "#28A745";       // 成功色 - 绿色
        QString warning = "#FFC107";       // 警告色 - 黄色
        QString error = "#DC3545";         // 错误色 - 红色
        QString border = "#DEE2E6";        // 边框色 - 浅灰
        QString shadow = "rgba(0, 0, 0, 0.1)"; // 阴影色
        QString hover = "#F8F9FA";         // 悬停背景 - 浅灰
        QString accent = "#17A2B8";        // 强调色 - 青色
    };

    static ColorScheme colors;

    // 控件样式应用方法
    static void applyButtonStyle(QPushButton *button, const QString &variant = "primary");
    static void applyLabelStyle(QLabel *label, const QString &variant = "normal");
    static void applyFrameStyle(QFrame *frame, bool withShadow = true);
    static void applyGroupBoxStyle(QGroupBox *groupBox);
    static void applyLineEditStyle(QLineEdit *lineEdit);
    static void applyTextEditStyle(QTextEdit *textEdit);
    static void applyScrollAreaStyle(QScrollArea *scrollArea);
    
    // 阴影效果
    static QGraphicsDropShadowEffect* createShadowEffect(QWidget *parent = nullptr);
    
    // 布局辅助
    static void applyContainerSpacing(QWidget *container);
    
    // 字体设置
    static void setupFonts();

private:
    // 获取样式表字符串
    static QString getGlobalStyleSheet();
    static QString getButtonStyleSheet(const QString &variant);
    static QString getLabelStyleSheet(const QString &variant);
    static QString getFrameStyleSheet();
    static QString getGroupBoxStyleSheet();
    static QString getLineEditStyleSheet();
    static QString getTextEditStyleSheet();
    static QString getScrollAreaStyleSheet();
};

#endif // UISTYLEMANAGER_H 