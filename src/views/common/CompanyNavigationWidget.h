#ifndef COMPANYNAVIGATIONWIDGET_H
#define COMPANYNAVIGATIONWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QPointF>
#include <QMap>
#include <QMouseEvent>
#include <QDebug>

struct Department {
    QString name;           // 部门名称
    QPointF position;       // 在地图上的位置
    QRectF area;           // 点击区域
    QList<QPointF> pathPoints; // 从入口到该部门的路径点
    QString description;    // 导航说明
};

class CompanyGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CompanyGraphicsView(QWidget *parent = nullptr);

signals:
    void departmentClicked(const QString &departmentName);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QMap<QString, Department> m_departments;
    friend class CompanyNavigationWidget;
};

class CompanyNavigationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CompanyNavigationWidget(QWidget *parent = nullptr);
    ~CompanyNavigationWidget();

    // 设置青蓝公司地图图片
    void setCompanyMap(const QString &imagePath);
    
    // 添加部门信息
    void addDepartment(const QString &name, const QPointF &position, 
                      const QRectF &area, const QList<QPointF> &pathPoints,
                      const QString &description);

public slots:
    void onDepartmentClicked(const QString &departmentName);
    void clearNavigation();

private:
    void setupUI();
    void initializeDepartments();
    void drawNavigationPath(const QList<QPointF> &pathPoints);
    void showNavigationText(const QString &text);

private:
    CompanyGraphicsView *m_graphicsView;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_mapItem;
    QGraphicsPathItem *m_pathItem;
    
    QLabel *m_navigationLabel;
    QLabel *m_titleLabel;
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_contentLayout;
    
    QMap<QString, Department> m_departments;
    QString m_currentDepartment;
};

#endif // COMPANYNAVIGATIONWIDGET_H
