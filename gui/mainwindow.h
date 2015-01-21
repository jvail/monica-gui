#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class DebugStream;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  /* auto connect slots */
  void on_actionRun_triggered();
  void on_actionOpenProject_triggered();
  void on_actionSave_triggered();
  void on_actionSaveAs_triggered();

  void onTabCloseRequested(int index);
  void onTabCurrentChanged(int index);

private:
  void writeProjectFiles(const QString &path, const QString &name);

  Ui::MainWindow ui;
  DebugStream* coutStream;
  DebugStream* cerrStream;

};

#endif // MAINWINDOW_H
