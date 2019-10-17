#ifndef CANSIGNALSENDERGUIINT_H
#define CANSIGNALSENDERGUIINT_H

#include <Qt>
#include <functional>
#include <QJsonArray>
#include <QModelIndexList>

class QWidget;
class QStandardItemModel;

struct CanSignalSenderGuiInt {
    using dockUndock_t = std::function<void()>;
    using remove_t = std::function<void()>;
    using add_t = std::function<void()>;
    using send_t = std::function<void(const QString&, const QString&, const QVariant&, bool, uint32_t)>;

    virtual ~CanSignalSenderGuiInt()
    {
    }

    virtual QWidget* mainWidget() = 0;
    virtual void initTv(QStandardItemModel& tvModel, std::map<uint32_t, QStringList>& sigNames) = 0;
    virtual void setRemoveCbk(const remove_t& cb) = 0;
    virtual void setAddCbk(const add_t& cb) = 0;
    virtual void setDockUndockCbk(const dockUndock_t& cb) = 0;
    virtual void setSendCbk(const send_t& cbk) = 0;
    virtual void addRow(const QString& id = "", const QString& sig = "", const QString& val = "") = 0;
    virtual QJsonArray getRows() = 0;
    virtual QModelIndexList getSelectedRows() = 0;
};

#endif // CANSIGNALSENDERGUIINT_H
