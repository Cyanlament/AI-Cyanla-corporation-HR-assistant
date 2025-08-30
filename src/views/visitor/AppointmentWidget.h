#ifndef APPOINTMENTWIDGET_H
#define APPOINTMENTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCalendarWidget>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>

class AppointmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AppointmentWidget(QWidget *parent = nullptr);

private slots:
    void onDepartmentChanged();
    void onSephirahSelected();
    void onDateSelected(const QDate& date);
    void onTimeSlotSelected();
    void makeAppointment();

private:
    void setupUI();
    void loadDepartments();
    void loadSephirahs();
    void loadTimeSlots();
    void updateAppointmentInfo();

private:
    QVBoxLayout* m_mainLayout;
    
    // 部门选择
    QGroupBox* m_departmentGroup;
    QComboBox* m_departmentCombo;
    
    // 部长选择
    QGroupBox* m_sephirahGroup;
    QListWidget* m_sephirahList;
    
    // 日期选择
    QGroupBox* m_dateGroup;
    QCalendarWidget* m_calendar;
    
    // 时间段选择
    QGroupBox* m_timeGroup;
    QListWidget* m_timeList;
    
    // 面试预约信息
    QGroupBox* m_infoGroup;
    QLabel* m_appointmentInfo;
    QPushButton* m_confirmButton;
    
    // 当前选择
    QString m_selectedDepartment;
    QString m_selectedSephirah;
    QDate m_selectedDate;
    QString m_selectedTime;
};

#endif // APPOINTMENTWIDGET_H 
