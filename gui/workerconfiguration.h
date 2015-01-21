#ifndef WORKERCONFIGURATION_H
#define WORKERCONFIGURATION_H

#include "src/configuration.h"

#include <QObject>

class WorkerConfiguration : public QObject, public Monica::Configuration
{
  Q_OBJECT

public:
  WorkerConfiguration(const QString &outPath, const QString &dirNameMet, const QString &preMetFiles, const QString &dbIniName);

  void setProgress(double progress);

public slots:
  void doRun();

signals:
  void resultReady(const QString &result);
  void progressed(double progress);
};

#endif // WORKERCONFIGURATION_H
