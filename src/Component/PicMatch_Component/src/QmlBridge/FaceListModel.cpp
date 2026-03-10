#include "FaceListModel.h"

FaceListModel::FaceListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int FaceListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_list.size();
}

QVariant FaceListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_list.size())
        return QVariant();
    const QVariantMap& map = m_list.at(index.row()).toMap();
    switch (role) {
    case RoleId:
        return map.value(QStringLiteral("id"));
    case RoleImageDataUrl:
        return map.value(QStringLiteral("imageDataUrl"));
    case RoleImageFileUrl:
        return map.value(QStringLiteral("imageFileUrl"));
    case RoleConfidence:
        return map.contains(QStringLiteral("attributes")) ? map.value(QStringLiteral("attributes")).toMap().value(QStringLiteral("confidence")) : QVariant();
    case RoleAge:
        return map.contains(QStringLiteral("attributes")) ? map.value(QStringLiteral("attributes")).toMap().value(QStringLiteral("age")) : QVariant();
    case RoleRectX:
        return map.value(QStringLiteral("rectX"));
    case RoleRectY:
        return map.value(QStringLiteral("rectY"));
    case RoleRectWidth:
        return map.value(QStringLiteral("rectWidth"));
    case RoleRectHeight:
        return map.value(QStringLiteral("rectHeight"));
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FaceListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[RoleId] = "faceId";
    names[RoleImageDataUrl] = "imageDataUrl";
    names[RoleImageFileUrl] = "imageFileUrl";
    names[RoleConfidence] = "confidence";
    names[RoleAge] = "age";
    names[RoleRectX] = "rectX";
    names[RoleRectY] = "rectY";
    names[RoleRectWidth] = "rectWidth";
    names[RoleRectHeight] = "rectHeight";
    return names;
}

void FaceListModel::setList(const QVariantList& list)
{
    beginResetModel();
    m_list = list;
    endResetModel();
    emit countChanged();
}
