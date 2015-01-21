#ifndef TABCONFIGURATION_H
#define TABCONFIGURATION_H

#include <QWidget>
#include <QThread>

class QProgressDialog;
class WorkerConfiguration;
class cson_value;
class QTreeView;
class CsonTreeModel;

class TabConfiguration : public QWidget
{
  Q_OBJECT
public:
  explicit TabConfiguration(QWidget *parent, cson_value* root, const QString& projectName, const QString& projectPath);
  ~TabConfiguration();
  QThread workerThread;
  void run();
  const cson_value* root() { return _root; }
  QString projectName() { return _projectName; }
  QString projectPath() { return _projectPath; }

signals:
  void operate();

public slots:
  void handleResults(const QString &result);
  void onProgressed(double progress);
  void onTreeViewDoubleClicked(const QModelIndex &index);

private:
  QProgressDialog* pDlg;
  WorkerConfiguration *worker;
  QTreeView* treeView;
  CsonTreeModel *treeModel;

  cson_value* _root = NULL;
  QString _projectName;
  QString _projectPath;
  /* TODO: move to worker */
  bool workerIsReady = true;
};

#endif // TABCONFIGURATION_H
