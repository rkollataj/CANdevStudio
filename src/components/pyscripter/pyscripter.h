#ifndef PYSCRIPTER_H
#define PYSCRIPTER_H

#include <QWidget>
#include <QtCore/QScopedPointer>
#include <componentinterface.h>
#include <componentcontext.h>
#include <memory>
#include <QJsonArray>

class PyScripterPrivate;
class QWidget;
typedef Context<> PyScripterCtx;

class PyScripter : public QObject, public ComponentInterface {
    Q_OBJECT
    Q_DECLARE_PRIVATE(PyScripter)

public:
    PyScripter();
    explicit PyScripter(PyScripterCtx&& ctx);
    ~PyScripter();

    QWidget* mainWidget() override;
    void setConfig(const QJsonObject& json) override;
    void setConfig(const QWidget& qobject) override;
    QJsonObject getConfig() const override;
    std::shared_ptr<QWidget> getQConfig() const override;
    void configChanged() override;
    bool mainWidgetDocked() const override;
    ComponentInterface::ComponentProperties getSupportedProperties() const override;
    QJsonArray inTypes() const;
    QJsonArray outTypes() const;

signals:
    void mainWidgetDockToggled(QWidget* widget) override;

public slots:
    void stopSimulation() override;
    void startSimulation() override;

private:
    QScopedPointer<PyScripterPrivate> d_ptr;
};

#endif //PYSCRIPTER_H
