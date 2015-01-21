#include "tabconfiguration.h"
#include "workerconfiguration.h"
#include "csontreemodel.h"
#include "csontreeitem.h"
#include "cson/cson_amalgamation_core.h"
#include "db/abstract-db-connections.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QProgressDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QDebug>

TabConfiguration::TabConfiguration(QWidget *parent, cson_value* root, const QString& projectName, const QString& projectPath) :
  QWidget(parent), _projectName(projectName), _projectPath(projectPath)
{
  _root = root;
  QVBoxLayout* layout = new QVBoxLayout;
  treeView = new QTreeView(this);
  treeView->setAlternatingRowColors(true);
  treeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  treeView->setExpandsOnDoubleClick(false);
  connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTreeViewDoubleClicked(QModelIndex)));
  treeModel = new CsonTreeModel(this, _root, _projectName);
  treeView->setModel(treeModel);
  treeView->expandAll();
  layout->addWidget(treeView);
  setLayout(layout);

  pDlg = new QProgressDialog(this);
  pDlg->setWindowModality(Qt::WindowModal);
  pDlg->setMaximum(100);
  pDlg->setAutoClose(false);
  pDlg->setCancelButton(NULL);

  worker = new WorkerConfiguration(QString("out"), QString("in/met"), QString("MET_HF."), QString("db.ini"));
  worker->moveToThread(&workerThread);
  connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
  connect(this, &TabConfiguration::operate, worker, &WorkerConfiguration::doRun);
  connect(worker, &WorkerConfiguration::resultReady, this, &TabConfiguration::handleResults);
  connect(worker, &WorkerConfiguration::progressed, this, &TabConfiguration::onProgressed);
  workerThread.start();
}

TabConfiguration::~TabConfiguration()
{
  workerThread.quit();
  workerThread.wait();
  if (_root)
    cson_value_free(_root);
}

void TabConfiguration::run()
{
  bool ok = true;
  cson_value* simVal = NULL;
  cson_value* siteVal = NULL;
  cson_value* cropVal = NULL;

  if (_root) {
    simVal = cson_value_clone(cson_object_get(cson_value_get_object(_root), "sim"));
    siteVal = cson_value_clone(cson_object_get(cson_value_get_object(_root), "site"));
    cropVal = cson_value_clone(cson_object_get(cson_value_get_object(_root), "crop"));
  }

  if (!simVal || !siteVal || !cropVal) {
    if (simVal)
      cson_value_free(simVal);
    if (siteVal)
      cson_value_free(siteVal);
    if (cropVal)
      cson_value_free(cropVal);
    std::cerr << "Could not create objects from JSON root object" << std::endl;
    return;
  }

  /* move the copy (and ownership) to config in thread */
  ok = worker->setJSON(simVal, siteVal, cropVal);
  if (ok && workerIsReady) {
    pDlg->reset();
    pDlg->show();
    workerIsReady = false;
    emit operate();
  }
}

void TabConfiguration::handleResults(const QString &result)
{
  pDlg->close();
  workerIsReady = true;
  std::cout << result.toStdString() << std::endl;
}

void TabConfiguration::onProgressed(double progress)
{
  pDlg->setValue(int(progress * 100));
}

void TabConfiguration::onTreeViewDoubleClicked(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  QString name = treeModel->data(index.sibling(index.row(), 0), Qt::DisplayRole).toString();
  QVariant data = treeModel->data(index.sibling(index.row(), 1), Qt::DisplayRole);
  QString unit = treeModel->data(index.sibling(index.row(), 2), Qt::DisplayRole).toString();
  QString labelText = name = name + " [" + unit + "]:";
  QVariant newData;

  switch (data.type()) {
  case QMetaType::Bool:
    newData = QInputDialog::getItem(this, tr("Set Data"), labelText, QStringList() << "true" << "false", (data.toBool()) ? 0 : 1, false);
    newData.convert(QVariant::Bool);
    break;
  case QMetaType::Int:
    newData = QInputDialog::getInt(this, tr("Set Data"), labelText, data.toInt());
    newData.convert(QVariant::Int);
    break;
  case QMetaType::Double:
    newData = QInputDialog::getDouble(this, tr("Set Data"), labelText, data.toDouble(), -1000000, 1000000, 4);
    newData.convert(QVariant::Double);
    break;
  case QMetaType::QString:
  {
    const cson_object *metaObj = cson_value_get_object(treeModel->item(index)->csonMetaVal());
    QStringList strList = QStringList();
    cson_value *enumVal = cson_object_get(metaObj, "enum");
    cson_value *dbVal = cson_object_get(metaObj, "db");
    if (enumVal && cson_value_is_array(enumVal)) {
      cson_array *enumArr = cson_value_get_array(enumVal);
      for (unsigned int i = 0; i < cson_array_length_get(enumArr); i++)
        strList << cson_string_cstr(cson_value_get_string(cson_array_get(enumArr, i)));
    }
    else if (dbVal && cson_value_is_object(dbVal)) {
      std::string table = Monica::Configuration::getStr(metaObj, "db.table");
      std::string column = Monica::Configuration::getStr(metaObj, "db.column");
      Db::DB *con = Db::newConnection("monica");
      if (con) {
        bool ok = con->select("SELECT " + column + " FROM " + table);
        Db::DBRow row;
        if (ok) {
          while (!(row = con->getRow()).empty())
            strList << QString::fromStdString(row[0]);
        }
        delete con;
      }
    }
    strList.sort();
    if (strList.isEmpty())
      newData = QInputDialog::getText(this, tr("Set Data"), labelText, QLineEdit::Normal, data.toString());
    else
      newData = QInputDialog::getItem(this, tr("Set Data"), labelText, strList, strList.indexOf(QRegExp(data.toString())), false);
    break;
  }
  case 0: /* unknown, invalid */
  {
    const cson_value *val = treeModel->item(index)->csonVal();
    const cson_value *metaVal = treeModel->item(index)->csonMetaVal();
    if (cson_value_is_array(val)) {

      Monica::Configuration::printJSON(metaVal);
      Monica::Configuration::printJSON(val);
      qDebug() << "is array";


    }
    else if (cson_value_is_object(val)) {
      qDebug() << "is object";

    }
    break;
  }
  default:
    break;
  }

  if (!newData.isNull() && newData != data)
    treeModel->setData(index, newData, Qt::EditRole);
}
