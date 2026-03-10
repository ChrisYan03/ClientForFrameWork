#ifndef FACELISTMODEL_H
#define FACELISTMODEL_H

#include <QAbstractListModel>
#include <QVariantList>

/**
 * @brief 供 QML GridView/ListView 使用的人脸列表模型
 * 使用命名 role，确保 imageDataUrl 等能正确绑定与刷新
 */
class FaceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum FaceRoles {
        RoleId = Qt::UserRole + 1,
        RoleImageDataUrl,
        RoleImageFileUrl,
        RoleImageProviderUrl,
        RoleConfidence,
        RoleAge,
        RoleRectX,
        RoleRectY,
        RoleRectWidth,
        RoleRectHeight
    };

    explicit FaceListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /** 用新的列表替换当前数据并通知视图刷新 */
    void setList(const QVariantList& list);

    int count() const { return m_list.size(); }

signals:
    void countChanged();

private:
    QVariantList m_list;
};

#endif // FACELISTMODEL_H
