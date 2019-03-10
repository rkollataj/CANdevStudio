#include "pyscripter.h"
#include "pyscripter_p.h"
#include <QCanBusFrame>
#include <confighelpers.h>
#include <datamodeltypes/canrawdata.h>
#include <log.h>

PyScripter::PyScripter()
    : d_ptr(new PyScripterPrivate(this))
{
}

PyScripter::PyScripter(PyScripterCtx&& ctx)
    : d_ptr(new PyScripterPrivate(this, std::move(ctx)))
{
}

PyScripter::~PyScripter() {}

QWidget* PyScripter::mainWidget()
{
    // Component does not have main widget
    return nullptr;
}

void PyScripter::setConfig(const QJsonObject& json)
{
    Q_D(PyScripter);

    d->setSettings(json);
}

void PyScripter::setConfig(const QWidget& qobject)
{
    Q_D(PyScripter);

    configHelpers::setQConfig(qobject, getSupportedProperties(), d->_props);
}

QJsonObject PyScripter::getConfig() const
{
    return d_ptr->getSettings();
}

std::shared_ptr<QWidget> PyScripter::getQConfig() const
{
    const Q_D(PyScripter);

    return configHelpers::getQConfig(getSupportedProperties(), d->_props);
}

void PyScripter::configChanged()
{
    QString scriptName = getQConfig()->property(d_ptr->_scriptProperty.toStdString().c_str()).toString();

    cds_info("Script to open: '{}'", scriptName.toStdString());

    d_ptr->loadScript(scriptName);

    auto&& initPortTypes = [this](std::vector<QtNodes::NodeDataType>& vec, const QString& func) {
        QVariantList vl;
        QVariant v = d_ptr->_pyModule.call(func);
        vec.clear();

        // Allow handling arrays and value
        if (v.isValid() && v.canConvert<QVariantList>()) {
            vl = v.value<QVariantList>();
        } else {
            vl << v;
        }

        int ndx = 0;
        for (auto& port : vl) {
            PortTypes::PortType pt = PortTypes::Invalid;

            if (port.canConvert<QVariantList>()) {
                // In ports are defined as a tuple
                QVariantList vl2 = port.value<QVariantList>();

                if (vl2.size() > 1) {
                    port = vl2[0];

                    if (vl2[1].canConvert<QString>()) {
                        d_ptr->_inClbks.push_back(vl2[1].value<QString>());
                    }
                }
            }

            if (port.canConvert<PortTypes::PortType>()) {
                pt = port.value<PortTypes::PortType>();
            }

            switch (pt) {
            case PortTypes::RawFrame:
                vec.push_back(CanRawData().type());
                break;

            case PortTypes::Invalid:
            default:
                cds_warn("Failed to recognize port, {}, {}", func.toStdString(), ndx);
                break;
            }

            ndx++;
        }
    };

    d_ptr->_inClbks.clear();
    initPortTypes(d_ptr->_inTypes, "inTypes");
    initPortTypes(d_ptr->_outTypes, "outTypes");

    emit scriptLoaded();
}

bool PyScripter::mainWidgetDocked() const
{
    // Widget does not exist. Return always true
    return true;
}

ComponentInterface::ComponentProperties PyScripter::getSupportedProperties() const
{
    return d_ptr->getSupportedProperties();
}

void PyScripter::stopSimulation()
{
    Q_D(PyScripter);

    d->_simStarted = false;
}

void PyScripter::startSimulation()
{
    Q_D(PyScripter);

    d->_simStarted = true;
}

std::vector<QtNodes::NodeDataType> PyScripter::inTypes() const
{
    return d_ptr->_inTypes;
}

std::vector<QtNodes::NodeDataType> PyScripter::outTypes() const
{
    return d_ptr->_outTypes;
}

void PyScripter::receive(const QVariantList& list, int portNdx)
{
    if (portNdx < static_cast<int>(d_ptr->_inClbks.size())) {
        d_ptr->_pyModule.call(d_ptr->_inClbks[portNdx], list);
    } else {
        cds_warn("Callback in python script for port {} not found", portNdx);
    }
}
