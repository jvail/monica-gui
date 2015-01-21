#include "csontreemodel.h"
#include "csontreeitem.h"
#include "cson/cson_amalgamation_core.h"
#include "src/configuration.h"

#include <QDebug>

CsonTreeModel::CsonTreeModel(QObject* parent, cson_value* rootVal, const QString& projectName) :
  QAbstractItemModel(parent)
{
  rootItem = new CsonTreeItem(NULL, rootVal, projectName);
  rootMeta = cson_value_new_object();
  cson_object* rootMetaObj = cson_value_get_object(rootMeta);
  cson_object_set(rootMetaObj, "sim", cson_value_clone(Monica::Configuration::metaSim));
  cson_object_set(rootMetaObj, "site", cson_value_clone(Monica::Configuration::metaSite));
  cson_object_set(rootMetaObj, "crop", cson_value_clone(Monica::Configuration::metaCrop));
  setup(rootVal, rootItem, rootMeta);
}

CsonTreeModel::~CsonTreeModel()
{
  cson_value_free(rootMeta);
}

void CsonTreeModel::setup(cson_value* parentVal, CsonTreeItem* parentItem, const cson_value* meta)
{
  cson_object_iterator iter;
  int rc = cson_object_iter_init(cson_value_get_object(parentVal), &iter);
  if (rc != 0)
    return;
  cson_kvp* kvp;
  while ((kvp = cson_object_iter_next(&iter))) {
    cson_string const* key = cson_kvp_key(kvp);
    cson_value* val = cson_kvp_value(kvp);
    if (cson_value_is_object(val)) {
      parentItem->insertChildren(parentItem->childCount());
      CsonTreeItem* item = parentItem->child(parentItem->childCount() - 1);
      item->setData(0, val, cson_string_cstr(key));
      setup(val, item, cson_object_get(cson_value_get_object(meta), cson_string_cstr(key)));
    }
    else if (cson_value_is_array(val)) {
      parentItem->insertChildren(parentItem->childCount());
      CsonTreeItem* arrItem = parentItem->child(parentItem->childCount() - 1);
      arrItem->setData(0, val, cson_string_cstr(key));
      cson_array* arr = cson_value_get_array(val);
      cson_value* metaVal = cson_array_get(cson_value_get_array(cson_object_get(cson_value_get_object(meta), cson_string_cstr(key))), 0);
      unsigned int is = cson_array_length_get(arr);
      unsigned int i;
      for (i = 0; i < is; ++i) {
        cson_value* val = cson_array_get(arr, i);
        arrItem->insertChildren(arrItem->childCount());
        CsonTreeItem* item = arrItem->child(arrItem->childCount() - 1);
        item->setData(0, val, QString::number(i), metaVal);
        /* arrays always contain objects */
        setup(val, item, metaVal);
      }
    }
    else {
      parentItem->insertChildren(parentItem->childCount());
      CsonTreeItem* item = parentItem->child(parentItem->childCount() - 1);
      /* TODO: parent korrekt? */
      item->setData(0, val, cson_string_cstr(key), cson_object_get(cson_value_get_object(meta), cson_string_cstr(key)));
    }
  }
}

QVariant CsonTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole)
    return item(index)->data(index.column());

  return QVariant();
}

bool CsonTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole)
    return false;

  CsonTreeItem *item = this->item(index);
  cson_value *csonValOld = item->csonVal();
  if (cson_value_is_array(csonValOld) || cson_value_is_object(csonValOld))
    return false;

  CsonTreeItem *parentItem = item->parent();
  cson_value* csonValNew = NULL;
  bool ok = false;

  switch (cson_value_type_id(csonValOld)) {
  case CSON_TYPE_BOOL:
    if (value.type() == QMetaType::Bool)
      csonValNew = cson_value_new_bool((value.toBool() ? '1' : NULL));
    break;
  case CSON_TYPE_INTEGER:
    if (value.type() == QMetaType::Int)
      csonValNew = cson_value_new_integer(value.toInt());
    break;
  case CSON_TYPE_DOUBLE:
    if (value.type() == QMetaType::Double)
      csonValNew = cson_value_new_double(value.toDouble());
    break;
  case CSON_TYPE_STRING:
    if (value.type() == QMetaType::QString)
      csonValNew = cson_value_new_string(value.toString().toStdString().c_str(),
                                         strlen(value.toString().toStdString().c_str()));
    break;
  case CSON_TYPE_ARRAY:
  case CSON_TYPE_OBJECT:
  case CSON_TYPE_NULL:
  case CSON_TYPE_UNDEF:
    break;
  default:
    break;
  }

  if (csonValNew) {
    int rc = cson_object_set(cson_value_get_object(parentItem->csonVal()),
              item->key().toStdString().c_str(), csonValNew);
    if (rc != 0) {
      ok = false;
      cson_value_free(csonValNew);
    }
    else {
      ok = true;
      item->setCsonValue(csonValNew);
      Monica::Configuration::printJSON(parentItem->csonVal());
      emit QAbstractItemModel::dataChanged(index.sibling(index.row(), 0), index.sibling(index.row(), 2));
    }
  }

  return ok;
}

CsonTreeItem *CsonTreeModel::item(const QModelIndex &index) const
{
  if (index.isValid()) {
    CsonTreeItem *item = static_cast<CsonTreeItem*>(index.internalPointer());
    if (item)
      return item;
  }
  return rootItem;
}

QVariant CsonTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant header = QVariant();

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {

    switch (section) {
    case 0:
      header = "Name";
      break;
    case 1:
      header = "Value";
      break;
    case 2:
      header = "Unit";
      break;
    case 3:
      header = "Description";
      break;
    default:
      header = QVariant();
      break;
    }
  }

  return header;
}

QModelIndex CsonTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid() && parent.column() != 0)
      return QModelIndex();

  CsonTreeItem *parentItem = item(parent);

  CsonTreeItem *childItem = parentItem->child(row);
  if (childItem)
      return createIndex(row, column, childItem);
  else
      return QModelIndex();
}

QModelIndex CsonTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
      return QModelIndex();

  CsonTreeItem *childItem = item(index);
  CsonTreeItem *parentItem = childItem->parent();

  if (parentItem == rootItem)
      return QModelIndex();

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

int CsonTreeModel::rowCount(const QModelIndex &parent) const
{
  CsonTreeItem *parentItem = item(parent);
  return parentItem->childCount();
}

int CsonTreeModel::columnCount(const QModelIndex &parent) const
{
  return 4;
}

bool CsonTreeModel::hasChildren(const QModelIndex &parent) const
{
  CsonTreeItem *item = this->item(parent);
  return (item->childCount() != 0);
}

Qt::ItemFlags CsonTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
     return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
