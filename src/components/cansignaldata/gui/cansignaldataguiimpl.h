#ifndef CANSIGNALDATAGUIIMPL_H
#define CANSIGNALDATAGUIIMPL_H

#include <QWidget>
#include "cansignaldataguiint.h"
#include "ui_cansignaldata.h"
#include "searchmodel.h"
#include <QItemDelegate>
#include <QItemEditorFactory>
#include <QStyledItemDelegate>
#include <log.h>
#include <QButtonGroup>

class IntervalLE : public QLineEdit
{
public:
    IntervalLE(uint32_t, QWidget *parent) : QLineEdit(parent)
    {
        QRegExp qRegExp("[0-9]*");
        auto v = new QRegExpValidator(qRegExp, this);
        setValidator(v);
    }
};

class InitValLE : public QLineEdit
{
public:
    InitValLE(uint32_t len, QWidget *parent) : QLineEdit(parent)
    {
        QRegExp qRegExp("[0-9A-Fa-f]*");
        auto v = new QRegExpValidator(qRegExp, this);
        setValidator(v);
    }
};

template<typename T>
class EditorCreator : public QItemEditorCreatorBase
{
public:
    EditorCreator(uint32_t len) :
        _len(len)
    {
    } 

    QWidget* createWidget(QWidget *parent) const override
    {
        cds_error("createWidget!");
        return  new T(_len, parent);
    }

    QByteArray valuePropertyName() const override
    {
        return {};
    }

private:
    uint32_t _len;
};

class MyDelegate : public QStyledItemDelegate
{
public:
    MyDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent)
    {
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override 
    {
        cds_warn("row {}, column {}", index.row(), index.column());

        QStyledItemDelegate::setEditorData(editor, index);
    }

    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        cds_error("createEditor!");
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

private:
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        cds_error("aaaaaaaaaaaaaaaa {}", event->type());

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};

struct CanSignalDataGuiImpl : public CanSignalDataGuiInt {
    CanSignalDataGuiImpl()
        : _ui(new Ui::CanSignalDataPrivate)
        , _widget(new QWidget)
    {
        _ui->setupUi(_widget);
        _widget->setMinimumSize(_ui->tv->minimumSize());

        QButtonGroup *bg = new QButtonGroup(_widget);
        bg->addButton(_ui->pbMsg);
        bg->addButton(_ui->pbSig);
    }

    virtual void setDockUndockCbk(const dockUndock_t& cb) override
    {
        QObject::connect(_ui->pbDockUndock, &QPushButton::toggled, cb);
    }

    virtual void setMsgViewCbk(const msgView_t& cb) override 
    {
        QObject::connect(_ui->pbMsg, &QPushButton::toggled, cb);
    }

    virtual void setMsgSettingsUpdatedCbk(const msgSettingsUpdated_t& cb) override 
    {
        _msgSettingsUpdatedCbk = cb;
    }

    virtual QWidget* mainWidget() override
    {
        return _widget;
    }

    template <typename F>
    void setDelegate(QTableView* tv, int col, MyDelegate& del, const std::function<void()>& cb)
    {
        QItemEditorFactory* factory = new QItemEditorFactory;
        QItemEditorCreatorBase* editor = new EditorCreator<F>(10);
        factory->registerEditor(QVariant::String, editor);
        del.setItemEditorFactory(factory);
        tv->setItemDelegateForColumn(col, &del);
        QObject::connect(&del, &QAbstractItemDelegate::closeEditor, cb);
    }

    virtual void setMsgView(QAbstractItemModel& tvModel) override
    {
        if(_settingsState.size() > 0) {
            _ui->tv->horizontalHeader()->restoreState(_settingsState);
        }
        
        _ui->tv->setModel(&tvModel);
        _ui->tv->horizontalHeader()->setSectionsMovable(true);
        _ui->tv->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        _ui->tv->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _ui->tv->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        _ui->tv->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
        _ui->tv->setColumnHidden(0, false);

        _settingsState = _ui->tv->horizontalHeader()->saveState();

        setDelegate<IntervalLE>(_ui->tv, 4, _cycleDelegate, std::bind(&CanSignalDataGuiImpl::msgSettingUpdated, this));
        setDelegate<InitValLE>(_ui->tv, 5, _initValDelegate, std::bind(&CanSignalDataGuiImpl::msgSettingUpdated, this));
    }

    void msgSettingUpdated()
    {
        if(_msgSettingsUpdatedCbk) {
            _msgSettingsUpdatedCbk();
        }
    }

    virtual void setSigView(QAbstractItemModel& tvModel) override
    {
        if(_tableState.size() > 0) {
            _ui->tv->horizontalHeader()->restoreState(_tableState);
        }

        _ui->tv->setModel(&tvModel);
        _ui->tv->horizontalHeader()->setSectionsMovable(true);
        _ui->tv->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        _ui->tv->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        _ui->tv->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _ui->tv->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        _ui->tv->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        _ui->tv->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        _ui->tv->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

        _tableState = _ui->tv->horizontalHeader()->saveState();
    }

    void initSearch(SearchModel& model) override
    {
        QObject::connect(_ui->searchLine, &QLineEdit::textChanged, &model, &SearchModel::updateFilter);
    }


private:
    Ui::CanSignalDataPrivate* _ui;
    QWidget* _widget;
    QByteArray _settingsState;
    QByteArray _tableState;
    MyDelegate _cycleDelegate;
    MyDelegate _initValDelegate;
    msgSettingsUpdated_t _msgSettingsUpdatedCbk{ nullptr };
};

#endif // CANSIGNALDATAGUIIMPL_H
