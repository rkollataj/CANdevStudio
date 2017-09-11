#ifndef __COMPONENTINTERFACE_H
#define __COMPONENTINTERFACE_H

#include <QtCore/QJsonObject>
#include <functional>

class QWidget;

struct ComponentInterface {
    virtual ~ComponentInterface() {}

    virtual void stopSimulation() = 0;
    virtual void startSimulation() = 0;
    virtual void setConfig(QJsonObject& json) = 0;
    virtual QJsonObject getConfig() const = 0;
    virtual QWidget* getMainWidget() = 0;
    // Do nothing by default. Override in component to handle dock/undock action
    virtual void setDockUndockClbk(const std::function<void()>&) {}
};

#endif /* !__COMPONENTINTERFACE_H */
