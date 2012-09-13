#include "Project.h"
#include "FileManager.h"
#include "File.h"
#include "Misc.h"
#include "Ref.h"
#include "../libindexdb/IndexDb.h"

namespace Nav {

Project *theProject;

Project::Project(const QString &path) :
    m_fileManager(new FileManager)
{
    m_index = new indexdb::Index(path.toStdString());
}

Project::~Project()
{
    delete m_fileManager;
    delete m_index;
}

QList<Ref> Project::queryReferencesOfSymbol(const QString &symbol)
{
    QList<Ref> result;

    indexdb::ID symbolID =
        m_index->stringTable("usr")->id(symbol.toStdString().c_str());
    if (symbolID == indexdb::kInvalidID)
        return result;

    indexdb::Row rowLookup(1);
    rowLookup[0] = symbolID;

    indexdb::TableIterator itEnd = m_index->table("ref")->end();
    indexdb::TableIterator it = m_index->table("ref")->lowerBound(rowLookup);
    for (; it != itEnd; ++it) {
        indexdb::Row rowItem(5);
        it.value(rowItem);
        if (rowLookup[0] != rowItem[0])
            break;

        const char *fileName = m_index->stringTable("path")->item(rowItem[1]);
        int line = rowItem[2];
        int column = rowItem[3];
        const char *kind = m_index->stringTable("kind")->item(rowItem[4]);

        if (fileName[0] != '\0') {
            Ref ref;
            ref.symbol = symbol;
            ref.file = &fileManager()->file(QString(fileName));
            ref.line = line;
            ref.column = column;
            ref.kind = QString(kind);
            result << ref;
        }
    }

    return result;
}

QStringList Project::querySymbolsAtLocation(File *file, int line, int column)
{
    QStringList result;

    indexdb::ID fileID = m_index->stringTable("path")->id(file->path().toStdString().c_str());
    if (fileID == indexdb::kInvalidID)
        return result;

    indexdb::Row rowLookup(3);
    rowLookup[0] = fileID;
    rowLookup[1] = line;
    rowLookup[2] = column;

    indexdb::TableIterator itEnd = m_index->table("loc")->end();
    indexdb::TableIterator it = m_index->table("loc")->lowerBound(rowLookup);
    for (; it != itEnd; ++it) {
        indexdb::Row rowItem(4);
        it.value(rowItem);
        if (rowLookup[0] != rowItem[0] ||
                rowLookup[1] != rowItem[1] ||
                rowLookup[2] != rowItem[2])
            break;
        result << m_index->stringTable("usr")->item(rowItem[3]);
    }

    return result;
}

void Project::queryAllSymbols(std::vector<const char*> &output)
{
    indexdb::StringTable *symbolTable = m_index->stringTable("usr");
    output.resize(symbolTable->size());
    for (uint32_t i = 0, iEnd = symbolTable->size(); i < iEnd; ++i) {
        const char *symbol = symbolTable->item(i);
        output[i] = symbol;
    }
}

QList<File*> Project::queryAllFiles()
{
    QList<File*> result;
    indexdb::StringTable *pathTable = m_index->stringTable("path");
    for (uint32_t i = 0; i < pathTable->size(); ++i) {
        const char *path = pathTable->item(i);
        if (path[0] != '\0') {
            File *file = &m_fileManager->file(path);
            result << file;
        }
    }
    return result;
}

// Finds the only definition ref (or declaration ref) of the symbol.  If there
// isn't a single such ref, return NULL.
Ref Project::findSingleDefinitionOfSymbol(const QString &symbol)
{
    int declCount = 0;
    int defnCount = 0;
    Ref decl;
    Ref defn;
    QList<Ref> refs = theProject->queryReferencesOfSymbol(symbol);
    for (const Ref &ref : refs) {
        if (declCount < 2 && ref.kind == "Declaration") {
            declCount++;
            decl = ref;
        }
        if (defnCount < 2 && ref.kind == "Definition") {
            defnCount++;
            defn = ref;
        }
    }
    if (defnCount == 1) {
        return defn;
    } else if (defnCount == 0 && declCount == 1) {
        return decl;
    } else {
        Ref ret;
        ret.file = NULL;
        ret.line = 0;
        ret.column = 0;
        return ret;
    }
}

} // namespace Nav
