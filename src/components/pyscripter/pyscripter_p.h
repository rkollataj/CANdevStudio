#ifndef PYSCRIPTER_P_H
#define PYSCRIPTER_P_H

#include "pyscripter.h"
#include <PythonQt.h>
#include <QCanBusFrame>
#include <memory>
#include <propertyfields.h>

Q_DECLARE_METATYPE(QCanBusFrame);

class PyScripter;

class PyScripterPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(PyScripter)

public:
    PyScripterPrivate(PyScripter* q, PyScripterCtx&& ctx = PyScripterCtx());
    ComponentInterface::ComponentProperties getSupportedProperties() const;
    QJsonObject getSettings();
    void setSettings(const QJsonObject& json);
    void loadScript(const QString& script);

private:
    void initProps();

public:
    bool _simStarted{ false };
    PyScripterCtx _ctx;
    std::map<QString, QVariant> _props;
    PythonQtObjectPtr _pyModule;

private:
    PyScripter* q_ptr;
    const QString _nameProperty = "name";
    const QString _scriptProperty = "script";

    // workaround for clang 3.5
    using cf = ComponentInterface::CustomEditFieldCbk;

    // clang-format off
    ComponentInterface::ComponentProperties _supportedProps = {
            std::make_tuple(_nameProperty, QVariant::String, true, cf(nullptr)),
            std::make_tuple(_scriptProperty, QVariant::String, true, cf([] { return new PropertyFieldPath; } )),
    };
    // clang-format on
};

#endif // PYSCRIPTER_P_H
