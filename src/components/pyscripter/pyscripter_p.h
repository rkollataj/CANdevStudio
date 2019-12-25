#ifndef PYSCRIPTER_P_H
#define PYSCRIPTER_P_H

#include "pyscripter.h"
#include <memory>
#include <pythonbackend.h>

class PyScripter;

class PyScripterPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(PyScripter)

public:
    PyScripterPrivate(PyScripter* q, PyScripterCtx&& ctx = PyScripterCtx());
    ComponentInterface::ComponentProperties getSupportedProperties() const;
    QJsonObject getSettings();
    void setSettings(const QJsonObject& json);

private:
    void initProps();

public:
    bool _simStarted{ false };
    PyScripterCtx _ctx;
    std::map<QString, QVariant> _props;
    PythonBackend _pyHandler;

private:
    PyScripter* q_ptr;
    const QString _nameProperty = "name";

    // workaround for clang 3.5
    using cf = ComponentInterface::CustomEditFieldCbk;

    // clang-format off
    ComponentInterface::ComponentProperties _supportedProps = {
            std::make_tuple(_nameProperty, QVariant::String, true, cf(nullptr))
    };
    // clang-format on
};

#endif // PYSCRIPTER_P_H
