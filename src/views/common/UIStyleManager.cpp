#include "UIStyleManager.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScreen>
#include <QSysInfo>
#include <QDebug>

// 静态成员初始化
UIStyleManager::ColorScheme UIStyleManager::colors;

UIStyleManager::UIStyleManager(QObject *parent)
    : QObject(parent)
{
}

UIStyleManager::~UIStyleManager()
{
}

void UIStyleManager::applyGlobalStyleSheet(QApplication *app)
{
    setupFonts();
    optimizeForHighDPI(app);
    app->setStyleSheet(getGlobalStyleSheet());
}

void UIStyleManager::optimizeForHighDPI(QApplication *app)
{
    // Qt6 中高DPI缩放默认开启，只在Qt5中需要手动设置
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        app->setAttribute(Qt::AA_EnableHighDpiScaling, true);
        app->setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    #endif
    
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        app->setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif
#endif

    // 获取屏幕DPI信息用于调试和微调
    QScreen *screen = app->primaryScreen();
    if (screen) {
        qreal dpi = screen->logicalDotsPerInch();
        qreal scale = dpi / 96.0; // 96 DPI 为标准
        
        qDebug() << "屏幕DPI:" << dpi << "缩放比例:" << scale << "平台:" << QSysInfo::productType();
        
        // 在高DPI显示器上微调字体大小
        if (scale > 1.5) {
            QFont font = app->font();
            int newSize = qRound(font.pointSize() * 1.1);
            font.setPointSize(newSize);
            app->setFont(font);
            qDebug() << "高DPI模式: 调整字体大小为" << newSize;
        }
    }
}

QString UIStyleManager::getGlobalStyleSheet()
{
    return QString(
        // 跨平台全局样式
        "QWidget {"
        "    color: %1;"
        "    font-family: system-ui, -apple-system, 'Segoe UI', 'Microsoft YaHei UI', 'Ubuntu', 'Noto Sans CJK SC', sans-serif;"
        "    font-size: 14px;"
        "    background-color: %2;"
        "    selection-background-color: %5;"
        "    selection-color: white;"
        "}"
        
        // 主窗口样式
        "QMainWindow {"
        "    background-color: %2;"
        "    border: none;"
        "}"
        
        // 跨平台按钮样式
        "QPushButton {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    color: %1;"
        "    min-height: 24px;"
        "    outline: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: %9;"
        "    border-color: %5;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %5;"
        "    border-color: %6;"
        "    color: white;"
        "}"
        "QPushButton:disabled {"
        "    background-color: %7;"
        "    color: %8;"
        "    border-color: %7;"
        "    opacity: 0.6;"
        "}"
        
        // 标签样式
        "QLabel {"
        "    color: %1;"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 5px;"
        "}"
        
        // 跨平台输入框样式
        "QLineEdit {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    color: %1;"
        "    outline: none;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %5;"
        "    border-width: 2px;"
        "}"
        "QLineEdit:hover {"
        "    border-color: %5;"
        "}"
        
        // 文本编辑器样式
        "QTextEdit {"
        "    background-color: %3;"
        "    border: 2px solid %4;"
        "    border-radius: 6px;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "    color: %1;"
        "    selection-background-color: %5;"
        "}"
        "QTextEdit:focus {"
        "    border-color: %5;"
        "    outline: none;"
        "}"
        
        // 分组框样式
        "QGroupBox {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: %1;"
        "    border: 2px solid %4;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background-color: %3;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 8px 0 8px;"
        "    color: %5;"
        "    background-color: %3;"
        "}"
        
        // 滚动区域样式
        "QScrollArea {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar:vertical {"
        "    background: %2;"
        "    width: 12px;"
        "    border-radius: 6px;"
        "    margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: %7;"
        "    border-radius: 6px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: %4;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        
        // 框架样式
        "QFrame {"
        "    background-color: %3;"
        "    border-radius: 8px;"
        "    border: 1px solid %4;"
        "}"
        
        // 列表视图样式
        "QListWidget {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 6px;"
        "    padding: 5px;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    margin: 1px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: %5;"
        "    color: white;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: %2;"
        "}"
        
        // 表格样式
        "QTableWidget {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 6px;"
        "    gridline-color: %4;"
        "    outline: none;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border: none;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: %5;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: %2;"
        "    color: %1;"
        "    border: 1px solid %4;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        
        // 跨平台选项卡样式
        "QTabWidget::pane {"
        "    border: 1px solid %4;"
        "    border-radius: 4px;"
        "    background-color: %3;"
        "    margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "    background-color: %2;"
        "    border: 1px solid %4;"
        "    padding: 10px 16px;"
        "    margin-right: 2px;"
        "    border-radius: 4px 4px 0 0;"
        "    min-width: 80px;"
        "    font-size: 14px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: %3;"
        "    border-bottom-color: %3;"
        "    font-weight: 600;"
        "    color: %5;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "    background-color: %9;"
        "}"
        
        // 工具提示样式
        "QToolTip {"
        "    background-color: %1;"
        "    color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    font-size: 12px;"
        "}"
    )
    .arg(colors.text)        // %1 - 文字色
    .arg(colors.background)   // %2 - 背景色
    .arg(colors.surface)      // %3 - 表面色
    .arg(colors.border)       // %4 - 边框色
    .arg(colors.primary)      // %5 - 主色调
    .arg(colors.primaryDark)  // %6 - 深橙色
    .arg(colors.secondary)    // %7 - 次要色
    .arg(colors.textSecondary) // %8 - 次要文字色
    .arg(colors.hover);       // %9 - 悬停背景
}

void UIStyleManager::setupFonts()
{
    QFont defaultFont;
    
    // 跨平台字体选择策略
    QStringList fontFamilies;
    
#ifdef Q_OS_WIN
    // Windows 字体优先级
    fontFamilies = {"Microsoft YaHei UI", "Microsoft YaHei", "Segoe UI", "Arial", "sans-serif"};
#elif defined(Q_OS_MAC)
    // macOS 字体优先级
    fontFamilies = {"PingFang SC", "Helvetica Neue", "San Francisco", "Arial", "sans-serif"};
#else
    // Linux/Ubuntu 字体优先级
    fontFamilies = {"Ubuntu", "Noto Sans CJK SC", "Source Han Sans SC", 
                    "WenQuanYi Micro Hei", "DejaVu Sans", "Liberation Sans", "Arial", "sans-serif"};
#endif
    
    for (const QString &family : fontFamilies) {
        if (QFontDatabase::families().contains(family)) {
            defaultFont.setFamily(family);
            qDebug() << "选择字体:" << family << "平台:" << QSysInfo::productType();
            break;
        }
    }
    
    // 跨平台字体大小设置
#ifdef Q_OS_WIN
    defaultFont.setPointSize(9);  // Windows 使用较小字体
#elif defined(Q_OS_MAC)
    defaultFont.setPointSize(13); // macOS 使用较大字体
#else
    defaultFont.setPointSize(10); // Linux 使用中等字体
#endif
    
    defaultFont.setWeight(QFont::Normal);
    defaultFont.setHintingPreference(QFont::PreferFullHinting);
    
    QApplication::setFont(defaultFont);
}

void UIStyleManager::applyButtonStyle(QPushButton *button, const QString &variant)
{
    if (!button) return;
    
    // button->setStyleSheet(getButtonStyleSheet(variant));

    button->setStyleSheet(R"(QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e0e0e0, stop:1 #ffffff);
    border: 1px solid #dcdcdc;
    padding: 5px 15px;
    border-radius: 5px;
    color: #333;
    font-size: 14px;
    font-weight: bold;
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d0d0d0, stop:1 #f0f0f0);
}

QPushButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c0c0c0, stop:1 #e0e0e0);
    border: 1px solid #a0a0a0;
})");
    button->setMinimumHeight(36);
    button->setCursor(Qt::PointingHandCursor);
}

QString UIStyleManager::getButtonStyleSheet(const QString &variant)
{
    QString baseStyle = QString(
        "QPushButton {"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    border: 2px solid %1;"
        "    min-height: 16px;"
        "}"
        "QPushButton:disabled {"
        "    background-color: %2;"
        "    color: %3;"
        "    border-color: %2;"
        "}"
    );
    
    if (variant == "primary") {
        return baseStyle.arg(colors.primary).arg(colors.secondary).arg(colors.textSecondary) +
               QString(
                   "QPushButton {"
                   "    background-color: %1;"
                   "    color: white;"
                   "}"
                   "QPushButton:hover {"
                   "    background-color: %2;"
                   "    border-color: %2;"
                   "}"
                   "QPushButton:pressed {"
                   "    background-color: %3;"
                   "}"
                     "QPushButton:disabled {"
                     "    color: white;"
                     "}"
               ).arg(colors.primary).arg(colors.primaryDark).arg(colors.primaryDark);
    }
    else if (variant == "secondary") {
        return baseStyle.arg(colors.border).arg(colors.secondary).arg(colors.textSecondary) +
               QString(
                   "QPushButton {"
                   "    background-color: %1;"
                   "    color: %2;"
                   "}"
                   "QPushButton:hover {"
                   "    background-color: %3;"
                   "    color: white;"
                   "    border-color: %3;"
                   "}"
                   "QPushButton:pressed {"
                   "    background-color: %4;"
                   "}"
                    "QPushButton:disabled {"
                    "    color: white;"
                    "}"
               ).arg(colors.surface).arg(colors.text).arg(colors.secondary).arg(colors.secondary);
    }
    else if (variant == "success") {
        return baseStyle.arg(colors.success).arg(colors.secondary).arg(colors.textSecondary) +
               QString(
                   "QPushButton {"
                   "    background-color: %1;"
                   "    color: white;"
                   "}"
                   "QPushButton:hover {"
                   "    background-color: #219a52;"
                   "    border-color: #219a52;"
                   "}"
                   "QPushButton:pressed {"
                   "    background-color: #1e8449;"
                   "}"
                    "QPushButton:disabled {"
                    "    color: white;"
                    "}"
               ).arg(colors.success);
    }
    else if (variant == "warning") {
        return baseStyle.arg(colors.warning).arg(colors.secondary).arg(colors.textSecondary) +
               QString(
                   "QPushButton {"
                   "    background-color: %1;"
                   "    color: white;"
                   "}"
                   "QPushButton:hover {"
                   "    background-color: #e67e22;"
                   "    border-color: #e67e22;"
                   "}"
                   "QPushButton:pressed {"
                   "    background-color: #d35400;"
                   "}"
                     "QPushButton:disabled {"
                     "    color: white;"
                     "}"
               ).arg(colors.warning);
    }
    else if (variant == "error") {
        return baseStyle.arg(colors.error).arg(colors.secondary).arg(colors.textSecondary) +
               QString(
                   "QPushButton {"
                   "    background-color: %1;"
                   "    color: white;"
                   "}"
                   "QPushButton:hover {"
                   "    background-color: #c0392b;"
                   "    border-color: #c0392b;"
                   "}"
                   "QPushButton:pressed {"
                   "    background-color: #a93226;"
                   "}"
                     "QPushButton:disabled {"
                     "    color: white;"
                     "}"
               ).arg(colors.error);
    }
    
    return baseStyle.arg(colors.border).arg(colors.secondary).arg(colors.textSecondary);
}

void UIStyleManager::applyLabelStyle(QLabel *label, const QString &variant)
{
    if (!label) return;
    
    label->setStyleSheet(getLabelStyleSheet(variant));
}

QString UIStyleManager::getLabelStyleSheet(const QString &variant)
{
    QString baseStyle = QString(
        "QLabel {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 5px;"
        "}"
    );
    
    if (variant == "title") {
        return baseStyle + QString(
            "QLabel {"
            "    font-size: 24px;"
            "    font-weight: bold;"
            "    color: %1;"
            "    margin-bottom: 10px;"
            "}"
        ).arg(colors.text);
    }
    else if (variant == "subtitle") {
        return baseStyle + QString(
            "QLabel {"
            "    font-size: 18px;"
            "    font-weight: 600;"
            "    color: %1;"
            "    margin-bottom: 8px;"
            "}"
        ).arg(colors.text);
    }
    else if (variant == "caption") {
        return baseStyle + QString(
            "QLabel {"
            "    font-size: 12px;"
            "    color: %1;"
            "}"
        ).arg(colors.textSecondary);
    }
    else if (variant == "success") {
        return baseStyle + QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: 500;"
            "}"
        ).arg(colors.success);
    }
    else if (variant == "warning") {
        return baseStyle + QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: 500;"
            "}"
        ).arg(colors.warning);
    }
    else if (variant == "error") {
        return baseStyle + QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: 500;"
            "}"
        ).arg(colors.error);
    }
    
    return baseStyle + QString(
        "QLabel {"
        "    color: %1;"
        "}"
    ).arg(colors.text);
}

void UIStyleManager::applyFrameStyle(QFrame *frame, bool withShadow)
{
    if (!frame) return;
    
    frame->setStyleSheet(getFrameStyleSheet());
    frame->setFrameStyle(QFrame::Box | QFrame::Plain);
    frame->setLineWidth(0);
    
    if (withShadow) {
        QGraphicsDropShadowEffect *shadow = createShadowEffect(frame);
        frame->setGraphicsEffect(shadow);
    }
}

QString UIStyleManager::getFrameStyleSheet()
{
    return QString(
        "QFrame {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "}"
    ).arg(colors.surface).arg(colors.border);
}

void UIStyleManager::applyGroupBoxStyle(QGroupBox *groupBox)
{
    if (!groupBox) return;
    
    groupBox->setStyleSheet(getGroupBoxStyleSheet());
}

QString UIStyleManager::getGroupBoxStyleSheet()
{
    return QString(
        "QGroupBox {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 8px;"
        "    margin-top: 12px;"
        "    padding-top: 15px;"
        "    background-color: %3;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 8px 0 8px;"
        "    color: %4;"
        "    background-color: %3;"
        "}"
    ).arg(colors.text).arg(colors.border).arg(colors.surface).arg(colors.primary);
}

void UIStyleManager::applyLineEditStyle(QLineEdit *lineEdit)
{
    if (!lineEdit) return;
    
    lineEdit->setStyleSheet(getLineEditStyleSheet());
}

QString UIStyleManager::getLineEditStyleSheet()
{
    return QString(
        "QLineEdit {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    color: %3;"
        "    selection-background-color: %4;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %4;"
        "    outline: none;"
        "}"
        "QLineEdit:disabled {"
        "    background-color: %5;"
        "    color: %6;"
        "}"
    ).arg(colors.surface).arg(colors.border).arg(colors.text)
     .arg(colors.primary).arg(colors.background).arg(colors.textSecondary);
}

void UIStyleManager::applyTextEditStyle(QTextEdit *textEdit)
{
    if (!textEdit) return;
    
    textEdit->setStyleSheet(getTextEditStyleSheet());
}

QString UIStyleManager::getTextEditStyleSheet()
{
    return QString(
        "QTextEdit {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 6px;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "    color: %3;"
        "    selection-background-color: %4;"
        "}"
        "QTextEdit:focus {"
        "    border-color: %4;"
        "    outline: none;"
        "}"
    ).arg(colors.surface).arg(colors.border).arg(colors.text).arg(colors.primary);
}

void UIStyleManager::applyScrollAreaStyle(QScrollArea *scrollArea)
{
    if (!scrollArea) return;
    
    scrollArea->setStyleSheet(getScrollAreaStyleSheet());
}

QString UIStyleManager::getScrollAreaStyleSheet()
{
    return QString(
        "QScrollArea {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 6px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background-color: %1;"
        "}"
    ).arg(colors.surface).arg(colors.border);
}

QGraphicsDropShadowEffect* UIStyleManager::createShadowEffect(QWidget *parent)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(parent);
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 30));
    return shadow;
}

void UIStyleManager::applyContainerSpacing(QWidget *container)
{
    if (!container) return;
    
    // 为容器应用统一的间距设置
    QLayout *layout = container->layout();
    if (!layout) return;
    
    if (auto *vLayout = qobject_cast<QVBoxLayout*>(layout)) {
        vLayout->setContentsMargins(20, 20, 20, 20);
        vLayout->setSpacing(15);
    }
    else if (auto *hLayout = qobject_cast<QHBoxLayout*>(layout)) {
        hLayout->setContentsMargins(20, 20, 20, 20);
        hLayout->setSpacing(15);
    }
    else if (auto *gLayout = qobject_cast<QGridLayout*>(layout)) {
        gLayout->setContentsMargins(20, 20, 20, 20);
        gLayout->setHorizontalSpacing(15);
        gLayout->setVerticalSpacing(15);
    }
}
