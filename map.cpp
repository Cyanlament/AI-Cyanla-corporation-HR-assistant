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
}

Map::~Map()
{
    delete ui;
}
