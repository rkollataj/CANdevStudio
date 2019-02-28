#include "pyscripter.h"
#include "pyscripter_p.h"
#include <confighelpers.h>
#include <log.h>

PyScripter::PyScripter()
    : d_ptr(new PyScripterPrivate(this))
{
}

PyScripter::PyScripter(PyScripterCtx&& ctx)
    : d_ptr(new PyScripterPrivate(this, std::move(ctx)))
{
}

PyScripter::~PyScripter()
{
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

void PyScripter::configChanged()
{
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
