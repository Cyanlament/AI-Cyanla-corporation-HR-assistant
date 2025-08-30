#ifndef PATIENTWINDOW_H
#define PATIENTWINDOW_H

#include "../common/BaseWindow.h"

class ChatWidget;
class FAQWidget;
class MapWidget;

class PatientWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit PatientWindow(QWidget *parent = nullptr);
    ~PatientWindow();

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

#endif // PATIENTWINDOW_H 