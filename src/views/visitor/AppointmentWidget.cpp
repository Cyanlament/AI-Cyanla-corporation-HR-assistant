#include "AppointmentWidget.h"
#include <QListWidgetItem>
#include <QMessageBox>
#include <QDate>
#include <QRandomGenerator>

AppointmentWidget::AppointmentWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_departmentGroup(nullptr)
    , m_departmentCombo(nullptr)
    , m_sephirahGroup(nullptr)
    , m_sephirahList(nullptr)
    , m_dateGroup(nullptr)
    , m_calendar(nullptr)
    , m_timeGroup(nullptr)
    , m_timeList(nullptr)
    , m_infoGroup(nullptr)
    , m_appointmentInfo(nullptr)
    , m_confirmButton(nullptr)
{
    setupUI();
    loadDepartments();
}

void AppointmentWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // 创建网格布局
    QGridLayout* gridLayout = new QGridLayout;
    
    // 部门选择
    m_departmentGroup = new QGroupBox("选择部门");
    QVBoxLayout* deptLayout = new QVBoxLayout(m_departmentGroup);
    m_departmentCombo = new QComboBox;
    connect(m_departmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppointmentWidget::onDepartmentChanged);
    deptLayout->addWidget(m_departmentCombo);
    
    // 部长选择
    m_sephirahGroup = new QGroupBox("选择面试官");
    QVBoxLayout* sephirahLayout = new QVBoxLayout(m_sephirahGroup);
    m_sephirahList = new QListWidget;
    m_sephirahList->setMaximumHeight(200);
    connect(m_sephirahList, &QListWidget::itemClicked,
            this, &AppointmentWidget::onSephirahSelected);
    sephirahLayout->addWidget(m_sephirahList);
    
    // 日期选择
    m_dateGroup = new QGroupBox("选择日期");
    QVBoxLayout* dateLayout = new QVBoxLayout(m_dateGroup);
    m_calendar = new QCalendarWidget;
    m_calendar->setMinimumDate(QDate::currentDate());
    m_calendar->setMaximumDate(QDate::currentDate().addDays(30));
    connect(m_calendar, &QCalendarWidget::clicked,
            this, &AppointmentWidget::onDateSelected);
    dateLayout->addWidget(m_calendar);
    
    // 时间段选择
    m_timeGroup = new QGroupBox("选择时间");
    QVBoxLayout* timeLayout = new QVBoxLayout(m_timeGroup);
    m_timeList = new QListWidget;
    m_timeList->setMaximumHeight(200);
    connect(m_timeList, &QListWidget::itemClicked,
            this, &AppointmentWidget::onTimeSlotSelected);
    timeLayout->addWidget(m_timeList);
    
    // 添加到网格
    gridLayout->addWidget(m_departmentGroup, 0, 0);
    gridLayout->addWidget(m_sephirahGroup, 0, 1);
    gridLayout->addWidget(m_dateGroup, 1, 0);
    gridLayout->addWidget(m_timeGroup, 1, 1);
    
    // 面试预约信息和确认按钮
    m_infoGroup = new QGroupBox("面试预约信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoGroup);
    
    m_appointmentInfo = new QLabel("请完成上述选择...");
    m_appointmentInfo->setWordWrap(true);
    m_appointmentInfo->setStyleSheet("QLabel { color: #6D6D70; padding: 10px; }");
    
    m_confirmButton = new QPushButton("确认预约");
    m_confirmButton->setEnabled(false);
    m_confirmButton->setStyleSheet(R"(
        QPushButton {
            background-color: #34C759;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 15px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #30B24A;
        }
        QPushButton:disabled {
            background-color: #C7C7CC;
        }
    )");
    connect(m_confirmButton, &QPushButton::clicked,
            this, &AppointmentWidget::makeAppointment);
    
    infoLayout->addWidget(m_appointmentInfo);
    infoLayout->addWidget(m_confirmButton);
    
    // 添加到主布局
    m_mainLayout->addLayout(gridLayout);
    m_mainLayout->addWidget(m_infoGroup);
    
    // 应用样式
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
        QComboBox, QListWidget, QCalendarWidget {
            border: 1px solid #CED4DA;
            border-radius: 6px;
            background-color: white;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #F1F3F4;
        }
        QListWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
    )");
}

void AppointmentWidget::loadDepartments()
{
    QStringList departments = {
        "控制部",
        "福利部", 
        "记录部",
        "培训部",
        "研发部",
        "情报部",
        "安保部",
        "中央本部一区",
        "中央本部二区",
        "惩戒部",
        "构筑部"
    };
    
    m_departmentCombo->addItems(departments);
}

void AppointmentWidget::loadSephirahs()
{
    m_sephirahList->clear();
    
    // 部长数据
    QStringList sephirahs;
    if (m_selectedDepartment == "控制部") {
        sephirahs << "Malkuth - 部长" << "妮妮 - 队长" << "耗 - 副队长";
    } else if (m_selectedDepartment == "情报部") {
        sephirahs << "Yesod - 部长" << "弗兰力 - 队长" << "上级 - 副队长";
    } else if (m_selectedDepartment == "培训部") {
        sephirahs << "Hod - 部长" << "白发 - 队长" << "啪啪 - 副队长";
    } else if (m_selectedDepartment == "安保部") {
        sephirahs << "Netzach - 部长" << "骨头哥 - 队长" << "阿良 - 副队长";
    } else if (m_selectedDepartment == "中央本部一区") {
        sephirahs << "TipherethA - 部长" << "张叔叔 - 队长" << "哈哈 - 副队长";
    } else if (m_selectedDepartment == "中央本部二区") {
        sephirahs << "TipherethB - 部长" << "张嫂 - 队长" << "崩坏 - 副队长";
    } else if (m_selectedDepartment == "福利部") {
        sephirahs << "Chesed - 部长" << "奥托 - 队长" << "粉色妖精小姐🎶 - 副队长";
    } else if (m_selectedDepartment == "惩戒部") {
        sephirahs << "Geburah - 部长" << "堂吉诃德 - 队长" << "涛哥 - 副队长";
    } else if (m_selectedDepartment == "记录部") {
        sephirahs << "Hokma - 部长" << "凑数人 - 队长" << "秃秃大侠 - 副队长";
    } else if (m_selectedDepartment == "研发部") {
        sephirahs << "Binah - 部长" << "凯特 - 队长" << "夜将明 - 副队长";
    } else if (m_selectedDepartment == "构筑部") {
        sephirahs << "Keter - 部长" << "Ayin - 队长" << "苍蓝礼悼 - 副队长";
    } else {
        sephirahs << "未知部门";
    }
    
    for (const QString& sephirah : sephirahs) {
        QListWidgetItem* item = new QListWidgetItem(sephirah);
        m_sephirahList->addItem(item);
    }
}

void AppointmentWidget::loadTimeSlots()
{
    m_timeList->clear();
    
    QStringList morningSlots = {
        "08:00-08:30", "08:30-09:00", "09:00-09:30", "09:30-10:00",
        "10:00-10:30", "10:30-11:00", "11:00-11:30", "11:30-12:00"
    };
    
    QStringList afternoonSlots = {
        "14:00-14:30", "14:30-15:00", "15:00-15:30", "15:30-16:00",
        "16:00-16:30", "16:30-17:00", "17:00-17:30", "17:30-18:00"
    };
    
    for (const QString& slot : morningSlots + afternoonSlots) {
        QListWidgetItem* item = new QListWidgetItem(slot);
        // 随机设置一些时段为已预约
        if (QRandomGenerator::global()->bounded(4) == 0) {
            item->setText(slot + " (已预约)");
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setBackground(QBrush(QColor(220, 220, 220)));
        }
        m_timeList->addItem(item);
    }
}

void AppointmentWidget::onDepartmentChanged()
{
    m_selectedDepartment = m_departmentCombo->currentText();
    loadSephirahs();
    
    // 清空其他选择
    m_selectedSephirah.clear();
    m_selectedTime.clear();
    m_timeList->clear();
    updateAppointmentInfo();
}

void AppointmentWidget::onSephirahSelected()
{
    QListWidgetItem* item = m_sephirahList->currentItem();
    if (item) {
        m_selectedSephirah = item->text();
        updateAppointmentInfo();
    }
}

void AppointmentWidget::onDateSelected(const QDate& date)
{
    m_selectedDate = date;
    loadTimeSlots();
    updateAppointmentInfo();
}

void AppointmentWidget::onTimeSlotSelected()
{
    QListWidgetItem* item = m_timeList->currentItem();
    if (item && item->flags() & Qt::ItemIsSelectable) {
        m_selectedTime = item->text();
        updateAppointmentInfo();
    }
}

void AppointmentWidget::updateAppointmentInfo()
{
    QString info;
    
    if (!m_selectedDepartment.isEmpty()) {
        info += QString("部门: %1\n").arg(m_selectedDepartment);
    }
    if (!m_selectedSephirah.isEmpty()) {
        info += QString("部长: %1\n").arg(m_selectedSephirah);
    }
    if (m_selectedDate.isValid()) {
        info += QString("日期: %1\n").arg(m_selectedDate.toString("yyyy年MM月dd日"));
    }
    if (!m_selectedTime.isEmpty()) {
        info += QString("时间: %1\n").arg(m_selectedTime);
    }
    
    if (info.isEmpty()) {
        info = "请完成上述选择...";
        m_confirmButton->setEnabled(false);
    } else {
        bool allSelected = !m_selectedDepartment.isEmpty() && 
                          !m_selectedSephirah.isEmpty() && 
                          m_selectedDate.isValid() && 
                          !m_selectedTime.isEmpty();
        m_confirmButton->setEnabled(allSelected);
    }
    
    m_appointmentInfo->setText(info);
}

void AppointmentWidget::makeAppointment()
{
    QString message = QString("预约成功！\n\n部门: %1\n部长: %2\n日期: %3\n时间: %4\n\n请按时参加面试，如需取消请提前联系公司。")
                     .arg(m_selectedDepartment)
                     .arg(m_selectedSephirah)
                     .arg(m_selectedDate.toString("yyyy年MM月dd日"))
                     .arg(m_selectedTime);
                     
    QMessageBox::information(this, "面试预约成功", message);
} 
