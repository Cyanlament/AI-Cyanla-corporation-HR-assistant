#include "PatientWindow.h"
#include "ChatWidget.h"
#include "FAQWidget.h"
#include "MapWidget.h"

PatientWindow::PatientWindow(QWidget *parent)
    : BaseWindow(UserRole::Patient, parent)
    , m_chatWidget(nullptr)
    , m_faqWidget(nullptr)
    , m_mapWidget(nullptr)
{
    setWindowTitle("医院智慧客服系统 - 患者端");
    createWidgets();
}

PatientWindow::~PatientWindow()
{
    // Qt会自动清理子对象
}

void PatientWindow::setupMenu()
{
    // 菜单已在BaseWindow中通过SideMenuWidget设置
}

void PatientWindow::setupFunctionWidgets()
{
    // 添加功能页面到堆叠widget
    addFunctionWidget(m_chatWidget, "智能分诊");
    addFunctionWidget(m_faqWidget, "常见问题");
    addFunctionWidget(m_mapWidget, "院内导航");
    
    // 默认显示聊天页面
    setCurrentWidget("智能分诊");
}

void PatientWindow::createWidgets()
{
    m_chatWidget = new ChatWidget(this);
    m_faqWidget = new FAQWidget(this);
    m_mapWidget = new MapWidget(this);
}

void PatientWindow::onMenuItemClicked(MenuAction action)
{
    switch (action) {
    case MenuAction::PatientChat:
        setCurrentWidget("智能分诊");
        break;
    case MenuAction::PatientAppointment:
        setCurrentWidget("常见问题");
        break;
    case MenuAction::PatientMap:
        setCurrentWidget("院内导航");
        break;
    default:
        break;
    }
} 