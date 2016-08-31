#ifndef CVIRTUALDIRECTORYMODEL
#define CVIRTUALDIRECTORYMODEL

#include "Editor/UICommon.h"
#include <Core/GameProject/CVirtualDirectory.h>
#include <QAbstractItemModel>
#include <QIcon>

class CVirtualDirectoryModel : public QAbstractItemModel
{
    Q_OBJECT
    CVirtualDirectory *mpRoot;

public:
    CVirtualDirectoryModel(QObject *pParent = 0)
        : QAbstractItemModel(pParent)
        , mpRoot(nullptr)
    {}

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const
    {
        if (!hasIndex(Row, Column, rkParent))
            return QModelIndex();

        CVirtualDirectory *pDir = IndexDirectory(rkParent);

        if (pDir && pDir->NumSubdirectories() > (u32) Row)
            return createIndex(Row, Column, pDir->SubdirectoryByIndex(Row));

        else if (!pDir)
            return createIndex(Row, Column, mpRoot);

        return QModelIndex();
    }

    QModelIndex parent(const QModelIndex& rkChild) const
    {
        CVirtualDirectory *pDir = IndexDirectory(rkChild);
        CVirtualDirectory *pParent = pDir->Parent();

        if (pParent)
        {
            CVirtualDirectory *pGrandparent = pParent->Parent();

            if (pGrandparent)
            {
                for (u32 iSub = 0; iSub < pGrandparent->NumSubdirectories(); iSub++)
                {
                    if (pGrandparent->SubdirectoryByIndex(iSub) == pParent)
                        return createIndex(iSub, 0, pParent);
                }
            }

            else return createIndex(0, 0, mpRoot);
        }

        return QModelIndex();
    }

    int rowCount(const QModelIndex& rkParent) const
    {
        CVirtualDirectory *pDir = IndexDirectory(rkParent);
        if (pDir) return pDir->NumSubdirectories();
        else return mpRoot ? 1 : 0;
    }

    int columnCount(const QModelIndex& /*rkParent*/) const
    {
        return 1;
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
        {
            CVirtualDirectory *pDir = IndexDirectory(rkIndex);
            if (pDir) return TO_QSTRING(pDir->Name());
        }

        if (Role == Qt::DecorationRole)
        {
            return QIcon(":/icons/Open_24px.png");
        }

        return QVariant::Invalid;
    }

    inline CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const
    {
        if (!rkIndex.isValid()) return nullptr;
        return static_cast<CVirtualDirectory*>(rkIndex.internalPointer());
    }

    inline void SetRoot(CVirtualDirectory *pDir)
    {
        beginResetModel();
        mpRoot = pDir;
        endResetModel();
    }
};

#endif // CVIRTUALDIRECTORYMODEL
