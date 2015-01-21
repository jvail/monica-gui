#ifndef CSONITEM_H
#define CSONITEM_H

#include <QString>
#include <QVariant>

struct MyStruct
{

};

Q_DECLARE_METATYPE(MyStruct)

class cson_value;

class CsonTreeItem
{
public:

  CsonTreeItem(CsonTreeItem *parent = 0, cson_value* val = NULL, const QString& key = QString());
  ~CsonTreeItem();

  CsonTreeItem* child(int number);
  CsonTreeItem* parent();
  int childCount() const;
  int columnCount() const;
  int childNumber() const;
  QVariant data(int column) const;
  bool setData(int column, cson_value* val, const QString& key, const cson_value* meta = NULL);
  bool insertChildren(int position, int count = 1, int columns = 1);
  bool insertColumns(int position, int columns);
  bool removeChildren(int position, int count = 1);
  bool removeColumns(int position, int columns = 1);
  void setCsonValue(cson_value* val) { dataVal = val; }
  cson_value* csonVal() { return dataVal; }
  const cson_value* csonMetaVal() { return dataMeta; }
  const QString key() { return dataKey; }

private:
  QList<CsonTreeItem*> childItems;
  CsonTreeItem* parentItem = NULL;
  cson_value* dataVal = NULL;
  const cson_value* dataMeta = NULL;
  QString dataKey = "";
};

#endif // CSONITEM_H
