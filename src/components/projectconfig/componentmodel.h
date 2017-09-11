#ifndef COMPONENTMODEL_H
#define COMPONENTMODEL_H

#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <nodes/NodeDataModel>

template <typename C, typename Derived> class ComponentModel : public QtNodes::NodeDataModel {

public:
    ComponentModel() = default;
    virtual ~ComponentModel() = default;

    /**
    *   @brief  Used to get node caption
    *   @return Node caption
    */
    virtual QString caption() const override
    {
        return _caption;
    }

    virtual void setCaption(const QString& caption)
    {
        _caption = caption;
    }

    /**
    *   @brief  Used to identify model by data model name
    *   @return Node model name
    */
    virtual QString name() const override
    {
        return _name;
    }

    virtual void setName(const QString& name)
    {
        _name = name;
    }

    /**
    *   @brief Creates new node of the same type
    *   @return cloned node
    */
    virtual std::unique_ptr<NodeDataModel> clone() const override
    {
        return std::make_unique<Derived>();
    }

    /**
     * @brief Possibility to save node properties
     * @return json object
     */
    virtual QJsonObject save() const override
    {
        QJsonObject json;
        json["name"] = _component.getConfig();
        return json;
    }

    /**
    *   @brief  Used to get model name
    *   @return Model name
    */
    virtual QString modelName() const
    {
        return _modelName;
    }

    virtual void setModelName(QString modelName)
    {
        _modelName = modelName;
    }

    /**
    *   @brief  Used to get widget embedded in Node
    *   @return QLabel
    */
    QWidget* embeddedWidget() override
    {
        return _label;
    }

    /**
    *   @brief  Used to get information if node is resizable
    *   @return false
    */
    bool resizable() const override
    {
        return _resizable;
    }

    void setResizable(bool resizable)
    {
        _resizable = resizable;
    }

    /**
    *   @brief Component getter
    *   @return Component managed by model
    */
    C& getComponent()
    {
        return _component;
    }

protected:
    C _component;
    QLabel* _label{ new QLabel };
    QString _caption;
    QString _name;
    QString _modelName;
    bool _resizable{ false };
};

#endif // COMPONENTMODEL_H
