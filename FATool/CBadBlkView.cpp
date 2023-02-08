#include "CBadBlkView.h"
#include <QObject>
#include <QtWidgets>
#include <QTableView>
#include <QStandardItemModel>
#include "PhysicalDriveInfo.h"

extern BAD_BLOCK_INFO BbInfo;

CBabBlkView::CBabBlkView(QWidget *parent) : QDialog(parent)
{
    QTableView *oriBbView = new QTableView(this);
    QTableView *newBbView = new QTableView(this);
    QTableView *totalBbView = new QTableView(this);

    QStandardItemModel *model0 = new QStandardItemModel();

    model0->setHorizontalHeaderLabels({"Total","Original Bad","Erase Fail","Program Fail","Read Fail"});
    totalBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //totalBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    totalBbView->verticalHeader()->hide();
    totalBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model0->setItem(0, 0, new QStandardItem (QString::asprintf("%d", BbInfo.TotalBadBlock)));     //Total bad
    model0->setItem(0, 1, new QStandardItem (QString::asprintf("%d", BbInfo.TotalOriBad)));     //Original bad
    model0->setItem(0, 2, new QStandardItem (QString::asprintf("%d", BbInfo.ToTalEraseFail)));     //Erase fail
    model0->setItem(0, 3, new QStandardItem (QString::asprintf("%d", BbInfo.TotalProgFail)));     //Program fail
    model0->setItem(0, 4, new QStandardItem (QString::asprintf("%d", BbInfo.TotalReadFail)));     //Read fail
    //model0->setRowCount(2);
    totalBbView->setModel(model0);

    QStandardItemModel *model1 = new QStandardItemModel();
    model1->setHorizontalHeaderLabels({"Ori BB","CH 0","CH 1","CH 2","CH 3","CH 4","CH 5","CH 6","CH 7"});
    oriBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    oriBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    oriBbView->verticalHeader()->hide();
    oriBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int ce=0; ce < MAX_CE; ce++)
    {
        model1->setItem(ce, 0, new QStandardItem (QString::asprintf("CE %d", ce)));
        for(int ch=0; ch < MAX_CH; ch++)
        {            
            model1->setItem(ce, ch+1, new QStandardItem (QString::asprintf("%d", BbInfo.BadBlkPerLun[ch][ce].OriBad)));
        }
    }
    oriBbView->setModel(model1);

    QStandardItemModel *model2 = new QStandardItemModel();
    model2->setHorizontalHeaderLabels({"new BB","CH 0","CH 1","CH 2","CH 3","CH 4","CH 5","CH 6","CH 7"});
    newBbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newBbView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newBbView->verticalHeader()->hide();
    newBbView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int ce=0; ce<MAX_CE; ce++)
    {
        model2->setItem(ce, 0, new QStandardItem (QString::asprintf("CE %d", ce)));
        for(int ch=0; ch<MAX_CH; ch++)
        {

            model2->setItem(ce, ch+1, new QStandardItem (QString::asprintf("%d", BbInfo.BadBlkPerLun[ch][ce].LaterBad)));

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
