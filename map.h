#ifndef MAP_H
#define MAP_H

#include <QDialog>

namespace Ui {
class Map;
}

class Map : public QDialog
{
    Q_OBJECT

public:
    explicit Map(QWidget *parent = nullptr);
    ~Map();
    // 重写 showEvent
    void showEvent(QShowEvent *event) override
    {
        // 在窗口显示时执行的操作
        qDebug() << "Window is shown!";

        // 调用基类的 showEvent 方法以保留默认行为
        QWidget::showEvent(event);
    }
private:
    Ui::Map *ui;
};

#endif // MAP_H
