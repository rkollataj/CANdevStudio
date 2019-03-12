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

    d_ptr->_inClbks.clear();
    d_ptr->initPortTypes("inTypes");
    d_ptr->initPortTypes("outTypes");
    d_ptr->initTimerTypes();

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

    for (auto&& timer : d_ptr->_timerTypes) {
        timer->stop();
    }
}

void PyScripter::startSimulation()
{
    Q_D(PyScripter);

    d->_simStarted = true;

    for (auto&& timer : d_ptr->_timerTypes) {
        timer->start();
    }
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
    if (d_ptr->_simStarted) {
        if (portNdx < static_cast<int>(d_ptr->_inClbks.size())) {
            d_ptr->_pyModule.call(d_ptr->_inClbks[portNdx], list);
        } else {
            cds_warn("Callback in python script for port {} not found", portNdx);
        }
    }
}
