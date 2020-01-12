#include "pyscripter.h"
#include "pyscripter_p.h"
#include "pythonbackend.h"
#include <confighelpers.h>
#include <log.h>
#include <datamodeltypes/datadirection.h>

PyScripter::PyScripter()
    : d_ptr(new PyScripterPrivate(this))
{
}

PyScripter::PyScripter(PyScripterCtx&& ctx)
    : d_ptr(new PyScripterPrivate(this, std::move(ctx)))
{
}

PyScripter::~PyScripter() {}

void PyScripter::typeInit()
{
    PythonBackend::_appShm.createShm(CdsShMem::id);
}

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

void PyScripter::configChanged() {}

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
    d->_pyHandler.stop();
}

void PyScripter::startSimulation()
{
    Q_D(PyScripter);

    d->_simStarted = true;
    d->_pyHandler.start("");
}

void PyScripter::simBcastRcv(const QJsonObject& msg, const QVariant& param)
{
    Q_UNUSED(msg);
    Q_UNUSED(param);
}

void PyScripter::rcvFrame(const QCanBusFrame& frame, Direction direction, bool status)
{
    Q_D(PyScripter);

    if (d->_simStarted && status) {
        QString dir;

        if (direction == Direction::TX) {
            dir = "TX";
        } else if (direction == Direction::RX) {
            dir = "RX";
        } else {
            cds_error("Wrong direction!");
            return;
        }

        d->_pyHandler.sendMsgFrame(frame, dir);
    }
}

