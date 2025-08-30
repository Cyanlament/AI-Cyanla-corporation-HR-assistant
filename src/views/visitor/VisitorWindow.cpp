#include "VisitorWindow.h"
#include "ChatWidget.h"
#include "FAQWidget.h"
#include "MapWidget.h"

VisitorWindow::VisitorWindow(QWidget *parent)
    : BaseWindow(UserRole::visitor, parent)
    , m_chatWidget(nullptr)
    , m_faqWidget(nullptr)
    , m_mapWidget(nullptr)
{
    setWindowTitle("医院智慧客服系统 - 患者端");
    createWidgets();
}

VisitorWindow::~VisitorWindow()
{
    // Qt会自动清理子对象
}

void VisitorWindow::setupMenu()
{
    // 菜单已在BaseWindow中通过SideMenuWidget设置
}

void VisitorWindow::setupFunctionWidgets()
{
    // 添加功能页面到堆叠widget
    addFunctionWidget(m_chatWidget, "智能分诊");
    addFunctionWidget(m_faqWidget, "常见问题");
    addFunctionWidget(m_mapWidget, "院内导航");
    
    // 默认显示聊天页面
    setCurrentWidget("智能分诊");
}

void VisitorWindow::createWidgets()
{
    m_chatWidget = new ChatWidget(this);
    m_faqWidget = new FAQWidget(this);
    m_mapWidget = new MapWidget(this);
}

void VisitorWindow::onMenuItemClicked(MenuAction action)
{
    switch (action) {
    case MenuAction::visitorChat:
        setCurrentWidget("智能分诊");
        break;
    case MenuAction::visitorAppointment:
        setCurrentWidget("常见问题");
        break;
    case MenuAction::visitorMap:
        setCurrentWidget("院内导航");
        break;
    default:
        break;
    }
} 
