#include "csontreeitem.h"
#include "cson/cson_amalgamation_core.h"


CsonTreeItem::CsonTreeItem(CsonTreeItem *parent, cson_value* val, const QString& key)
{
  parentItem = parent;
  dataVal = val;
  dataKey = key;
}

CsonTreeItem::~CsonTreeItem()
{
    qDeleteAll(childItems);
}

CsonTreeItem* CsonTreeItem::child(int number)
{
  return childItems.value(number);
}

CsonTreeItem* CsonTreeItem::parent()
{
  return parentItem;
}

int CsonTreeItem::childCount() const
{
  return childItems.count();
}

int CsonTreeItem::columnCount() const
{
  return 4;
}

int CsonTreeItem::childNumber() const
{
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<CsonTreeItem*>(this));

  return 0;
}

QVariant CsonTreeItem::data(int column) const
{
  if (column == 0)
    return dataKey;
  else if (column == 1) {
    switch (cson_value_type_id(dataVal))
    {
    case CSON_TYPE_BOOL:
      return bool(cson_value_get_bool(dataVal));
      break;
    case CSON_TYPE_INTEGER:
      return int(cson_value_get_integer(dataVal));
      break;
    case CSON_TYPE_DOUBLE:
      return cson_value_get_double(dataVal);
      break;
    case CSON_TYPE_STRING:
      return cson_value_get_cstr(dataVal);
      break;
    case CSON_TYPE_ARRAY:
    case CSON_TYPE_OBJECT:
      return QVariant();
      break;
    case CSON_TYPE_NULL:
    case CSON_TYPE_UNDEF:
    default:
      return QVariant();
      break;
    }
  }
  else if (column == 2 && dataMeta) {
    return cson_string_cstr(cson_value_get_string(cson_object_get(cson_value_get_object(dataMeta), "unit")));
  }
  else if (column == 3 && dataMeta) {
    return cson_string_cstr(cson_value_get_string(cson_object_get(cson_value_get_object(dataMeta), "desc")));
  }

  return QVariant();
}
bool CsonTreeItem::setData(int column, cson_value* val, const QString& key, const cson_value* meta)
{
  if (column < 0 || column > 3)
      return false;

  dataVal = val;
  dataKey = key;
  dataMeta = meta;
  return true;
}

bool CsonTreeItem::insertChildren(int position, int count, int columns)
{
  if (position < 0 || position > childItems.size())
      return false;

  for (int row = 0; row < count; ++row) {
      cson_value* val = NULL;
      CsonTreeItem *item = new CsonTreeItem(this);
      childItems.insert(position, item);
  }

  return true;
}

bool CsonTreeItem::insertColumns(int position, int columns)
{
  return false;
}

bool CsonTreeItem::removeChildren(int position, int count)
{
  return false;
}

bool CsonTreeItem::removeColumns(int position, int columns)
{
  return false;
}
