#include "pyscripter.h"
#include "pyscripter_p.h"
#include "pythonbackend.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <confighelpers.h>
#include <datamodeltypes/datadirection.h>
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

    d->_settings.setValue(d->_editorProperty, d->_props[d->_editorProperty]);
    d->_settings.setValue(d->_editorArgsProperty, d->_props[d->_editorArgsProperty]);
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
    d->_pyHandler.startScript(d->_props[d->_scriptProperty].toString());
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
        d->_pyHandler.sendMsgFrame(frame, direction);
    }
}

void PyScripter::rcvSignal(const QString& name, const QVariant& val, Direction direction)
{
    Q_D(PyScripter);

    if (d->_simStarted) {
        auto nameSplit = name.split('_');

        if (nameSplit.size() < 2) {
            cds_error("Wrong signal name: {}", name.toStdString());
            return;
        }

        uint32_t id = nameSplit[0].toUInt(nullptr, 16);

        QString sigName = name.mid(name.indexOf("_") + 1);

        d->_pyHandler.sendMsgSignal(id, sigName, val.toDouble(), direction);
    }
}

std::vector<QAction*> PyScripter::getCustomMenu()
{
    Q_D(PyScripter);

    // Ownership to be handled by caller
    QAction* editAction = new QAction("Edit script");

    QFileInfo scriptFi(d->_props[d->_scriptProperty].toString());
    QFileInfo editorFi(d->_props[d->_editorProperty].toString());
    QString editorArgs = d->_props[d->_editorArgsProperty].toString();

    if (scriptFi.exists()) {
        if (editorFi.exists()) {
            connect(editAction, &QAction::triggered, [scriptFi, editorFi, editorArgs] {
                QStringList args;
                if (editorArgs.length() > 0) {
                    for (auto& a : editorArgs.split(" ")) {
                        args << a;
                    }
                }
                args << scriptFi.absoluteFilePath();

                cds_error("{}", args.join("_").toStdString());

                QProcess p;
                p.startDetached(editorFi.absoluteFilePath(), args);
            });
        } else {
            connect(editAction, &QAction::triggered,
                [scriptFi] { QDesktopServices::openUrl(QUrl::fromLocalFile(scriptFi.absoluteFilePath())); });
        }
    } else {
        editAction->setEnabled(false);
    }

    std::vector<QAction*> ret = { editAction };

    return ret;
}
