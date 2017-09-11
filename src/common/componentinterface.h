#ifndef __COMPONENTINTERFACE_H
#define __COMPONENTINTERFACE_H

#include <QtCore/QJsonObject>

class QWidget;

struct ComponentInterface {
    virtual ~ComponentInterface() {}

    virtual void stopSimulation() = 0;
    virtual void startSimulation() = 0;
    virtual void setConfig(QJsonObject& json) = 0;
    virtual QJsonObject getConfig() const = 0;
    virtual QWidget* getMainWidget() = 0;
};

#endif /* !__COMPONENTINTERFACE_H */
