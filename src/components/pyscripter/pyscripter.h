#ifndef PYSCRIPTER_H
#define PYSCRIPTER_H

#include <QWidget>
#include <QtCore/QScopedPointer>
#include <common/context.h>
#include <componentinterface.h>
#include <datamodeltypes/datadirection.h>
#include <memory>

class PyScripterPrivate;
class QWidget;
typedef Context<> PyScripterCtx;

class QCanBusFrame;

class PyScripter : public QObject, public ComponentInterface {
    Q_OBJECT
    Q_DECLARE_PRIVATE(PyScripter)

public:
    PyScripter();
    explicit PyScripter(PyScripterCtx&& ctx);
    ~PyScripter();

    static void typeInit();

    QWidget* mainWidget() override;
    void setConfig(const QJsonObject& json) override;
    void setConfig(const QWidget& qobject) override;
    QJsonObject getConfig() const override;
    std::shared_ptr<QWidget> getQConfig() const override;
    void configChanged() override;
    bool mainWidgetDocked() const override;
    ComponentInterface::ComponentProperties getSupportedProperties() const override;

signals:
    void mainWidgetDockToggled(QWidget* widget) override;
    void simBcastSnd(const QJsonObject& msg, const QVariant& param = QVariant()) override;

public slots:
    void stopSimulation() override;
    void startSimulation() override;
    void simBcastRcv(const QJsonObject& msg, const QVariant& param) override;
    void rcvFrame(const QCanBusFrame& frame, Direction direction, bool status);

private:
    QScopedPointer<PyScripterPrivate> d_ptr;
};

#endif // PYSCRIPTER_H
