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
    void onDoctorSelected();
    void onDateSelected(const QDate& date);
    void onTimeSlotSelected();
    void makeAppointment();

private:
    void setupUI();
    void loadDepartments();
    void loadDoctors();
    void loadTimeSlots();
    void updateAppointmentInfo();

private:
    QVBoxLayout* m_mainLayout;
    
    // 科室选择
    QGroupBox* m_departmentGroup;
    QComboBox* m_departmentCombo;
    
    // 医生选择
    QGroupBox* m_doctorGroup;
    QListWidget* m_doctorList;
    
    // 日期选择
    QGroupBox* m_dateGroup;
    QCalendarWidget* m_calendar;
    
    // 时间段选择
    QGroupBox* m_timeGroup;
    QListWidget* m_timeList;
    
    // 预约信息
    QGroupBox* m_infoGroup;
    QLabel* m_appointmentInfo;
    QPushButton* m_confirmButton;
    
    // 当前选择
    QString m_selectedDepartment;
    QString m_selectedDoctor;
    QDate m_selectedDate;
    QString m_selectedTime;
};

#endif // APPOINTMENTWIDGET_H 