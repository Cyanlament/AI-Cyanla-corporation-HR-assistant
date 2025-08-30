#include "HospitalNavigationWidget.h"
#include "UIStyleManager.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QPushButton>

// HospitalGraphicsView 实现
HospitalGraphicsView::HospitalGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing);
}

void HospitalGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    
    // 检查点击的位置是否在某个科室区域内
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        if (it.value().area.contains(scenePos)) {
            emit departmentClicked(it.key());
            return;
        }
    }
    
    QGraphicsView::mousePressEvent(event);
}

// HospitalNavigationWidget 实现
HospitalNavigationWidget::HospitalNavigationWidget(QWidget *parent)
    : QWidget(parent)
    , m_graphicsView(nullptr)
    , m_scene(nullptr)
    , m_mapItem(nullptr)
    , m_pathItem(nullptr)
    , m_navigationLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_mainLayout(nullptr)
    , m_contentLayout(nullptr)
{
    setupUI();
    initializeDepartments();
}

HospitalNavigationWidget::~HospitalNavigationWidget()
{
    if (m_scene) {
        delete m_scene;
    }
}

void HospitalNavigationWidget::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    UIStyleManager::applyContainerSpacing(this);

    // 标题标签
    m_titleLabel = new QLabel("🏥 医院导航系统", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    UIStyleManager::applyLabelStyle(m_titleLabel, "title");
    m_titleLabel->setStyleSheet(m_titleLabel->styleSheet() + 
        QString("QLabel {"
                "background-color: %1;"
                "border-radius: 12px;"
                "padding: 20px;"
                "margin-bottom: 15px;"
                "}")
        .arg(UIStyleManager::colors.surface));
    
    // 添加阴影效果
    m_titleLabel->setGraphicsEffect(UIStyleManager::createShadowEffect(m_titleLabel));
    m_mainLayout->addWidget(m_titleLabel);

    // 内容水平布局
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(25);

    // 创建图形视图和场景
    m_scene = new QGraphicsScene(this);
    m_graphicsView = new HospitalGraphicsView(this);
    m_graphicsView->setScene(m_scene);
    m_graphicsView->setMinimumSize(700, 500);
    
    // 设置图形视图样式 - 使用现代化样式
    m_graphicsView->setStyleSheet(
        QString("QGraphicsView {"
               "border: 2px solid %1;"
               "border-radius: 12px;"
               "background-color: %2;"
               "padding: 5px;"
               "}")
        .arg(UIStyleManager::colors.border)
        .arg(UIStyleManager::colors.surface)
    );
    
    // 添加阴影效果
    m_graphicsView->setGraphicsEffect(UIStyleManager::createShadowEffect(m_graphicsView));

    // 创建右侧信息面板
    QFrame *infoPanel = new QFrame(this);
    UIStyleManager::applyFrameStyle(infoPanel, true);
    infoPanel->setFixedWidth(300);
    
    QVBoxLayout *infoPanelLayout = new QVBoxLayout(infoPanel);
    infoPanelLayout->setContentsMargins(15, 15, 15, 15);
    infoPanelLayout->setSpacing(15);
    
    // 面板标题
    QLabel *panelTitle = new QLabel("🗺️ 导航信息", infoPanel);
    UIStyleManager::applyLabelStyle(panelTitle, "subtitle");
    infoPanelLayout->addWidget(panelTitle);
    
    // 导航说明标签
    m_navigationLabel = new QLabel("点击地图上的科室名称查看导航路线", infoPanel);
    m_navigationLabel->setWordWrap(true);
    m_navigationLabel->setAlignment(Qt::AlignTop);
    UIStyleManager::applyLabelStyle(m_navigationLabel, "normal");
    m_navigationLabel->setStyleSheet(m_navigationLabel->styleSheet() + 
        QString("QLabel {"
               "background-color: %1;"
               "border: 1px solid %2;"
               "border-radius: 8px;"
               "padding: 15px;"
               "line-height: 1.6;"
               "}")
        .arg(UIStyleManager::colors.background)
        .arg(UIStyleManager::colors.border));
    
    infoPanelLayout->addWidget(m_navigationLabel);
    
    // 添加清除按钮
    QPushButton *clearButton = new QPushButton("🔄 清除路径", infoPanel);
    UIStyleManager::applyButtonStyle(clearButton, "secondary");
    connect(clearButton, &QPushButton::clicked, this, &HospitalNavigationWidget::clearNavigation);
    infoPanelLayout->addWidget(clearButton);
    
    // 添加弹性空间
    infoPanelLayout->addStretch();
    
    // 添加使用说明
    QLabel *helpLabel = new QLabel(
        "💡 <b>使用说明：</b><br>"
        "• 点击地图中的科室区域<br>"
        "• 查看红色导航路径<br>"
        "• 阅读详细导航说明<br>"
        "• 点击清除路径重置", infoPanel);
    UIStyleManager::applyLabelStyle(helpLabel, "caption");
    helpLabel->setWordWrap(true);
    infoPanelLayout->addWidget(helpLabel);

    m_contentLayout->addWidget(m_graphicsView, 2);
    m_contentLayout->addWidget(infoPanel, 1);

    m_mainLayout->addLayout(m_contentLayout);

    // 连接信号
    connect(m_graphicsView, &HospitalGraphicsView::departmentClicked,
            this, &HospitalNavigationWidget::onDepartmentClicked);
}

void HospitalNavigationWidget::initializeDepartments()
{
    // 创建虚拟医院地图
    QPixmap hospitalMap(800, 600);
    hospitalMap.fill(Qt::white);
    
    QPainter painter(&hospitalMap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制医院基本结构
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(QColor(240, 240, 240)));
    
    // 主建筑轮廓
    QRect mainBuilding(50, 50, 700, 500);
    painter.drawRect(mainBuilding);
    
    // 入口
    painter.setBrush(QBrush(QColor(76, 175, 80))); // 绿色入口
    QRect entrance(350, 550, 100, 50);
    painter.drawRect(entrance);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawText(entrance, Qt::AlignCenter, "主入口");
    
    // 绘制科室区域
    painter.setPen(QPen(Qt::black, 1));
    
    // 内科
    painter.setBrush(QBrush(QColor(255, 193, 7))); // 黄色
    QRect internal(100, 100, 150, 100);
    painter.drawRect(internal);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawText(internal, Qt::AlignCenter, "内科");
    
    // 外科
    painter.setBrush(QBrush(QColor(33, 150, 243))); // 蓝色
    QRect surgery(300, 100, 150, 100);
    painter.drawRect(surgery);
    painter.drawText(surgery, Qt::AlignCenter, "外科");
    
    // 儿科
    painter.setBrush(QBrush(QColor(255, 87, 34))); // 橙色
    QRect pediatric(500, 100, 150, 100);
    painter.drawRect(pediatric);
    painter.drawText(pediatric, Qt::AlignCenter, "儿科");
    
    // 药房
    painter.setBrush(QBrush(QColor(139, 195, 74))); // 浅绿色
    QRect pharmacy(100, 300, 150, 100);
    painter.drawRect(pharmacy);
    painter.drawText(pharmacy, Qt::AlignCenter, "药房");
    
    // 挂号处
    painter.setBrush(QBrush(QColor(156, 39, 176))); // 紫色
    QRect registration(300, 300, 150, 100);
    painter.drawRect(registration);
    painter.drawText(registration, Qt::AlignCenter, "挂号处");
    
    // 急诊科
    painter.setBrush(QBrush(QColor(244, 67, 54))); // 红色
    QRect emergency(500, 300, 150, 100);
    painter.drawRect(emergency);
    painter.drawText(emergency, Qt::AlignCenter, "急诊科");
    
    painter.end();
    
    // 设置地图到场景
    m_mapItem = m_scene->addPixmap(hospitalMap);
    m_scene->setSceneRect(hospitalMap.rect());
    
    // 添加科室信息到内部存储
    QPointF entrancePoint(400, 575); // 入口点
    
    // 内科路径
    addDepartment("内科", QPointF(175, 150), 
                 QRectF(100, 100, 150, 100),
                 {entrancePoint, QPointF(400, 400), QPointF(200, 400), QPointF(175, 150)},
                 "📍 内科导航:\n1. 从主入口进入\n2. 直行至大厅中央\n3. 左转走廊\n4. 到达内科诊室\n\n⏱️ 预计步行时间: 3分钟");
    
    // 外科路径
    addDepartment("外科", QPointF(375, 150),
                 QRectF(300, 100, 150, 100),
                 {entrancePoint, QPointF(400, 400), QPointF(375, 150)},
                 "📍 外科导航:\n1. 从主入口进入\n2. 直行至大厅中央\n3. 继续直行\n4. 到达外科诊室\n\n⏱️ 预计步行时间: 2分钟");
    
    // 儿科路径
    addDepartment("儿科", QPointF(575, 150),
                 QRectF(500, 100, 150, 100),
                 {entrancePoint, QPointF(400, 400), QPointF(600, 400), QPointF(575, 150)},
                 "📍 儿科导航:\n1. 从主入口进入\n2. 直行至大厅中央\n3. 右转走廊\n4. 到达儿科诊室\n\n⏱️ 预计步行时间: 3分钟");
    
    // 药房路径
    addDepartment("药房", QPointF(175, 350),
                 QRectF(100, 300, 150, 100),
                 {entrancePoint, QPointF(400, 450), QPointF(175, 350)},
                 "📍 药房导航:\n1. 从主入口进入\n2. 左转至服务区\n3. 到达药房窗口\n\n⏱️ 预计步行时间: 2分钟\n💊 可在此取药和咨询");
    
    // 挂号处路径
    addDepartment("挂号处", QPointF(375, 350),
                 QRectF(300, 300, 150, 100),
                 {entrancePoint, QPointF(375, 350)},
                 "📍 挂号处导航:\n1. 从主入口进入\n2. 直接前方就是挂号处\n\n⏱️ 预计步行时间: 1分钟\n🎫 请先在此挂号");
    
    // 急诊科路径
    addDepartment("急诊科", QPointF(575, 350),
                 QRectF(500, 300, 150, 100),
                 {entrancePoint, QPointF(400, 450), QPointF(575, 350)},
                 "📍 急诊科导航:\n1. 从主入口进入\n2. 右转至急诊区\n3. 到达急诊科\n\n⏱️ 预计步行时间: 2分钟\n🚨 24小时开放");
    
    // 将科室信息同步到图形视图
    m_graphicsView->m_departments = m_departments;
}

void HospitalNavigationWidget::setHospitalMap(const QString &imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        if (m_mapItem) {
            m_scene->removeItem(m_mapItem);
            delete m_mapItem;
        }
        m_mapItem = m_scene->addPixmap(pixmap);
        m_scene->setSceneRect(pixmap.rect());
    }
}

void HospitalNavigationWidget::addDepartment(const QString &name, const QPointF &position,
                                           const QRectF &area, const QList<QPointF> &pathPoints,
                                           const QString &description)
{
    Department dept;
    dept.name = name;
    dept.position = position;
    dept.area = area;
    dept.pathPoints = pathPoints;
    dept.description = description;
    
    m_departments[name] = dept;
    
    // 同步到图形视图
    if (m_graphicsView) {
        m_graphicsView->m_departments = m_departments;
    }
}

void HospitalNavigationWidget::onDepartmentClicked(const QString &departmentName)
{
    qDebug() << "科室被点击:" << departmentName;
    
    if (!m_departments.contains(departmentName)) {
        qWarning() << "未找到科室:" << departmentName;
        return;
    }
    
    m_currentDepartment = departmentName;
    const Department &dept = m_departments[departmentName];
    
    // 绘制导航路径
    drawNavigationPath(dept.pathPoints);
    
    // 显示导航文字说明
    showNavigationText(dept.description);
}

void HospitalNavigationWidget::clearNavigation()
{
    // 清除路径
    if (m_pathItem) {
        m_scene->removeItem(m_pathItem);
        delete m_pathItem;
        m_pathItem = nullptr;
    }
    
    // 重置导航说明
    m_navigationLabel->setText("点击地图上的科室名称查看导航路线");
    m_currentDepartment.clear();
}

void HospitalNavigationWidget::drawNavigationPath(const QList<QPointF> &pathPoints)
{
    if (pathPoints.isEmpty()) return;
    
    // 清除之前的路径
    if (m_pathItem) {
        m_scene->removeItem(m_pathItem);
        delete m_pathItem;
        m_pathItem = nullptr;
    }
    
    // 创建路径
    QPainterPath path;
    path.moveTo(pathPoints.first());
    
    for (int i = 1; i < pathPoints.size(); ++i) {
        path.lineTo(pathPoints[i]);
    }
    
    // 添加路径到场景
    m_pathItem = m_scene->addPath(path, QPen(Qt::red, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    
    // 在路径上添加箭头
    for (int i = 0; i < pathPoints.size() - 1; ++i) {
        QPointF start = pathPoints[i];
        QPointF end = pathPoints[i + 1];
        QPointF direction = end - start;
        qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (length > 0) {
            direction /= length;
            QPointF arrowPos = start + direction * (length * 0.5);
            
            // 简单箭头
            QPainterPath arrow;
            arrow.moveTo(arrowPos);
            arrow.lineTo(arrowPos + QPointF(-direction.y() * 8 - direction.x() * 8, 
                                           direction.x() * 8 - direction.y() * 8));
            arrow.lineTo(arrowPos + QPointF(direction.y() * 8 - direction.x() * 8, 
                                           -direction.x() * 8 - direction.y() * 8));
            arrow.closeSubpath();
            
            m_scene->addPath(arrow, QPen(Qt::red, 2), QBrush(Qt::red));
        }
    }
    
    // 在终点添加目标标记
    QPointF target = pathPoints.last();
    m_scene->addEllipse(target.x() - 8, target.y() - 8, 16, 16, 
                       QPen(Qt::red, 3), QBrush(Qt::red));
}

void HospitalNavigationWidget::showNavigationText(const QString &text)
{
    QString styledText = QString(
        "<div style='color: %1; line-height: 1.8; font-size: 14px;'>"
        "<h3 style='color: %2; margin-bottom: 15px; font-size: 16px;'>🗺️ %3</h3>"
        "<div style='background-color: %4; padding: 10px; border-radius: 6px; margin: 10px 0;'>"
        "%5"
        "</div>"
        "</div>"
    ).arg(UIStyleManager::colors.text)
     .arg(UIStyleManager::colors.primary)
     .arg(m_currentDepartment)
     .arg(UIStyleManager::colors.surface)
     .arg(text.split('\n').join("<br>"));
    
    m_navigationLabel->setText(styledText);
} 