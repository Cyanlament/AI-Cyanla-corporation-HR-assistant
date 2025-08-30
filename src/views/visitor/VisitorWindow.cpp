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
    setWindowTitle("青蓝公司HR制度智能问答系统 - 访客端");
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
    addFunctionWidget(m_chatWidget, "智能HR");
    addFunctionWidget(m_faqWidget, "常见问题");
    addFunctionWidget(m_mapWidget, "公司内导航");
    
    // 默认显示聊天页面
    setCurrentWidget("智能HR");
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
        setCurrentWidget("智能HR");
        break;
    case MenuAction::visitorAppointment:
        setCurrentWidget("常见问题");
        break;
    case MenuAction::visitorMap:
        setCurrentWidget("公司内导航");
        break;
    default:
        break;
    }
} 
