#include "mainwindow.h"
#include "tabconfiguration.h"
#include "debugstream.h"

#include "src/configuration.h"
#include "cson/cson_amalgamation_core.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
  ui.setupUi(this);
  setWindowTitle("MONICA-GUI");
  ui.textEditOut->setVisible(false);

  /* crash mit Monica::activateDebug = true */
  coutStream = new DebugStream(std::cout, ui.textEditOut, Qt::darkGray);
  cerrStream = new DebugStream(std::cerr, ui.textEditOut, Qt::red);

  connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
  connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabCurrentChanged(int)));
}

MainWindow::~MainWindow()
{
  delete coutStream;
  delete cerrStream;
}

void MainWindow::on_actionRun_triggered()
{
  TabConfiguration* tab = qobject_cast<TabConfiguration*>(ui.tabWidget->currentWidget());
  if (tab)
    tab->run();
}

void MainWindow::on_actionOpenProject_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open Project"), "./in/json", tr("Simulation Files (*.sim.json)"));

  QStringList strList = fileName.split('/');
  QString projectName = strList.takeLast().split('.').first();
  QString projectPath = strList.join('/');
  if (projectName.isEmpty())
    return;

  /* read json files*/
  QString simStr;
  QFile simFile(projectPath + "/" + projectName + ".sim.json");
  if (!simFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cerr << "Could not open sim.json." << std::endl;
    return;
  }
  QTextStream simIn(&simFile);
  simStr = simIn.readAll();

  QString siteStr;
  QFile siteFile(projectPath + "/" + projectName + ".site.json");
  if (!siteFile.open(QIODevice::ReadOnly | QIODevice::Text)){
    std::cerr << "Could not open site.json." << std::endl;
    return;
  }
  QTextStream siteIn(&siteFile);
  siteStr = siteIn.readAll();

  QString cropStr;
  QFile cropFile(projectPath + "/" + projectName + ".crop.json");
  if (!cropFile.open(QIODevice::ReadOnly | QIODevice::Text)){
    std::cerr << "Could not open crop.json." << std::endl;
    return;
  }
  QTextStream cropIn(&cropFile);
  cropStr += cropIn.readAll();

  int rc = 0;
  bool ok = false;
  cson_value* simVal = NULL;

  rc = Monica::Configuration::readJSON(simStr.toStdString(), &simVal);
  ok = Monica::Configuration::isValid(simVal, Monica::Configuration::metaSim, "sim");

  if (rc != 0 || !ok) {
    cson_free_value(simVal);
    std::cerr << "Error reading sim.json" << std::endl;
    return;
  }

  cson_value* siteVal = NULL;
  rc = Monica::Configuration::readJSON(siteStr.toStdString(), &siteVal);
  ok = Monica::Configuration::isValid(siteVal, Monica::Configuration::metaSite, "site");

  if (rc != 0 || !ok) {
    cson_free_value(simVal);
    cson_free_value(siteVal);
    std::cerr << "Error reading site.json" << std::endl;
    return;
  }

  cson_value* cropVal = NULL;
  rc = Monica::Configuration::readJSON(cropStr.toStdString(), &cropVal);
  ok = Monica::Configuration::isValid(cropVal, Monica::Configuration::metaCrop, "crop");

  if (rc != 0 || !ok) {
    cson_free_value(simVal);
    cson_free_value(siteVal);
    cson_free_value(cropVal);
    std::cerr << "Error reading crop.json" << std::endl;
    return;
  }

  /* readable and valid */
  cson_value* rootVal = cson_value_new_object();
  cson_object* rootObj = cson_value_get_object(rootVal);
  cson_object_set(rootObj, "sim", simVal);
  cson_object_set(rootObj, "site", siteVal);
  cson_object_set(rootObj, "crop", cropVal);
  TabConfiguration* tab = new TabConfiguration(ui.tabWidget, rootVal, projectName, projectPath);
  ui.tabWidget->addTab(tab, projectName);
  ui.tabWidget->setCurrentIndex(ui.tabWidget->count() - 1);
}

void MainWindow::on_actionSave_triggered()
{
  int index = ui.tabWidget->currentIndex();
  if (index < 0)
    return;

  TabConfiguration* tab = qobject_cast<TabConfiguration*>(ui.tabWidget->currentWidget());
  if (!tab)
    return;

  QString path = tab->projectPath();
  QString name = tab->projectName();
  writeProjectFiles(path, name);
}

void MainWindow::on_actionSaveAs_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Project"), "./in/json", tr("(*)"));

  QStringList strList = fileName.split('/');
  QString name = strList.takeLast().split('.').first();
  QString path = strList.join('/');
  if (name.isEmpty())
    return;
  writeProjectFiles(path, name);
}

void MainWindow::onTabCloseRequested(int index)
{
  ui.tabWidget->removeTab(index);
}

void MainWindow::onTabCurrentChanged(int index)
{
  ui.actionSave->setText(tr("Save \"%1\"").arg(ui.tabWidget->tabText(index)));
  ui.actionSaveAs->setText(tr("Save \"%1\" As...").arg(ui.tabWidget->tabText(index)));
}

void MainWindow::writeProjectFiles(const QString &path, const QString &name)
{
  if (path.isEmpty() || name.isEmpty())
    return;

  TabConfiguration* tab = qobject_cast<TabConfiguration*>(ui.tabWidget->currentWidget());
  if (!tab)
    return;

  FILE* simFile = fopen(QString("%1/%2.%3.json").arg(path, name, "sim").toStdString().c_str(), "w");
  if (!simFile) {
    std::cerr << "Could not open file " << QString("%1/%2.%3.json").arg(path, name, "sim").toStdString() << std::endl;
    return;
  }
  else {
    int rc = Monica::Configuration::writeJSON(simFile, cson_object_get(cson_value_get_object(tab->root()), "sim"));
    if (rc != 0)
      std::cerr << "Could not write JSON file " << QString("%1/%2.%3.json").arg(path, name, "sim").toStdString() <<
                   cson_rc_string(rc) << std::endl;
    fclose(simFile);
  }

  FILE* siteFile = fopen(QString("%1/%2.%3.json").arg(path, name, "site").toStdString().c_str(), "w");
  if (!siteFile) {
    std::cerr << "Could not open file " << QString("%1/%2.%3.json").arg(path, name, "site").toStdString() << std::endl;
    return;
  }
  else {
    int rc = Monica::Configuration::writeJSON(siteFile, cson_object_get(cson_value_get_object(tab->root()), "site"));
    if (rc != 0)
      std::cerr << "Could not write JSON file " << QString("%1/%2.%3.json").arg(path, name, "site").toStdString() <<
                   cson_rc_string(rc) << std::endl;
    fclose(siteFile);
  }

  FILE* cropFile = fopen(QString("%1/%2.%3.json").arg(path, name, "crop").toStdString().c_str(), "w");
  if (!cropFile) {
    std::cerr << "Could not open file " << QString("%1/%2.%3.json").arg(path, name, "crop").toStdString() << std::endl;
    return;
  }
  else {
    int rc = Monica::Configuration::writeJSON(cropFile, cson_object_get(cson_value_get_object(tab->root()), "crop"));
    if (rc != 0)
      std::cerr << "Could not write JSON file " << QString("%1/%2.%3.json").arg(path, name, "crop").toStdString() <<
                   cson_rc_string(rc) << std::endl;
    fclose(cropFile);
  }
}
