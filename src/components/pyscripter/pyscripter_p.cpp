#include "pyscripter_p.h"
#include <QUuid>
#include <log.h>

PyScripterPrivate::PyScripterPrivate(PyScripter* q, PyScripterCtx&& ctx)
    : _ctx(std::move(ctx))
    , q_ptr(q)
{
    initProps();

    connect(&_pyHandler, &PythonBackend::sndFrame, q, &PyScripter::sndFrame);
    connect(&_pyHandler, &PythonBackend::sndSignal, q, &PyScripter::sndSignal);
}

void PyScripterPrivate::initProps()
{
    for (const auto& p : _supportedProps) {
        _props[ComponentInterface::propertyName(p)];
    }
}

ComponentInterface::ComponentProperties PyScripterPrivate::getSupportedProperties() const
{
    return _supportedProps;
}

QJsonObject PyScripterPrivate::getSettings()
{
    QJsonObject json;

    for (const auto& p : _props) {
        json[p.first] = QJsonValue::fromVariant(p.second);
    }

    return json;
}

void PyScripterPrivate::setSettings(const QJsonObject& json)
{
    for (const auto& p : _supportedProps) {
        const QString& propName = ComponentInterface::propertyName(p);
        if (json.contains(propName))
            _props[propName] = json[propName].toVariant();
    }
}
