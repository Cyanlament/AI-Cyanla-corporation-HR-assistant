#ifndef VISITORWINDOW_H
#define VISITORWINDOW_H

#include "../common/BaseWindow.h"

class ChatWidget;
class FAQWidget;
class MapWidget;

class VisitorWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit VisitorWindow(QWidget *parent = nullptr);
    ~VisitorWindow();

protected:
    void setupMenu() override;
    void setupFunctionWidgets() override;

protected slots:
    void onMenuItemClicked(MenuAction action) override;

private:
    void createWidgets();

private:
    ChatWidget* m_chatWidget;
    FAQWidget* m_faqWidget;
    MapWidget* m_mapWidget;
};

#endif // VISITORWINDOW_H
