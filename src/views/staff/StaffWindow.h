#ifndef STAFFWINDOW_H
#define STAFFWINDOW_H

#include "../common/BaseWindow.h"
#include <QToolBar>
#include <QAction>

class RecordWidget;
class StatsWidget;
class ManualChatWidget;

class StaffWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit StaffWindow(QWidget *parent = nullptr);
    ~StaffWindow();

protected:
    void setupMenu() override;
    void setupFunctionWidgets() override;

protected slots:
    void onMenuItemClicked(MenuAction action) override;

private slots:
    void onRefreshClicked();
    void onExportClicked();
    void onSettingsClicked();

private:
    void createWidgets();
    void setupToolBar();

private:
    // 功能组件
    RecordWidget* m_recordWidget;
    StatsWidget* m_statsWidget;
    ManualChatWidget* m_manualChatWidget;
    
    // 工具栏
    QToolBar* m_toolBar;
    QAction* m_actRefresh;
    QAction* m_actExport;
    QAction* m_actSettings;
};

#endif // STAFFWINDOW_H 