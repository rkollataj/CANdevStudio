#ifndef __CDSDATABASE_H
#define __CDSDATABASE_H

#include <nodes/NodeDataModel>

using QtNodes::NodeDataType;

class CdsDataBase : public QtNodes::NodeData {
public:
    virtual QVariantList toVariant() = 0;
    virtual void fromVariant(const QVariantList& list) = 0;
};

#endif /* !__CDSDATABASE_H */
