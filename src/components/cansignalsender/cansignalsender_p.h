#ifndef CANSIGNALSENDER_P_H
#define CANSIGNALSENDER_P_H

#include <QtCore/QObject>
#include <memory>
#include "gui/cansignalsenderguiimpl.h"
#include "cansignalsender.h"
#include <QStandardItemModel>
#include <cantypes.hpp>
#include <QTimer>

class CanSignalSender;

class CanSignalSenderPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(CanSignalSender)

public:
    CanSignalSenderPrivate(CanSignalSender* q, CanSignalSenderCtx&& ctx = CanSignalSenderCtx(new CanSignalSenderGuiImpl));
    ComponentInterface::ComponentProperties getSupportedProperties() const;
    QJsonObject getSettings();
    void setSettings(const QJsonObject& json);

private:
    void initProps();

public:
    bool _simStarted{ false };
    CanSignalSenderCtx _ctx;
    CanSignalSenderGuiInt& _ui;
    bool _docked{ true };
    std::map<QString, QVariant> _props;

private:
    QStandardItemModel _tvModel;
    QStringList _tvColumns;
    std::map<uint32_t, QStringList> _signalNames;
    CanSignalSender* q_ptr;
    CANmessages_t _messages;
    const QString _nameProperty = "name";
    ComponentInterface::ComponentProperties _supportedProps = {
            {_nameProperty,   {QVariant::String, true}}
    };
    std::map<std::string, std::unique_ptr<QTimer>> _timerMap;
};

#endif // CANSIGNALSENDER_P_H
