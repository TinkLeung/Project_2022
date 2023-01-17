#include "CBadBlkView.h"
#include <QObject>
#include <QtWidgets>
#include <QTableView>
#include <QStandardItemModel>


CBabBlkView::CBabBlkView(QWidget *parent) : QDialog(parent)
{
    QTableView *oriBbView = new QTableView(this);
    QTableView *newBbView = new QTableView(this);
    QTableView *totalBbView = new QTableView(this);

    QStandardItemModel *model0 = new QStandardItemModel();

    model0->setHorizontalHeaderLabels({"Total","Original","Erase","Program","ECC"});
    totalBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //totalBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    totalBbView->verticalHeader()->hide();
    totalBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model0->setItem(0, 0, new QStandardItem (QString::asprintf("%d", 0)));     //Total
    model0->setItem(0, 1, new QStandardItem (QString::asprintf("%d", 0)));     //Original
    model0->setItem(0, 2, new QStandardItem (QString::asprintf("%d", 0)));     //Erase
    model0->setItem(0, 3, new QStandardItem (QString::asprintf("%d", 0)));     //Program
    model0->setItem(0, 4, new QStandardItem (QString::asprintf("%d", 0)));     //ECC
    //model0->setRowCount(2);
    totalBbView->setModel(model0);

    QStandardItemModel *model1 = new QStandardItemModel();
    model1->setHorizontalHeaderLabels({"Ori BB","CH 0","CH 1","CH 2","CH 3","CH 4","CH 5","CH 6","CH 7"});
    oriBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    oriBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    oriBbView->verticalHeader()->hide();
    oriBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int ce=0; ce<4; ce++)
    {
        model1->setItem(ce, 0, new QStandardItem (QString::asprintf("CE %d", ce)));
        for(int ch=0; ch<8; ch++)
        {
            model1->setItem(ce, ch+1, new QStandardItem (QString::asprintf("%d", 0)));

        }
    }
    oriBbView->setModel(model1);

    QStandardItemModel *model2 = new QStandardItemModel();
    model2->setHorizontalHeaderLabels({"new BB","CH 0","CH 1","CH 2","CH 3","CH 4","CH 5","CH 6","CH 7"});
    newBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newBbView->verticalHeader()->hide();
    newBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int ce=0; ce<4; ce++)
    {
        model2->setItem(ce, 0, new QStandardItem (QString::asprintf("CE %d", ce)));
        for(int ch=0; ch<8; ch++)
        {
            model2->setItem(ce, ch+1, new QStandardItem (QString::asprintf("%d", 0)));

        }
    }
    newBbView->setModel(model2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(totalBbView);
    mainLayout->addWidget(oriBbView);
    mainLayout->addWidget(newBbView);
    mainLayout->setStretchFactor(totalBbView, 1);
    mainLayout->setStretchFactor(oriBbView, 5);
    mainLayout->setStretchFactor(newBbView, 5);
    setLayout(mainLayout);

}
