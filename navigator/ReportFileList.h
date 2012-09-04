#ifndef NAV_REPORTFILELIST_H
#define NAV_REPORTFILELIST_H

#include "TreeReport.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>

namespace Nav {

class Project;
class File;

class ReportFileList : public TableReport
{
public:
    ReportFileList(Project *project);

    QString getTitle();
    QStringList getColumns();
    int getRowCount();
    QList<QVariant> getText(const Index &index);
    void select(const Index &index);

private:
    Project *m_project;
    QList<File*> m_files;
};

} // namespace Nav

#endif // NAV_REPORTFILELIST_H