#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <QGroupBox>
#include <QSplitter>
#include <QComboBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QMap>
#include <QRect>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

// 部门信息结构体
struct DepartmentInfo {
    QString name;           // 部门名称
    QString description;    // 部门描述
    QString location;       // 位置描述
    QString hours;          // 开放时间
    QString phone;          // 联系电话
    QRect mapRect;          // 在地图上的区域
    QColor highlightColor;  // 高亮颜色
    QString floor;          // 楼层
    QString building;       // 建筑物
};

// 自定义地图绘制Widget
class CompanyMapWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double pulseOpacity READ pulseOpacity WRITE setPulseOpacity)

public:
    explicit CompanyMapWidget(QWidget* parent = nullptr);
    
    void setDepartments(const QMap<QString, DepartmentInfo>& departments);
    void highlightDepartment(const QString& departmentName);
    void clearHighlight();
    void showRoute(const QString& from, const QString& to);
    void setZoomLevel(double zoom);
    
    QSize sizeHint() const override;
    
    // 动画属性
    double pulseOpacity() const { return m_pulseOpacity; }
    void setPulseOpacity(double opacity) { m_pulseOpacity = opacity; update(); }
    
signals:
    void departmentClicked(const QString& departmentName);
    void routeRequested(const QString& from, const QString& to);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawBackground(QPainter& painter);
    void drawBuildings(QPainter& painter);
    void drawDepartments(QPainter& painter);
    void drawRoute(QPainter& painter);
    void drawLabels(QPainter& painter);
    
    QString getDepartmentAt(const QPoint& pos) const;
    QRect getScaledRect(const QRect& rect) const;
    
private:
    QMap<QString, DepartmentInfo> m_departments;
    QString m_highlightedDepartment;
    QString m_hoveredDepartment;
    QString m_routeFrom;
    QString m_routeTo;
    double m_zoomLevel;
    QPoint m_lastMousePos;
    
    // 动画相关
    QPropertyAnimation* m_pulseAnimation;
    QGraphicsOpacityEffect* m_pulseEffect;
    double m_pulseOpacity;
    
    // 路径动画
    QTimer* m_routeTimer;
    int m_routeAnimationStep;
    QList<QPoint> m_routePoints;
};

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

private slots:
    // 搜索和选择
    void onSearchTextChanged();
    void onDepartmentSelected();
    void onFloorChanged();
    void onClearSelection();
    
    // 地图交互
    void onMapDepartmentClicked(const QString& departmentName);
    void onMapRouteRequested(const QString& from, const QString& to);
    void onZoomIn();
    void onZoomOut();
    void onResetView();
    
    // 路径规划
    void onGetRoute();
    void onShowEmergencyRoute();
    
    // 动画和效果
    void updateRouteAnimation();
    void showLocationDetails(const QString& departmentName);

private:
    // UI初始化
    void setupUI();
    void setupLeftPanel();
    void setupRightPanel();
    void setupToolbar();
    
    // 数据初始化
    void initializeDepartments();
    void loadFloorData();
    
    // 搜索和过滤
    void updateDepartmentList();
    void filterDepartments(const QString& keyword);
    
    // 地图操作
    void highlightDepartment(const QString& departmentName);
    void showRoute(const QString& from, const QString& to);
    void updateMapDisplay();
    
    // 工具方法
    QString formatDepartmentInfo(const DepartmentInfo& info) const;
    QStringList searchDepartments(const QString& keyword) const;
    void applyModernStyle();

private:
    // 主布局
    QHBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    
    // 左侧面板
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    
    // 搜索区域
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_floorCombo;
    QPushButton* m_btnClearSearch;
    
    // 部门列表
    QGroupBox* m_departmentGroup;
    QListWidget* m_departmentList;
    QLabel* m_listInfo;
    
    // 快捷功能
    QGroupBox* m_quickGroup;
    QPushButton* m_btnEmergency;
    QPushButton* m_btnParking;
    QPushButton* m_btnPharmacy;
    QPushButton* m_btnCashier;
    
    // 右侧面板
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    
    // 工具栏
    QWidget* m_toolbar;
    QHBoxLayout* m_toolbarLayout;
    QLabel* m_titleLabel;
    QPushButton* m_btnZoomIn;
    QPushButton* m_btnZoomOut;
    QPushButton* m_btnResetView;
    QPushButton* m_btnGetRoute;
    
    // 地图区域
    QScrollArea* m_mapScrollArea;
    CompanyMapWidget* m_mapWidget;
    
    // 信息面板
    QGroupBox* m_infoGroup;
    QLabel* m_infoTitle;
    QLabel* m_infoContent;
    QPushButton* m_btnShowRoute;
    
    // 数据存储
    QMap<QString, DepartmentInfo> m_departments;
    QMap<QString, QStringList> m_floorDepartments;
    QString m_selectedDepartment;
    QString m_currentFloor;
    
    // 状态管理
    double m_currentZoom;
    bool m_isRouteMode;
    QString m_routeStart;
    QString m_routeEnd;
    
    // 动画和效果
    QTimer* m_searchTimer;
    QPropertyAnimation* m_highlightAnimation;
};

#endif // MAPWIDGET_H 