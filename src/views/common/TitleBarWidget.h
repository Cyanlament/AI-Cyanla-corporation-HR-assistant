#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class TitleBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBarWidget(QWidget *parent = nullptr);
    
    void setTitle(const QString& title);
    void setUserInfo(const QString& username, const QString& role);

private:
    void setupUI();

private:
    QHBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QLabel* m_userLabel;
    QLabel* m_timeLabel;
    
    QString m_currentTitle;

private slots:
    void updateTime();

signals:
};

#endif // TITLEBARWIDGET_H 