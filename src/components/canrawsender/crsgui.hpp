#ifndef CRSGUI_HPP
#define CRSGUI_HPP

#include "crsguiinterface.hpp"
#include "ui_canrawsender.h"
#include "ui_configwindow.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QMenu>
#include <cdswidget.h>
#include <memory>

namespace Ui {
class CanRawSenderPrivate;
}

namespace Ui {
class ConfigWindow;
}

struct CRSGui : public CRSGuiInterface {
    CRSGui()
        : ui(new Ui::CanRawSenderPrivate)
        , widget(new CDSWidget)
    {
        ui->setupUi(widget);

        _menu.addAction(ui->actionOpen);
        _menu.addAction(ui->actionConfigure);

        QObject::connect(ui->actionOpen, &QAction::triggered, [this]() {
            if (widget->parentWidget()) {
                widget->parentWidget()->show();
            } else {
                widget->show();
            }
        });

        QObject::connect(ui->actionConfigure, &QAction::triggered, [this]() {
            QDialog dialog;
            Ui::ConfigWindow configWindow;
            configWindow.setupUi(&dialog);
            dialog.exec();
        });
    }

    void setAddCbk(const add_t& cb) override { QObject::connect(ui->pbAdd, &QPushButton::pressed, cb); }

    void setRemoveCbk(const remove_t& cb) override { QObject::connect(ui->pbRemove, &QPushButton::pressed, cb); }

    void setDockUndockCbk(const dockUndock_t& cb) override
    {
        QObject::connect(ui->pbDockUndock, &QPushButton::pressed, cb);
    }

    QWidget* getMainWidget() override { return widget; }

    void initTableView(QAbstractItemModel& tvModel) override
    {

        ui->tv->setModel(&tvModel);
        ui->tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    }

    QModelIndexList getSelectedRows() override { return ui->tv->selectionModel()->selectedRows(); }

    void setIndexWidget(const QModelIndex& index, QWidget* widget) override { ui->tv->setIndexWidget(index, widget); }

    QMenu& getMenu() override { return _menu; }

private:
    Ui::CanRawSenderPrivate* ui;
    CDSWidget* widget;
    QMenu _menu;
};
#endif // CRSGUI_HPP
