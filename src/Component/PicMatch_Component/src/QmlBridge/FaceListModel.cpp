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
    case RoleImageProviderUrl:
        return map.value(QStringLiteral("imageProviderUrl"));
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
    names[RoleImageProviderUrl] = "imageProviderUrl";
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
    // 检查是否真的需要更新，避免不必要的重置
    if (m_list.size() == list.size()) {
        bool same = true;
        for (int i = 0; i < list.size() && same; ++i) {
            same = (m_list.at(i) == list.at(i));
        }
        if (same) return;  // 数据相同，不触发更新
    }

    beginResetModel();
    m_list = list;
    endResetModel();
    emit countChanged();
}
