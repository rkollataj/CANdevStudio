#include "pyscripter_p.h"
#include "qcanbusframedecorator.h"
#include <QJsonArray>
#include <datamodeltypes/canrawdata.h>
#include <log.h>

namespace {
static bool pythonInitiated = false;
void initPythonQt()
{
    if (!pythonInitiated) {
        PythonQt::init(PythonQt::IgnoreSiteModule);
        PythonQt::self()->registerCPPClass(
            "QCanBusFrame", nullptr, nullptr, PythonQtCreateObject<QCanBusFrameDecorator>);
        PythonQt::self()->registerCPPClass("PortTypes", nullptr, nullptr, PythonQtCreateObject<PortTypes>);
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

    _outHandler = std::make_unique<OutHandler>();
    connect(_outHandler.get(), qOverload<const QVariant&>(&OutHandler::send), q_ptr,
        qOverload<const QVariant&>(&PyScripter::send));
    connect(_outHandler.get(), qOverload<const QVariant&, int>(&OutHandler::send), q_ptr,
        qOverload<const QVariant&, int>(&PyScripter::send));
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

    PythonQt::self()->addObject(_pyModule, "out", _outHandler.get());
}

void PyScripterPrivate::initPortTypes(const QString& func)
{
    QVariantList vl;
    QVariant v = _pyModule.call(func);
    std::vector<QtNodes::NodeDataType>* vec = nullptr;

    if (func == "inTypes") {
        vec = &_inTypes;
    } else if (func == "outTypes") {
        vec = &_outTypes;
    } else {
        cds_error("Wrong function passed '{}'", func.toStdString());
        return;
    }

    vec->clear();

    // Allow handling arrays and value
    if (v.isValid() && v.canConvert<QVariantList>()) {
        vl = v.value<QVariantList>();
    } else {
        vl << v;
    }

    int ndx = 0;
    for (auto& port : vl) {
        QString altName = "";
        PortTypes::PortType pt = PortTypes::Invalid;

        if (port.canConvert<QVariantList>()) {
            QVariantList vl2 = port.value<QVariantList>();

            if (func == "inTypes") {
                if (vl2.size() >= 2) {
                    port = vl2[0];

                    if (vl2[1].canConvert<QString>()) {
                        _inClbks.push_back(vl2[1].value<QString>());
                    }
                }

                if (vl2.size() >= 3) {
                    if (vl2[2].canConvert<QString>()) {
                        altName = vl2[2].value<QString>();
                    }
                }

            } else {
                if (vl2.size() >= 2) {
                    port = vl2[0];

                    if (vl2[1].canConvert<QString>()) {
                        altName = vl2[1].value<QString>();
                    }
                }
            }
        }

        if (port.canConvert<PortTypes::PortType>()) {
            pt = port.value<PortTypes::PortType>();
        }

        switch (pt) {
        case PortTypes::RawFrame:
            if (altName.length() > 0) {
                vec->push_back(NodeDataType{ "rawframe", altName });
            } else {
                vec->push_back(CanRawData().type());
            }
            break;

        case PortTypes::Invalid:
        default:
            cds_warn("Failed to recognize port, {}, {}", func.toStdString(), ndx);
            break;
        }

        ndx++;
    }
}

void PyScripterPrivate::initTimerTypes()
{
    QVariantList vl;
    QVariant v = _pyModule.call("timerTypes");
    _timerTypes.clear();

    if (v.canConvert<QVariantList>()) {
        vl = v.value<QVariantList>();
    }

    for (auto& timer : vl) {
        uint32_t timeout = 0;
        QString clbk = "";
        QVariantList vl2;

        if (timer.canConvert<QVariantList>()) {
            vl2 = timer.value<QVariantList>();
        }

        if (vl2.size() > 1) {
            if (vl2[0].canConvert<int>()) {
                timeout = vl2[0].value<int>();
            }

            if (vl2[1].canConvert<QString>()) {
                clbk = vl2[1].value<QString>();
            }
        }

        if ((timeout > 0) && (clbk.length() > 0)) {
            cds_debug("Adding timer {}. {}", timeout, clbk.toStdString());

            auto qt = std::make_shared<QTimer>();
            qt->setInterval(timeout);
            qt->setTimerType(Qt::PreciseTimer);
            connect(qt.get(), &QTimer::timeout, [this, clbk] { _pyModule.call(clbk); });
            _timerTypes.push_back(qt);
        }
    }
}
