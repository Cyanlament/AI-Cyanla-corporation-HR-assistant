#include "map.h"
#include "ui_map.h"
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>

Map::Map(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Map)
{
    ui->setupUi(this);


    // ui->Lib
    // 设置按钮样式
    QString buttonStyle =
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   color: rgba(255, 255, 255, 150);"
        "   padding: 12px 24px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   border-radius: 4px;"
        "   transition: all 0.3s ease;"
        "}"
        "QPushButton:hover {"
        "   color: rgba(255, 255, 255, 255);"
        "   transform: scale(1.05);"
        "}"
        "QPushButton:pressed {"
        "   background: rgba(255, 255, 255, 50);"
        "   transform: scale(0.95);"
        "}";

    ui->Ket->setStyleSheet(buttonStyle);

     ui->Hoc->setStyleSheet(buttonStyle);
      ui->Che->setStyleSheet(buttonStyle);
       ui->Ti->setStyleSheet(buttonStyle);
        ui->Net->setStyleSheet(buttonStyle);
         ui->Hod->setStyleSheet(buttonStyle);
          ui->Yes->setStyleSheet(buttonStyle);
           ui->Mal->setStyleSheet(buttonStyle);
            ui->Bin->setStyleSheet(buttonStyle);
           ui->Geb->setStyleSheet(buttonStyle);

}

Map::~Map()
{
    delete ui;
}
