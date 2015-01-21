#include "workerconfiguration.h"
#include "src/monica.h"

#include <QDebug>

WorkerConfiguration::WorkerConfiguration(const QString &outPath, const QString &dirNameMet, const QString &preMetFiles, const QString &dbIniName) :
  Configuration(outPath.toStdString(), dirNameMet.toStdString(), preMetFiles.toStdString(), dbIniName.toStdString())
{

}

void WorkerConfiguration::setProgress(double progress)
{
  emit progressed(progress);
}

void WorkerConfiguration::doRun()
{
  Monica::Result res = Configuration::run();
  QString resString = "DM Yields:\n";

  for (std::vector<Monica::PVResult>::iterator it = res.pvrs.begin(); it != res.pvrs.end(); ++it) {
    resString += QString::number(it->pvResults.find(Monica::ResultId::cropname)->second);
    resString += ": ";
    resString += QString::number(it->pvResults.find(Monica::ResultId::primaryYieldTM)->second);
    resString += "\n";
  }

  emit resultReady(resString);
}
