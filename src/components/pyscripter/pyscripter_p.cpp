#include "pyscripter_p.h"
#include "qcanbusframedecorator.h"
#include <QJsonArray>
#include <log.h>

namespace {
static bool pythonInitiated = false;
void initPythonQt()
{
    if (!pythonInitiated) {
        PythonQt::init(PythonQt::IgnoreSiteModule);
        PythonQt::self()->registerCPPClass(
            "QCanBusFrame", nullptr, nullptr, PythonQtCreateObject<QCanBusFrameDecorator>);
    }

    pythonInitiated = true;
}
} // namespace

PyScripterPrivate::PyScripterPrivate(PyScripter* q, PyScripterCtx&& ctx)
    : _ctx(std::move(ctx))
    , q_ptr(q)
{
    initProps();
    initPythonQt();
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

void PyScripterPrivate::loadScript(const QString& script)
{
    _pyModule = PythonQt::self()->createUniqueModule();
    _pyModule.evalFile(script);

    _outHandler = std::make_unique<OutHandler>();
    PythonQt::self()->addObject(_pyModule, "outHandler", _outHandler.get());
}
