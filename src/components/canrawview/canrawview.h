#ifndef CANRAWVIEW_H
#define CANRAWVIEW_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtWidgets/QWidget>
#include <memory>
#include <context.h>

class QCanBusFrame;
class CanRawViewPrivate;
struct CRVFactoryInterface;
class QWidget;

class CanRawView : public QObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(CanRawView)

public:
    explicit CanRawView();
    explicit CanRawView(CanRawViewCtx *ctx);
    ~CanRawView();

    void saveSettings(QJsonObject& json) const;
    QWidget* getMainWidget();

signals:
    void dockUndock();

public Q_SLOTS:
    void frameReceived(const QCanBusFrame& frame);
    void frameSent(bool status, const QCanBusFrame& frame);
    void stopSimulation(void);
    void startSimulation(void);

private:
    QScopedPointer<CanRawViewPrivate> d_ptr;
};

#endif // CANRAWVIEW_H
