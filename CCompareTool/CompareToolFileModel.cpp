#include "StdAfx.h"

#include "CompareToolFileModel_p.h"
#include "CompareToolFileModel.h"

#include <qlocale.h>
#include <qmime.h>
#include <qurl.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qapplication.h>


#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif
#ifdef Q_OS_WIN32
#include <QtCore/QVarLengthArray>
#endif



bool CompareToolFileModel::remove(const QModelIndex &aindex) const
{
    //### TODO optim
    QString path = filePath(aindex);
    CompareToolFileModelPrivate * d = const_cast<CompareToolFileModelPrivate*>(d_func());
    d->fileInfoGatherer.removePath(path);
    QDirIterator it(path,
            QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
    QStringList children;
    while (it.hasNext())
        children.prepend(it.next());
    children.append(path);

    bool error = false;
    for (int i = 0; i < children.count(); ++i) {
        QFileInfo info(children.at(i));
        QModelIndex modelIndex = index(children.at(i));
        if (info.isDir()) {
            QDir dir;
            if (children.at(i) != path)
                error |= remove(modelIndex);
            error |= rmdir(modelIndex);
        } else {
            error |= QFile::remove(filePath(modelIndex));
        }
    }
    return error;
}

//void CompareToolFileModel::remove( const QModelIndexList & indexList ) const
//{
//	bool bResult = true;
//	Q_FOREACH(const QModelIndex & index, indexList)
//	{
//		if (index.column() != g_nFileName)
//		{
//			continue;
//		}
//		if ( !remove(index) )
//		{
//			bResult = false;
//		}
//	}
//	if (!bResult)
//	{
//		QMessageBox::warning(NULL, "����", "�ļ�ɾ��ʧ�ܣ�");
//	}
//	else
//	{
//		QMessageBox::about(NULL, "��ʾ", "�ļ�ɾ����ɣ�");
//	}
//	//emit removeResult(bResult);
//}

/*!
  Constructs a file system model with the given \a parent.
*/
CompareToolFileModel::CompareToolFileModel(QObject *parent)
    : QAbstractItemModel(*new CompareToolFileModelPrivate, parent)
{
    Q_D(CompareToolFileModel);
    d->init();
}

/*!
    \internal
*/
CompareToolFileModel::CompareToolFileModel(CompareToolFileModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(CompareToolFileModel);
    d->init();
}

/*!
  Destroys this file system model.
*/
CompareToolFileModel::~CompareToolFileModel()
{
}

/*!
    \reimp
*/
QModelIndex CompareToolFileModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const CompareToolFileModel);
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = (d->indexValid(parent) ? d->node(parent) :
                                                   const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&d->root));
    Q_ASSERT(parentNode);

    // now get the internal pointer for the index
    QString childName = parentNode->visibleChildren[d->translateVisibleLocation(parentNode, row)];
    const CompareToolFileModelPrivate::QFileSystemNode *indexNode = parentNode->children.value(childName);
    Q_ASSERT(indexNode);

    return createIndex(row, column, const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(indexNode));
}

/*!
    \overload

    Returns the model item index for the given \a path and \a column.
*/
QModelIndex CompareToolFileModel::index(const QString &path, int column) const
{
    Q_D(const CompareToolFileModel);
    CompareToolFileModelPrivate::QFileSystemNode *node = d->node(path, false);
    QModelIndex idx = d->index(node);
    if (idx.column() != column)
        idx = idx.sibling(idx.row(), column);
    return idx;
}

/*!
    \internal

    Return the QFileSystemNode that goes to index.
  */
CompareToolFileModelPrivate::QFileSystemNode *CompareToolFileModelPrivate::node(const QModelIndex &index) const
{
    if (!index.isValid())
        return const_cast<QFileSystemNode*>(&root);
    CompareToolFileModelPrivate::QFileSystemNode *indexNode = static_cast<CompareToolFileModelPrivate::QFileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);
    return indexNode;
}

#ifdef Q_OS_WIN32
static QString qt_GetLongPathName(const QString &strShortPath)
{
    if (strShortPath.isEmpty()
        || strShortPath == QLatin1String(".") || strShortPath == QLatin1String(".."))
        return strShortPath;
    if (strShortPath.length() == 2 && strShortPath.endsWith(QLatin1Char(':')))
        return strShortPath.toUpper();
    const QString absPath = QDir(strShortPath).absolutePath();
    if (absPath.startsWith(QLatin1String("//"))
        || absPath.startsWith(QLatin1String("\\\\"))) // unc
        return QDir::fromNativeSeparators(absPath);
    if (absPath.startsWith(QLatin1Char('/')))
        return QString();
    const QString inputString = QLatin1String("\\\\?\\") + QDir::toNativeSeparators(absPath);
    QVarLengthArray<TCHAR, MAX_PATH> buffer(MAX_PATH);
    DWORD result = ::GetLongPathName((wchar_t*)inputString.utf16(),
                                     buffer.data(),
                                     buffer.size());
    if (result > DWORD(buffer.size())) {
        buffer.resize(result);
        result = ::GetLongPathName((wchar_t*)inputString.utf16(),
                                   buffer.data(),
                                   buffer.size());
    }
    if (result > 4) {
        QString longPath = QString::fromWCharArray(buffer.data() + 4); // ignoring prefix
        longPath[0] = longPath.at(0).toUpper(); // capital drive letters
        return QDir::fromNativeSeparators(longPath);
    } else {
        return QDir::fromNativeSeparators(strShortPath);
    }
}
#endif

/*!
    \internal

    Given a path return the matching QFileSystemNode or &root if invalid
*/
CompareToolFileModelPrivate::QFileSystemNode *CompareToolFileModelPrivate::node(const QString &path, bool fetch) const
{
    Q_Q(const CompareToolFileModel);
    Q_UNUSED(q);
    if (path.isEmpty() || path == myComputer() || path.startsWith(QLatin1Char(':')))
        return const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&root);

    // Construct the nodes up to the new root path if they need to be built
    QString absolutePath;
#ifdef Q_OS_WIN32
    QString longPath = qt_GetLongPathName(path);
#else
    QString longPath = path;
#endif
    if (longPath == rootDir.path())
        absolutePath = rootDir.absolutePath();
    else
        absolutePath = QDir(longPath).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if ((pathElements.isEmpty())
#if (!defined(Q_OS_WIN) || defined(Q_OS_WINCE)) && !defined(Q_OS_SYMBIAN)
        && QDir::fromNativeSeparators(longPath) != QLatin1String("/")
#endif
        )
        return const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&root);
    QModelIndex index = QModelIndex(); // start with "My Computer"
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = QLatin1String("\\\\") + pathElements.first();
        if (absolutePath == QDir::fromNativeSeparators(host))
            absolutePath.append(QLatin1Char('/'));
        if (longPath.endsWith(QLatin1Char('/')) && !absolutePath.endsWith(QLatin1Char('/')))
            absolutePath.append(QLatin1Char('/'));
        int r = 0;
        CompareToolFileModelPrivate::QFileSystemNode *rootNode = const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&root);
        if (!root.children.contains(host.toLower())) {
            if (pathElements.count() == 1 && !absolutePath.endsWith(QLatin1Char('/')))
                return rootNode;
            QFileInfo info(host);
            if (!info.exists())
                return rootNode;
            CompareToolFileModelPrivate *p = const_cast<CompareToolFileModelPrivate*>(this);
            p->addNode(rootNode, host,info);
            p->addVisibleFiles(rootNode, QStringList(host));
        }
        r = rootNode->visibleLocation(host);
        r = translateVisibleLocation(rootNode, r);
        index = q->index(r, 0, QModelIndex());
        pathElements.pop_front();
    } else
#endif

#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    {
        if (!pathElements.at(0).contains(QLatin1String(":"))) {
            // The reason we express it like this instead of with anonymous, temporary
            // variables, is to workaround a compiler crash with Q_CC_NOKIAX86.
            QString rootPath = QDir(longPath).rootPath();
            pathElements.prepend(rootPath);
        }
        if (pathElements.at(0).endsWith(QLatin1Char('/')))
            pathElements[0].chop(1);
    }
#else
    // add the "/" item, since it is a valid path element on Unix
    if (absolutePath[0] == QLatin1Char('/'))
        pathElements.prepend(QLatin1String("/"));
#endif

    CompareToolFileModelPrivate::QFileSystemNode *parent = node(index);

    for (int i = 0; i < pathElements.count(); ++i) {
        QString element = pathElements.at(i);
#ifdef Q_OS_WIN
        // On Windows, "filename......." and "filename" are equivalent Task #133928
        while (element.endsWith(QLatin1Char('.')))
            element.chop(1);
#endif
        bool alreadyExisted = parent->children.contains(element);

        // we couldn't find the path element, we create a new node since we
        // _know_ that the path is valid
        if (alreadyExisted) {
            if ((parent->children.count() == 0)
                || (parent->caseSensitive()
                    && parent->children.value(element)->fileName != element)
                || (!parent->caseSensitive()
                    && parent->children.value(element)->fileName.toLower() != element.toLower()))
                alreadyExisted = false;
        }

        CompareToolFileModelPrivate::QFileSystemNode *node;
        if (!alreadyExisted) {
            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QFileInfo info(absolutePath);
            if (!info.exists())
                return const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&root);
            CompareToolFileModelPrivate *p = const_cast<CompareToolFileModelPrivate*>(this);
            node = p->addNode(parent, element,info);
#ifndef QT_NO_FILESYSTEMWATCHER
            node->populate(fileInfoGatherer.getInfo(info));
#endif
        } else {
            node = parent->children.value(element);
        }

        Q_ASSERT(node);
        if (!node->isVisible) {
            // It has been filtered out
            if (alreadyExisted && node->hasInformation() && !fetch)
                return const_cast<CompareToolFileModelPrivate::QFileSystemNode*>(&root);

            CompareToolFileModelPrivate *p = const_cast<CompareToolFileModelPrivate*>(this);
            p->addVisibleFiles(parent, QStringList(element));
            if (!p->bypassFilters.contains(node))
                p->bypassFilters[node] = 1;
            QString dir = q->filePath(this->index(parent));
            if (!node->hasInformation() && fetch) {
                Fetching f;
                f.dir = dir;
                f.file = element;
                f.node = node;
                p->toFetch.append(f);
                p->fetchingTimer.start(0, const_cast<CompareToolFileModel*>(q));
            }
        }
        parent = node;
    }

    return parent;
}

/*!
    \reimp
*/
void CompareToolFileModel::timerEvent(QTimerEvent *event)
{
    Q_D(CompareToolFileModel);
    if (event->timerId() == d->fetchingTimer.timerId()) {
        d->fetchingTimer.stop();
#ifndef QT_NO_FILESYSTEMWATCHER
        for (int i = 0; i < d->toFetch.count(); ++i) {
            const CompareToolFileModelPrivate::QFileSystemNode *node = d->toFetch.at(i).node;
            if (!node->hasInformation()) {
                d->fileInfoGatherer.fetchExtendedInformation(d->toFetch.at(i).dir,
                                                 QStringList(d->toFetch.at(i).file));
            } else {
                // qDebug() << "yah!, you saved a little gerbil soul";
            }
        }
#endif
        d->toFetch.clear();
    }
}

/*!
    Returns true if the model item \a index represents a directory;
    otherwise returns false.
*/
bool CompareToolFileModel::isDir(const QModelIndex &index) const
{
    // This function is for public usage only because it could create a file info
    Q_D(const CompareToolFileModel);
    if (!index.isValid())
        return true;
    CompareToolFileModelPrivate::QFileSystemNode *n = d->node(index);
    if (n->hasInformation())
        return n->isDir();
    return fileInfo(index).isDir();
}

/*!
    Returns the size in bytes of \a index. If the file does not exist, 0 is returned.
  */
qint64 CompareToolFileModel::size(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    if (!index.isValid())
        return 0;
    return d->node(index)->size();
}

/*!
    Returns the type of file \a index such as "Directory" or "JPEG file".
  */
QString CompareToolFileModel::type(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    if (!index.isValid())
        return QString();
    return d->node(index)->type();
}

/*!
    Returns the date and time when \a index was last modified.
 */
QDateTime CompareToolFileModel::lastModified(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    if (!index.isValid())
        return QDateTime();
    return d->node(index)->lastModified();
}

/*!
    \reimp
*/
QModelIndex CompareToolFileModel::parent(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    if (!d->indexValid(index))
        return QModelIndex();

    CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(index);
    Q_ASSERT(indexNode != 0);
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = (indexNode ? indexNode->parent : 0);
    if (parentNode == 0 || parentNode == &d->root)
        return QModelIndex();

    // get the parent's row
    CompareToolFileModelPrivate::QFileSystemNode *grandParentNode = parentNode->parent;
    Q_ASSERT(grandParentNode->children.contains(parentNode->fileName));
    int visualRow = d->translateVisibleLocation(grandParentNode, grandParentNode->visibleLocation(grandParentNode->children.value(parentNode->fileName)->fileName));
    if (visualRow == -1)
        return QModelIndex();
    return createIndex(visualRow, 0, parentNode);
}

/*
    \internal

    return the index for node
*/
QModelIndex CompareToolFileModelPrivate::index(const CompareToolFileModelPrivate::QFileSystemNode *node) const
{
    Q_Q(const CompareToolFileModel);
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = (node ? node->parent : 0);
    if (node == &root || !parentNode)
        return QModelIndex();

    // get the parent's row
    Q_ASSERT(node);
    if (!node->isVisible)
        return QModelIndex();

    int visualRow = translateVisibleLocation(parentNode, parentNode->visibleLocation(node->fileName));
    return q->createIndex(visualRow, 0, const_cast<QFileSystemNode*>(node));
}

/*!
    \reimp
*/
bool CompareToolFileModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const CompareToolFileModel);
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // drives
        return true;

    const CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    Q_ASSERT(indexNode);
    return (indexNode->isDir());
}

/*!
    \reimp
 */
bool CompareToolFileModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const CompareToolFileModel);
    const CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    return (!indexNode->populatedChildren);
}

/*!
    \reimp
 */
void CompareToolFileModel::fetchMore(const QModelIndex &parent)
{
    Q_D(CompareToolFileModel);
    if (!d->setRootPath)
        return;
    CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(parent);
    if (indexNode->populatedChildren)
        return;
    indexNode->populatedChildren = true;
    d->fileInfoGatherer.list(filePath(parent));
}

/*!
    \reimp
*/
int CompareToolFileModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const CompareToolFileModel);
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return d->root.visibleChildren.count();

    const CompareToolFileModelPrivate::QFileSystemNode *parentNode = d->node(parent);
    return parentNode->visibleChildren.count();
}

/*!
    \reimp
*/
int CompareToolFileModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 4;
}

/*!
    Returns the data stored under the given \a role for the item "My Computer".

    \sa Qt::ItemDataRole
 */
QVariant CompareToolFileModel::myComputer(int role) const
{
    Q_D(const CompareToolFileModel);
    switch (role) {
    case Qt::DisplayRole:
        return d->myComputer();
    case Qt::DecorationRole:
        return d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Computer);
    }
    return QVariant();
}

/*!
    \reimp
*/
QVariant CompareToolFileModel::data(const QModelIndex &index, int role) const
{
    Q_D(const CompareToolFileModel);
    if (!index.isValid() || index.model() != this)
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return d->displayName(index);
        case 1: return d->size(index);
        case 2: return d->type(index);
        case 3: return d->time(index);
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return filePath(index);
    case FileNameRole:
        return d->name(index);
    case Qt::DecorationRole:
        if (index.column() == 0) {
            QIcon icon = d->icon(index);
            if (icon.isNull()) {
                if (d->node(index)->isDir())
                    icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Folder);
                else
                    icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::File);
            }
            return icon;
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return Qt::AlignRight;
		break;
	case Qt::BackgroundRole:
		return d->node(index)->m_NodeBackground;
    case FilePermissions:
        int p = permissions(index);
        return p;

    }

    return QVariant();
}

QString CompareToolFileModelPrivate::size(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    const QFileSystemNode *n = node(index);
    if (n->isDir()) {
#ifdef Q_OS_MAC
        return QLatin1String("--");
#else
        return QLatin1String("");
#endif
    // Windows   - ""
    // OS X      - "--"
    // Konqueror - "4 KB"
    // Nautilus  - "9 items" (the number of children)
    }
    return size(n->size());
}

QString CompareToolFileModelPrivate::size(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return CompareToolFileModel::tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return CompareToolFileModel::tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return CompareToolFileModel::tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return CompareToolFileModel::tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return CompareToolFileModel::tr("%1 bytes").arg(QLocale().toString(bytes));
}

/*!
    \internal
*/
QString CompareToolFileModelPrivate::time(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
#ifndef QT_NO_DATESTRING
    return node(index)->lastModified().toString(Qt::SystemLocaleDate);
#else
    Q_UNUSED(index);
    return QString();
#endif
}

/*
    \internal
*/
QString CompareToolFileModelPrivate::type(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    return node(index)->type();
}

/*!
    \internal
*/
QString CompareToolFileModelPrivate::name(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    QFileSystemNode *dirNode = node(index);
    if (fileInfoGatherer.resolveSymlinks() && !resolvedSymLinks.isEmpty() &&
        dirNode->isSymLink(/* ignoreNtfsSymLinks = */ true)) {
        QString fullPath = QDir::fromNativeSeparators(filePath(index));
        if (resolvedSymLinks.contains(fullPath))
            return resolvedSymLinks[fullPath];
    }
    return dirNode->fileName;
}

/*!
    \internal
*/
QString CompareToolFileModelPrivate::displayName(const QModelIndex &index) const
{
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    QFileSystemNode *dirNode = node(index);
    if (!dirNode->volumeName.isNull())
        return dirNode->volumeName + QLatin1String(" (") + name(index) + QLatin1Char(')');
#endif
    return name(index);
}

/*!
    \internal
*/
QIcon CompareToolFileModelPrivate::icon(const QModelIndex &index) const
{
    if (!index.isValid())
        return QIcon();
    return node(index)->icon();
}

/*!
    \reimp
*/
bool CompareToolFileModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    Q_D(CompareToolFileModel);
	//hx:���ñ���
	if (role == Qt::BackgroundRole)
	{
		CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(idx);
		if (indexNode->isVisible)
		{
			indexNode->m_NodeBackground = value.value<QColor>();
		}
	}
	//if (role != Qt::BackgroundRole)
	{
		if (!idx.isValid()
			|| idx.column() != 0
			|| role != Qt::EditRole
			|| (flags(idx) & Qt::ItemIsEditable) == 0) {
				return false;
		}
	}
    QString newName = value.toString();
    QString oldName = idx.data().toString();
    if (newName == idx.data().toString())
        return true;

    if (newName.isEmpty()
        || QDir::toNativeSeparators(newName).contains(QDir::separator())
        || !QDir(filePath(parent(idx))).rename(oldName, newName)) {
#ifndef QT_NO_MESSAGEBOX
        QMessageBox::information(0, CompareToolFileModel::tr("Invalid filename"),
                                CompareToolFileModel::tr("<b>The name \"%1\" can not be used.</b><p>Try using another name, with fewer characters or no punctuations marks.")
                                .arg(newName),
                                 QMessageBox::Ok);
#endif // QT_NO_MESSAGEBOX
        return false;
    } else {
        /*
            *After re-naming something we don't want the selection to change*
            - can't remove rows and later insert
            - can't quickly remove and insert
            - index pointer can't change because treeview doesn't use persistant index's

            - if this get any more complicated think of changing it to just
              use layoutChanged
         */

        CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(idx);
        CompareToolFileModelPrivate::QFileSystemNode *parentNode = indexNode->parent;
        int visibleLocation = parentNode->visibleLocation(parentNode->children.value(indexNode->fileName)->fileName);

        d->addNode(parentNode, newName,indexNode->info->fileInfo());
        parentNode->visibleChildren.removeAt(visibleLocation);
        CompareToolFileModelPrivate::QFileSystemNode * oldValue = parentNode->children.value(oldName);
        parentNode->children[newName] = oldValue;
        QFileInfo info(d->rootDir, newName);
        oldValue->fileName = newName;
        oldValue->parent = parentNode;
        oldValue->populate(d->fileInfoGatherer.getInfo(info));
        oldValue->isVisible = true;

        parentNode->children.remove(oldName);
        parentNode->visibleChildren.insert(visibleLocation, newName);
        d->delayedSort();
        emit fileRenamed(filePath(idx.parent()), oldName, newName);
    }
    return true;
}

/*!
    \reimp
*/
Qt::ItemFlags CompareToolFileModel::flags(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(index);
    if (d->nameFilterDisables && !d->passNameFilters(indexNode)) {
        flags &= ~Qt::ItemIsEnabled;
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags;
    }

    flags |= Qt::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    if ((index.column() == 0) && indexNode->permissions() & QFile::WriteUser) {
        flags |= Qt::ItemIsEditable;
        if (indexNode->isDir())
            flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

/*!
    \internal
*/
void CompareToolFileModelPrivate::_q_performDelayedSort()
{
    Q_Q(CompareToolFileModel);
    q->sort(sortColumn, sortOrder);
}

static inline QChar getNextChar(const QString &s, int location)
{
    return (location < s.length()) ? s.at(location) : QChar();
}

/*!
    Natural number sort, skips spaces.

    Examples:
    1, 2, 10, 55, 100
    01.jpg, 2.jpg, 10.jpg

    Note on the algorithm:
    Only as many characters as necessary are looked at and at most they all
    are looked at once.

    Slower then QString::compare() (of course)
  */
int CompareToolFileModelPrivate::naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity cs)
{
    for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
        // skip spaces, tabs and 0's
        QChar c1 = getNextChar(s1, l1);
        while (c1.isSpace())
            c1 = getNextChar(s1, ++l1);
        QChar c2 = getNextChar(s2, l2);
        while (c2.isSpace())
            c2 = getNextChar(s2, ++l2);

        if (c1.isDigit() && c2.isDigit()) {
            while (c1.digitValue() == 0)
                c1 = getNextChar(s1, ++l1);
            while (c2.digitValue() == 0)
                c2 = getNextChar(s2, ++l2);

            int lookAheadLocation1 = l1;
            int lookAheadLocation2 = l2;
            int currentReturnValue = 0;
            // find the last digit, setting currentReturnValue as we go if it isn't equal
            for (
                QChar lookAhead1 = c1, lookAhead2 = c2;
                (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
                lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
                lookAhead2 = getNextChar(s2, ++lookAheadLocation2)
                ) {
                bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
                bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
                if (!is1ADigit && !is2ADigit)
                    break;
                if (!is1ADigit)
                    return -1;
                if (!is2ADigit)
                    return 1;
                if (currentReturnValue == 0) {
                    if (lookAhead1 < lookAhead2) {
                        currentReturnValue = -1;
                    } else if (lookAhead1 > lookAhead2) {
                        currentReturnValue = 1;
                    }
                }
            }
            if (currentReturnValue != 0)
                return currentReturnValue;
        }

        if (cs == Qt::CaseInsensitive) {
            if (!c1.isLower()) c1 = c1.toLower();
            if (!c2.isLower()) c2 = c2.toLower();
        }
        int r = QString::localeAwareCompare(c1, c2);
        if (r < 0)
            return -1;
        if (r > 0)
            return 1;
    }
    // The two strings are the same (02 == 2) so fall back to the normal sort
    return QString::compare(s1, s2, cs);
}

/*
    \internal
    Helper functor used by sort()
*/
class CompareToolFileModelSorter
{
public:
    inline CompareToolFileModelSorter(int column) : sortColumn(column) {}

    bool compareNodes(const CompareToolFileModelPrivate::QFileSystemNode *l,
                    const CompareToolFileModelPrivate::QFileSystemNode *r) const
    {
        switch (sortColumn) {
        case 0: {
#ifndef Q_OS_MAC
            // place directories before files
            bool left = l->isDir();
            bool right = r->isDir();
            if (left ^ right)
                return left;
#endif
            return CompareToolFileModelPrivate::naturalCompare(l->fileName,
                                                r->fileName, Qt::CaseInsensitive) < 0;
                }
        case 1:
            // Directories go first
            if (l->isDir() && !r->isDir())
                return true;
            return l->size() < r->size();
        case 2:
            return l->type() < r->type();
        case 3:
            return l->lastModified() < r->lastModified();
        }
        Q_ASSERT(false);
        return false;
    }

    bool operator()(const QPair<CompareToolFileModelPrivate::QFileSystemNode*, int> &l,
                           const QPair<CompareToolFileModelPrivate::QFileSystemNode*, int> &r) const
    {
        return compareNodes(l.first, r.first);
    }


private:
    int sortColumn;
};

/*
    \internal

    Sort all of the children of parent
*/
void CompareToolFileModelPrivate::sortChildren(int column, const QModelIndex &parent)
{
    Q_Q(CompareToolFileModel);
    CompareToolFileModelPrivate::QFileSystemNode *indexNode = node(parent);
    if (indexNode->children.count() == 0)
        return;

    QList<QPair<CompareToolFileModelPrivate::QFileSystemNode*, int> > values;
    QHash<QString, QFileSystemNode *>::const_iterator iterator;
    int i = 0;
    for(iterator = indexNode->children.begin() ; iterator != indexNode->children.end() ; ++iterator) {
        if (filtersAcceptsNode(iterator.value())) {
            values.append(QPair<CompareToolFileModelPrivate::QFileSystemNode*, int>((iterator.value()), i));
        } else {
            iterator.value()->isVisible = false;
        }
        i++;
    }
    CompareToolFileModelSorter ms(column);
    qStableSort(values.begin(), values.end(), ms);
    // First update the new visible list
    indexNode->visibleChildren.clear();
    //No more dirty item we reset our internal dirty index
    indexNode->dirtyChildrenIndex = -1;
    for (int i = 0; i < values.count(); ++i) {
        indexNode->visibleChildren.append(values.at(i).first->fileName);
        values.at(i).first->isVisible = true;
    }

    if (!disableRecursiveSort) {
        for (int i = 0; i < q->rowCount(parent); ++i) {
            const QModelIndex childIndex = q->index(i, 0, parent);
            CompareToolFileModelPrivate::QFileSystemNode *indexNode = node(childIndex);
            //Only do a recursive sort on visible nodes
            if (indexNode->isVisible)
                sortChildren(column, childIndex);
        }
    }
}

/*!
    \reimp
*/
void CompareToolFileModel::sort(int column, Qt::SortOrder order)
{
    Q_D(CompareToolFileModel);
    if (d->sortOrder == order && d->sortColumn == column && !d->forceSort)
        return;

    emit layoutAboutToBeChanged();
    QModelIndexList oldList = persistentIndexList();
    QList<QPair<CompareToolFileModelPrivate::QFileSystemNode*, int> > oldNodes;
    for (int i = 0; i < oldList.count(); ++i) {
        QPair<CompareToolFileModelPrivate::QFileSystemNode*, int> pair(d->node(oldList.at(i)), oldList.at(i).column());
        oldNodes.append(pair);
    }

    if (!(d->sortColumn == column && d->sortOrder != order && !d->forceSort)) {
        //we sort only from where we are, don't need to sort all the model
        d->sortChildren(column, index(rootPath()));
        d->sortColumn = column;
        d->forceSort = false;
    }
    d->sortOrder = order;

    QModelIndexList newList;
    for (int i = 0; i < oldNodes.count(); ++i) {
        QModelIndex idx = d->index(oldNodes.at(i).first);
        idx = idx.sibling(idx.row(), oldNodes.at(i).second);
        newList.append(idx);
    }
    changePersistentIndexList(oldList, newList);
    emit layoutChanged();
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/
QStringList CompareToolFileModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *CompareToolFileModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QList<QModelIndex>::const_iterator it = indexes.begin();
    for (; it != indexes.end(); ++it)
        if ((*it).column() == 0)
            urls << QUrl::fromLocalFile(filePath(*it));
    QMimeData *data = new QMimeData();
    data->setUrls(urls);
    return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row and
    \a column and by the \a parent index.

    \sa supportedDropActions()
*/
bool CompareToolFileModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid() || isReadOnly())
        return false;

    bool success = true;
    QString to = filePath(parent) + QDir::separator();

    QList<QUrl> urls = data->urls();
    QList<QUrl>::const_iterator it = urls.constBegin();

    switch (action) {
    case Qt::CopyAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::rename(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    default:
        return false;
    }

    return success;
}

/*!
    \reimp
*/
Qt::DropActions CompareToolFileModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

/*!
    Returns the path of the item stored in the model under the
    \a index given.
*/
QString CompareToolFileModel::filePath(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    QString fullPath = d->filePath(index);
    CompareToolFileModelPrivate::QFileSystemNode *dirNode = d->node(index);
    if (dirNode->isSymLink() && d->fileInfoGatherer.resolveSymlinks()
        && d->resolvedSymLinks.contains(fullPath)
        && dirNode->isDir()) {
        QFileInfo resolvedInfo(fullPath);
        resolvedInfo = resolvedInfo.canonicalFilePath();
        if (resolvedInfo.exists())
            return resolvedInfo.filePath();
    }
    return fullPath;
}

QString CompareToolFileModelPrivate::filePath(const QModelIndex &index) const
{
    Q_Q(const CompareToolFileModel);
    Q_UNUSED(q);
    if (!index.isValid())
        return QString();
    Q_ASSERT(index.model() == q);

    QStringList path;
    QModelIndex idx = index;
    while (idx.isValid()) {
        CompareToolFileModelPrivate::QFileSystemNode *dirNode = node(idx);
        if (dirNode)
            path.prepend(dirNode->fileName);
        idx = idx.parent();
    }
    QString fullPath = QDir::fromNativeSeparators(path.join(QDir::separator()));
#if !defined(Q_OS_WIN) || defined(Q_OS_WINCE)
    if ((fullPath.length() > 2) && fullPath[0] == QLatin1Char('/') && fullPath[1] == QLatin1Char('/'))
        fullPath = fullPath.mid(1);
#endif
#if defined(Q_OS_WIN) || defined(Q_OS_SYMBIAN)
    if (fullPath.length() == 2 && fullPath.endsWith(QLatin1Char(':')))
        fullPath.append(QLatin1Char('/'));
#endif
    return fullPath;
}

/*!
    Create a directory with the \a name in the \a parent model index.
*/
QModelIndex CompareToolFileModel::mkdir(const QModelIndex &parent, const QString &name)
{
    Q_D(CompareToolFileModel);
    if (!parent.isValid())
        return parent;

    QDir dir(filePath(parent));
    if (!dir.mkdir(name))
        return QModelIndex();
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = d->node(parent);
    d->addNode(parentNode, name, QFileInfo());
    Q_ASSERT(parentNode->children.contains(name));
    CompareToolFileModelPrivate::QFileSystemNode *node = parentNode->children[name];
    node->populate(d->fileInfoGatherer.getInfo(QFileInfo(dir.absolutePath() + QDir::separator() + name)));
    d->addVisibleFiles(parentNode, QStringList(name));
    return d->index(node);
}

/*!
    Returns the complete OR-ed together combination of QFile::Permission for the \a index.
 */
QFile::Permissions CompareToolFileModel::permissions(const QModelIndex &index) const
{
    Q_D(const CompareToolFileModel);
    QFile::Permissions p = d->node(index)->permissions();
    if (d->readOnly) {
        p ^= (QFile::WriteOwner | QFile::WriteUser
            | QFile::WriteGroup | QFile::WriteOther);
    }
    return p;
}

/*!
    Sets the directory that is being watched by the model to \a newPath by
    installing a \l{QFileSystemWatcher}{file system watcher} on it. Any
    changes to files and directories within this directory will be
    reflected in the model.

    If the path is changed, the rootPathChanged() signal will be emitted.

    \note This function does not change the structure of the model or
    modify the data available to views. In other words, the "root" of
    the model is \e not changed to include only files and directories
    within the directory specified by \a newPath in the file system.
  */
QModelIndex CompareToolFileModel::setRootPath(const QString &newPath)
{
    Q_D(CompareToolFileModel);
#ifdef Q_OS_WIN
#ifdef Q_OS_WIN32
    QString longNewPath = qt_GetLongPathName(newPath);
#else
    QString longNewPath = QDir::fromNativeSeparators(newPath);
#endif
#else
    QString longNewPath = newPath;
#endif
    QDir newPathDir(longNewPath);
    //we remove .. and . from the given path if exist
    if (!newPath.isEmpty()) {
        longNewPath = QDir::cleanPath(longNewPath);
        newPathDir.setPath(longNewPath);
    }

    d->setRootPath = true;

    //user don't ask for the root path ("") but the conversion failed
    if (!newPath.isEmpty() && longNewPath.isEmpty())
        return d->index(rootPath());

    if (d->rootDir.path() == longNewPath)
        return d->index(rootPath());

    bool showDrives = (longNewPath.isEmpty() || longNewPath == d->myComputer());
    if (!showDrives && !newPathDir.exists())
        return d->index(rootPath());

    //We remove the watcher on the previous path
    if (!rootPath().isEmpty() && rootPath() != QLatin1String(".")) {
        //This remove the watcher for the old rootPath
        d->fileInfoGatherer.removePath(rootPath());
        //This line "marks" the node as dirty, so the next fetchMore
        //call on the path will ask the gatherer to install a watcher again
        //But it doesn't re-fetch everything
        d->node(rootPath())->populatedChildren = false;
    }

    // We have a new valid root path
    d->rootDir = newPathDir;
    QModelIndex newRootIndex;
    if (showDrives) {
        // otherwise dir will become '.'
        d->rootDir.setPath(QLatin1String(""));
    } else {
        newRootIndex = d->index(newPathDir.path());
    }
    fetchMore(newRootIndex);
    emit rootPathChanged(longNewPath);
    d->forceSort = true;
    d->delayedSort();
    return newRootIndex;
}

/*!
    The currently set root path

    \sa rootDirectory()
*/
QString CompareToolFileModel::rootPath() const
{
    Q_D(const CompareToolFileModel);
    return d->rootDir.path();
}

/*!
    The currently set directory

    \sa rootPath()
*/
QDir CompareToolFileModel::rootDirectory() const
{
    Q_D(const CompareToolFileModel);
    QDir dir(d->rootDir);
    dir.setNameFilters(nameFilters());
    dir.setFilter(filter());
    return dir;
}

/*!
    Sets the \a provider of file icons for the directory model.
*/
void CompareToolFileModel::setIconProvider(QFileIconProvider *provider)
{
    Q_D(CompareToolFileModel);
    d->fileInfoGatherer.setIconProvider(provider);
    d->root.updateIcon(provider, QString());
}

/*!
    Returns the file icon provider for this directory model.
*/
QFileIconProvider *CompareToolFileModel::iconProvider() const
{
    Q_D(const CompareToolFileModel);
    return d->fileInfoGatherer.iconProvider();
}

/*!
    Sets the directory model's filter to that specified by \a filters.

    Note that the filter you set should always include the QDir::AllDirs enum value,
    otherwise CompareToolFileModel won't be able to read the directory structure.

    \sa QDir::Filters
*/
void CompareToolFileModel::setFilter(QDir::Filters filters)
{
    Q_D(CompareToolFileModel);
    if (d->filters == filters)
        return;
    d->filters = filters;
    // CaseSensitivity might have changed
    setNameFilters(nameFilters());
    d->forceSort = true;
    d->delayedSort();
}

/*!
    Returns the filter specified for the directory model.

    If a filter has not been set, the default filter is QDir::AllEntries |
    QDir::NoDotAndDotDot | QDir::AllDirs.

    \sa QDir::Filters
*/
QDir::Filters CompareToolFileModel::filter() const
{
    Q_D(const CompareToolFileModel);
    return d->filters;
}

/*!
    \property CompareToolFileModel::resolveSymlinks
    \brief Whether the directory model should resolve symbolic links

    This is only relevant on operating systems that support symbolic links.

    By default, this property is false.
*/
void CompareToolFileModel::setResolveSymlinks(bool enable)
{
    Q_D(CompareToolFileModel);
    d->fileInfoGatherer.setResolveSymlinks(enable);
}

bool CompareToolFileModel::resolveSymlinks() const
{
    Q_D(const CompareToolFileModel);
    return d->fileInfoGatherer.resolveSymlinks();
}

/*!
    \property CompareToolFileModel::readOnly
    \brief Whether the directory model allows writing to the file system

    If this property is set to false, the directory model will allow renaming, copying
    and deleting of files and directories.

    This property is true by default
*/
void CompareToolFileModel::setReadOnly(bool enable)
{
    Q_D(CompareToolFileModel);
    d->readOnly = enable;
}

bool CompareToolFileModel::isReadOnly() const
{
    Q_D(const CompareToolFileModel);
    return d->readOnly;
}

/*!
    \property CompareToolFileModel::nameFilterDisables
    \brief Whether files that don't pass the name filter are hidden or disabled

    This property is true by default
*/
void CompareToolFileModel::setNameFilterDisables(bool enable)
{
    Q_D(CompareToolFileModel);
    if (d->nameFilterDisables == enable)
        return;
    d->nameFilterDisables = enable;
    d->forceSort = true;
    d->delayedSort();
}

bool CompareToolFileModel::nameFilterDisables() const
{
    Q_D(const CompareToolFileModel);
    return d->nameFilterDisables;
}

/*!
    Sets the name \a filters to apply against the existing files.
*/
void CompareToolFileModel::setNameFilters(const QStringList &filters)
{
    // Prep the regexp's ahead of time
#ifndef QT_NO_REGEXP
    Q_D(CompareToolFileModel);

    if (!d->bypassFilters.isEmpty()) {
        // update the bypass filter to only bypass the stuff that must be kept around
        d->bypassFilters.clear();
        // We guarantee that rootPath will stick around
        QPersistentModelIndex root(index(rootPath()));
        QModelIndexList persistantList = persistentIndexList();
        for (int i = 0; i < persistantList.count(); ++i) {
            CompareToolFileModelPrivate::QFileSystemNode *node;
            node = d->node(persistantList.at(i));
            while (node) {
                if (d->bypassFilters.contains(node))
                    break;
                if (node->isDir())
                    d->bypassFilters[node] = true;
                node = node->parent;
            }
        }
    }

    d->nameFilters.clear();
    const Qt::CaseSensitivity caseSensitive =
        (filter() & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (int i = 0; i < filters.size(); ++i) {
        d->nameFilters << QRegExp(filters.at(i), caseSensitive, QRegExp::Wildcard);
    }
    d->forceSort = true;
    d->delayedSort();
#endif
}

/*!
    Returns a list of filters applied to the names in the model.
*/
QStringList CompareToolFileModel::nameFilters() const
{
    Q_D(const CompareToolFileModel);
    QStringList filters;
#ifndef QT_NO_REGEXP
    for (int i = 0; i < d->nameFilters.size(); ++i) {
         filters << d->nameFilters.at(i).pattern();
    }
#endif
    return filters;
}

/*!
    \reimp
*/
bool CompareToolFileModel::event(QEvent *event)
{
    Q_D(CompareToolFileModel);
    if (event->type() == QEvent::LanguageChange) {
        d->root.retranslateStrings(d->fileInfoGatherer.iconProvider(), QString());
        return true;
    }
    return QAbstractItemModel::event(event);
}

bool CompareToolFileModel::rmdir(const QModelIndex &aindex) const
{
    QString path = filePath(aindex);
    CompareToolFileModelPrivate * d = const_cast<CompareToolFileModelPrivate*>(d_func());
    d->fileInfoGatherer.removePath(path);
    return QDir().rmdir(path);
}

/*!
     \internal

    Performed quick listing and see if any files have been added or removed,
    then fetch more information on visible files.
 */
void CompareToolFileModelPrivate::_q_directoryChanged(const QString &directory, const QStringList &files)
{
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = node(directory, false);
    if (parentNode->children.count() == 0)
        return;
    QStringList toRemove;
#if defined(Q_OS_SYMBIAN)
    // Filename case must be exact in qBinaryFind below, so create a list of all lowercase names.
    QStringList newFiles;
    for(int i = 0; i < files.size(); i++) {
        newFiles << files.at(i).toLower();
    }
#else
    QStringList newFiles = files;
#endif
    qSort(newFiles.begin(), newFiles.end());
    QHash<QString, QFileSystemNode*>::const_iterator i = parentNode->children.constBegin();
    while (i != parentNode->children.constEnd()) {
        QStringList::iterator iterator;
        iterator = qBinaryFind(newFiles.begin(), newFiles.end(),
#if defined(Q_OS_SYMBIAN)
                    i.value()->fileName.toLower());
#else
                    i.value()->fileName);
#endif
        if (iterator == newFiles.end()) {
            toRemove.append(i.value()->fileName);
        }
        ++i;
    }
    for (int i = 0 ; i < toRemove.count() ; ++i )
        removeNode(parentNode, toRemove[i]);
}

/*!
    \internal

    Adds a new file to the children of parentNode

    *WARNING* this will change the count of children
*/
CompareToolFileModelPrivate::QFileSystemNode* CompareToolFileModelPrivate::addNode(QFileSystemNode *parentNode, const QString &fileName, const QFileInfo& info)
{
    // In the common case, itemLocation == count() so check there first
    CompareToolFileModelPrivate::QFileSystemNode *node = new CompareToolFileModelPrivate::QFileSystemNode(fileName, parentNode);
#ifndef QT_NO_FILESYSTEMWATCHER
    node->populate(info);
#endif
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    //The parentNode is "" so we are listing the drives
    if (parentNode->fileName.isEmpty()) {
        wchar_t name[MAX_PATH + 1];
        //GetVolumeInformation requires to add trailing backslash
        const QString nodeName = fileName + QLatin1String("\\");
        BOOL success = ::GetVolumeInformation((wchar_t *)(nodeName.utf16()),
                name, MAX_PATH + 1, NULL, 0, NULL, NULL, 0);
        if (success && name[0])
            node->volumeName = QString::fromWCharArray(name);
    }
#endif
    parentNode->children.insert(fileName, node);
    return node;
}

/*!
    \internal

    File at parentNode->children(itemLocation) has been removed, remove from the lists
    and emit signals if necessary

    *WARNING* this will change the count of children and could change visibleChildren
 */
void CompareToolFileModelPrivate::removeNode(CompareToolFileModelPrivate::QFileSystemNode *parentNode, const QString& name)
{
    Q_Q(CompareToolFileModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);

    int vLocation = parentNode->visibleLocation(name);
    if (vLocation >= 0 && !indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    QFileSystemNode * node = parentNode->children.take(name);
    delete node;
    // cleanup sort files after removing rather then re-sorting which is O(n)
    if (vLocation >= 0)
        parentNode->visibleChildren.removeAt(vLocation);
    if (vLocation >= 0 && !indexHidden)
        q->endRemoveRows();
}

/*
    \internal
    Helper functor used by addVisibleFiles()
*/
class CompareToolFileModelVisibleFinder
{
public:
    inline CompareToolFileModelVisibleFinder(CompareToolFileModelPrivate::QFileSystemNode *node, CompareToolFileModelSorter *sorter) : parentNode(node), sorter(sorter) {}

    bool operator()(const QString &, QString r) const
    {
        return sorter->compareNodes(parentNode->children.value(name), parentNode->children.value(r));
    }

    QString name;
private:
    CompareToolFileModelPrivate::QFileSystemNode *parentNode;
    CompareToolFileModelSorter *sorter;
};

/*!
    \internal

    File at parentNode->children(itemLocation) was not visible before, but now should be
    and emit signals if necessary.

    *WARNING* this will change the visible count
 */
void CompareToolFileModelPrivate::addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles)
{
    Q_Q(CompareToolFileModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);
    if (!indexHidden) {
        q->beginInsertRows(parent, parentNode->visibleChildren.count() , parentNode->visibleChildren.count() + newFiles.count() - 1);
    }

    if (parentNode->dirtyChildrenIndex == -1)
        parentNode->dirtyChildrenIndex = parentNode->visibleChildren.count();

    for (int i = 0; i < newFiles.count(); ++i) {
            parentNode->visibleChildren.append(newFiles.at(i));
            parentNode->children[newFiles.at(i)]->isVisible = true;
        }
    if (!indexHidden)
      q->endInsertRows();
}

/*!
    \internal

    File was visible before, but now should NOT be

    *WARNING* this will change the visible count
 */
void CompareToolFileModelPrivate::removeVisibleFile(QFileSystemNode *parentNode, int vLocation)
{
    Q_Q(CompareToolFileModel);
    if (vLocation == -1)
        return;
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);
    if (!indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    parentNode->children[parentNode->visibleChildren.at(vLocation)]->isVisible = false;
    parentNode->visibleChildren.removeAt(vLocation);
    if (!indexHidden)
        q->endRemoveRows();
}

/*!
    \internal

    The thread has received new information about files,
    update and emit dataChanged if it has actually changed.
 */
void CompareToolFileModelPrivate::_q_fileSystemChanged(const QString &path, const QList<QPair<QString, QFileInfo> > &updates)
{
    Q_Q(CompareToolFileModel);
    QVector<QString> rowsToUpdate;
    QStringList newFiles;
    CompareToolFileModelPrivate::QFileSystemNode *parentNode = node(path, false);
    QModelIndex parentIndex = index(parentNode);
    for (int i = 0; i < updates.count(); ++i) {
        QString fileName = updates.at(i).first;
        Q_ASSERT(!fileName.isEmpty());
        QExtendedInformation info = fileInfoGatherer.getInfo(updates.at(i).second);
        bool previouslyHere = parentNode->children.contains(fileName);
        if (!previouslyHere) {
            addNode(parentNode, fileName, info.fileInfo());
        }
        CompareToolFileModelPrivate::QFileSystemNode * node = parentNode->children.value(fileName);
        bool isCaseSensitive = parentNode->caseSensitive();
        if (isCaseSensitive) {
            if (node->fileName != fileName)
                continue;
        } else {
            if (QString::compare(node->fileName,fileName,Qt::CaseInsensitive) != 0)
                continue;
        }
        if (isCaseSensitive) {
            Q_ASSERT(node->fileName == fileName);
        } else {
            node->fileName = fileName;
        }

        if (info.size() == -1 && !info.isSymLink()) {
            removeNode(parentNode, fileName);
            continue;
        }
        if (*node != info ) {
            node->populate(info);
            bypassFilters.remove(node);
            // brand new information.
            if (filtersAcceptsNode(node)) {
                if (!node->isVisible) {
                    newFiles.append(fileName);
                } else {
                    rowsToUpdate.append(fileName);
                }
            } else {
                if (node->isVisible) {
                    int visibleLocation = parentNode->visibleLocation(fileName);
                    removeVisibleFile(parentNode, visibleLocation);
                } else {
                    // The file is not visible, don't do anything
                }
            }
        }
    }

    // bundle up all of the changed signals into as few as possible.
    qSort(rowsToUpdate.begin(), rowsToUpdate.end());
    QString min;
    QString max;
    for (int i = 0; i < rowsToUpdate.count(); ++i) {
        QString value = rowsToUpdate.at(i);
        //##TODO is there a way to bundle signals with QString as the content of the list?
        /*if (min.isEmpty()) {
            min = value;
            if (i != rowsToUpdate.count() - 1)
                continue;
        }
        if (i != rowsToUpdate.count() - 1) {
            if ((value == min + 1 && max.isEmpty()) || value == max + 1) {
                max = value;
                continue;
            }
        }*/
        max = value;
        min = value;
        int visibleMin = parentNode->visibleLocation(min);
        int visibleMax = parentNode->visibleLocation(max);
        if (visibleMin >= 0
            && visibleMin < parentNode->visibleChildren.count()
            && parentNode->visibleChildren.at(visibleMin) == min
            && visibleMax >= 0) {
            QModelIndex bottom = q->index(translateVisibleLocation(parentNode, visibleMin), 0, parentIndex);
            QModelIndex top = q->index(translateVisibleLocation(parentNode, visibleMax), 3, parentIndex);
            emit q->dataChanged(bottom, top);
        }

        /*min = QString();
        max = QString();*/
    }

    if (newFiles.count() > 0) {
        addVisibleFiles(parentNode, newFiles);
    }

    if (newFiles.count() > 0 || (sortColumn != 0 && rowsToUpdate.count() > 0)) {
        forceSort = true;
        delayedSort();
    }
}

/*!
    \internal
*/
void CompareToolFileModelPrivate::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
    resolvedSymLinks[fileName] = resolvedName;
}

/*!
    \internal
*/
void CompareToolFileModelPrivate::init()
{
    Q_Q(CompareToolFileModel);
    qRegisterMetaType<QList<QPair<QString,QFileInfo> > >("QList<QPair<QString,QFileInfo> >");
    q->connect(&fileInfoGatherer, SIGNAL(newListOfFiles(QString,QStringList)),
               q, SLOT(_q_directoryChanged(QString,QStringList)));
    q->connect(&fileInfoGatherer, SIGNAL(updates(QString,QList<QPair<QString,QFileInfo> >)),
            q, SLOT(_q_fileSystemChanged(QString,QList<QPair<QString,QFileInfo> >)));
    q->connect(&fileInfoGatherer, SIGNAL(nameResolved(QString,QString)),
            q, SLOT(_q_resolvedName(QString,QString)));
    q->connect(&fileInfoGatherer, SIGNAL(directoryLoaded(QString)),
               q, SIGNAL(directoryLoaded(QString)));
    q->connect(&delayedSortTimer, SIGNAL(timeout()), q, SLOT(_q_performDelayedSort()), Qt::QueuedConnection);

    QHash<int, QByteArray> roles = q->roleNames();
    roles.insertMulti(CompareToolFileModel::FileIconRole, "fileIcon"); // == Qt::decoration
    roles.insert(CompareToolFileModel::FilePathRole, "filePath");
    roles.insert(CompareToolFileModel::FileNameRole, "fileName");
    roles.insert(CompareToolFileModel::FilePermissions, "filePermissions");
    q->setRoleNames(roles);
}

/*!
    \internal

    Returns false if node doesn't pass the filters otherwise true

    QDir::Modified is not supported
    QDir::Drives is not supported
*/
bool CompareToolFileModelPrivate::filtersAcceptsNode(const QFileSystemNode *node) const
{
    // always accept drives
    if (node->parent == &root || bypassFilters.contains(node))
        return true;

    // If we don't know anything yet don't accept it
    if (!node->hasInformation())
        return false;

    const bool filterPermissions = ((filters & QDir::PermissionMask)
                                   && (filters & QDir::PermissionMask) != QDir::PermissionMask);
    const bool hideDirs          = !(filters & (QDir::Dirs | QDir::AllDirs));
    const bool hideFiles         = !(filters & QDir::Files);
    const bool hideReadable      = !(!filterPermissions || (filters & QDir::Readable));
    const bool hideWritable      = !(!filterPermissions || (filters & QDir::Writable));
    const bool hideExecutable    = !(!filterPermissions || (filters & QDir::Executable));
    const bool hideHidden        = !(filters & QDir::Hidden);
    const bool hideSystem        = !(filters & QDir::System);
    const bool hideSymlinks      = (filters & QDir::NoSymLinks);
    const bool hideDot           = (filters & QDir::NoDot) || (filters & QDir::NoDotAndDotDot); // ### Qt5: simplify (because NoDotAndDotDot=NoDot|NoDotDot)
    const bool hideDotDot        = (filters & QDir::NoDotDot) || (filters & QDir::NoDotAndDotDot); // ### Qt5: simplify (because NoDotAndDotDot=NoDot|NoDotDot)

    // Note that we match the behavior of entryList and not QFileInfo on this and this
    // incompatibility won't be fixed until Qt 5 at least
    bool isDot    = (node->fileName == QLatin1String("."));
    bool isDotDot = (node->fileName == QLatin1String(".."));
    if (   (hideHidden && !(isDot || isDotDot) && node->isHidden())
        || (hideSystem && node->isSystem())
        || (hideDirs && node->isDir())
        || (hideFiles && node->isFile())
        || (hideSymlinks && node->isSymLink())
        || (hideReadable && node->isReadable())
        || (hideWritable && node->isWritable())
        || (hideExecutable && node->isExecutable())
        || (hideDot && isDot)
        || (hideDotDot && isDotDot))
        return false;

    return nameFilterDisables || passNameFilters(node);
}

/*
    \internal

    Returns true if node passes the name filters and should be visible.
 */
bool CompareToolFileModelPrivate::passNameFilters(const QFileSystemNode *node) const
{
#ifndef QT_NO_REGEXP
    if (nameFilters.isEmpty())
        return true;

    // Check the name regularexpression filters
    if (!(node->isDir() && (filters & QDir::AllDirs))) {
        for (int i = 0; i < nameFilters.size(); ++i) {
            if (nameFilters.at(i).exactMatch(node->fileName))
                return true;
        }
        return false;
    }
#endif
    return true;
}


bool CompareToolFileModelPrivate::passNameFilters( const QString & filePath ) const
{
#ifndef QT_NO_REGEXP
	if (nameFilters.isEmpty())
		return true;

	QFileInfo file(filePath);
	// Check the name regularexpression filters
	if (!(file.isDir() && (filters & QDir::AllDirs))) {
		for (int i = 0; i < nameFilters.size(); ++i) {
			if (nameFilters.at(i).exactMatch(file.fileName()))
				return true;
		}
		return false;
	}
#endif
	return true;
}


QVariant CompareToolFileModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const 
{
	switch (role) {
	case Qt::DecorationRole:
		if (section == 0) {
			// ### TODO oh man this is ugly and doesn't even work all the way!
			// it is still 2 pixels off
			QImage pixmap(16, 1, QImage::Format_Mono);
			pixmap.fill(0);
			pixmap.setAlphaChannel(pixmap.createAlphaMask());
			return pixmap;
		}
		break;
	case Qt::TextAlignmentRole:
		return Qt::AlignLeft;
	}

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QAbstractItemModel::headerData(section, orientation, role);

	QString returnValue;
	switch (section) {
	case 0: returnValue = tr("�ļ���");
		break;
	case 1: returnValue = tr("��С");
		break;
	case 2: returnValue = tr("����");
		break;
		// Windows   - Type
		// OS X      - Kind
		// Konqueror - File Type
		// Nautilus  - Type
	case 3: returnValue = tr("����޸�ʱ��");
		break;
	default: return QVariant();
	}
	return returnValue;

}
//
////�����ļ���
//bool CompareToolFileModel::copyFileToPath( QString sourceDir ,QString toDir, bool coverFileIfExist /*= true*/ ) const
//{
//	Q_D(const CompareToolFileModel);
//	toDir.replace("\\","/");
//	if (sourceDir == toDir){
//		return true;
//	}
//	if (!QFile::exists(sourceDir)){
//		return false;
//	}
//	
//	if (d->nameFilterDisables && !d->passNameFilters(sourceDir)) 
//	{
//		return false;
//	}
//
//	QDir * createfile     = new QDir;
//	bool exist = createfile->exists(toDir);
//	if (exist){
//		if(coverFileIfExist){
//			createfile->remove(toDir);
//		}
//	}
//	delete createfile;
//	if(!QFile::copy(sourceDir, toDir))
//	{
//		return false;
//	}
//	return true;
//}
//
////�����ļ��У�
//bool CompareToolFileModel::copyDirectoryFiles( const QString &fromDir, const QString &toDir, bool coverFileIfExist /*= true*/ ) const
//{
//	Q_D(const CompareToolFileModel);
//	QDir sourceDir(fromDir);
//	QDir targetDir(toDir);
//	if(!targetDir.exists()){    /**< ���Ŀ��Ŀ¼�����ڣ�����д��� */
//		if(!targetDir.mkdir(targetDir.absolutePath()))
//			return false;
//	}
//
//	QFileInfoList fileInfoList = sourceDir.entryInfoList();
//	foreach(QFileInfo fileInfo, fileInfoList){
//		if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
//			continue;
//
//		if(fileInfo.isDir()){    /**< ��ΪĿ¼ʱ���ݹ�Ľ���copy */
//			if(!copyDirectoryFiles(fileInfo.filePath(), 
//				targetDir.filePath(fileInfo.fileName()),
//				coverFileIfExist))
//				return false;
//		}
//		else{            /**< �������ǲ���ʱ�������ļ�����ɾ������ */
//
//			if (d->nameFilterDisables && !d->passNameFilters(fileInfo.fileName())) 
//			{
//				continue;
//			}
//
//			if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
//				targetDir.remove(fileInfo.fileName()); 
//			}
//
//			/// �����ļ�copy
//			if(!QFile::copy(fileInfo.filePath(), 
//				targetDir.filePath(fileInfo.fileName()))){
//					return false;
//			}
//		}
//	}
//	return true;
//}
//
//void  CompareToolFileModel::move( const QModelIndex &index, const QString & toDir, bool coverFileIfExist ) const
//{
//	Q_D(const CompareToolFileModel);
//	bool bResult = false;
//	QString path = filePath(index);
//	CompareToolFileModelPrivate::QFileSystemNode *indexNode = d->node(index);
//
//	if(indexNode->isDir())
//	{
//		bResult = copyDirectoryFiles(path, toDir, true);
//	}
//	if (d->nameFilterDisables && !d->passNameFilters(indexNode)) 
//	{
//		bResult = false;
//	}
//	else
//	{
//		bResult = copyFileToPath(path, toDir, true);
//	}
//	if (!bResult)
//	{
//		QMessageBox::warning(NULL, "����", "�ļ��ƶ����!");
//	}
//	else
//	{
//		QMessageBox::about(NULL, "��ʾ", "�ļ�ɾ����ɣ�");
//	}
//	//emit moveResult(bResult);
//}
