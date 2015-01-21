#ifndef CSONTREEMODEL_H
#define CSONTREEMODEL_H

#include <QAbstractItemModel>

class CsonTreeItem;
class cson_value;

class CsonTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  CsonTreeModel(QObject* parent, cson_value* rootVal, const QString& projectName);
  ~CsonTreeModel();

  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole);
  CsonTreeItem* item(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

private:
  void setup(cson_value* parentData, CsonTreeItem* parentItem, const cson_value* meta = NULL);
  CsonTreeItem* rootItem;
  cson_value* rootMeta;
};

#endif // CSONTREEMODEL_H
