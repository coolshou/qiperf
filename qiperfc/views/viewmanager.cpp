#include "viewmanager.h"
#include <QSettings>
#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QDockWidget>
#include <QPluginLoader>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "abstractview.h"

ViewManager::ViewManager(QString *docPath, ThroughputView *tpview, QMainWindow *window) :
    m_docPath(docPath), m_throughputview(tpview), m_window(window)
{
    m_views = new QVector<AbstractView *>;
    m_docks = new QMap<AbstractView *, QDockWidget *>;

    delete window->takeCentralWidget();
    window->setDockNestingEnabled(true);

    // int index = 0;
    // QDockWidget *align = nullptr;
    // for (AbstractView *view : std::as_const(*m_views)) {
    //     addView(view, align, index);
    //     index++;
    // }
    addView(m_throughputview, false);
}

ViewManager::~ViewManager()
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        delete view;
    }
    delete m_views;
    // for (QDockWidget *dock : std::as_const(*m_docks)) {
    //     delete dock;
    // }
    // delete m_docks;
}

void ViewManager::loadConfig(QSettings *config)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        view->loadConfig(config);
    }
    //TODO: loadConfig of dock
}

void ViewManager::saveConfig(QSettings *config)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        view->saveConfig(config);
    }
    //TODO: saveConfig of dock
}

void ViewManager::loadSettings(QSettings *config)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        view->loadSettings(config);
    }
    //TODO: loadSettings of dock
}

void ViewManager::retranslate()
{
    for (int i = 0; i < m_views->size(); ++i) {
        AbstractView *view = m_views->at(i);
        view->retranslate();
    }
}

void ViewManager::dispatchMessage(const QString &receiver, const QByteArray &message)
{
    AbstractView *sender = dynamic_cast<AbstractView *>(QObject::sender());
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        if (view->iid() == receiver || receiver.isEmpty()) {
            view->takeMessage(sender->iid(), message);
        }
    }
}

void ViewManager::receiveData(const QByteArray &array)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        if (view->isVisible()) {
            view->receiveData(array);
        }
    }
}

void ViewManager::setEnabled(bool enabled)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        view->setEnabled(enabled);
    }
}

void ViewManager::clear(void)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    for (AbstractView *view : *m_views) {
        view->clear();
    }
}

void ViewManager::setFileAction(QAction *openAction, QAction *saveAction)
{
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
}

void ViewManager::addView(AbstractView *view, bool closeable)
{
    int idx = m_views->count();

    QDockWidget *dock = new QDockWidget(view->title(), m_window);
    if (!closeable){
        dock->setFeatures(dock->features() & ~QDockWidget::DockWidgetClosable &
                          ~QDockWidget::DockWidgetFloatable);
    }
    dock->setObjectName(view->iid());
    // dock->setFeatures(QDockWidget::AllDockWidgetFeatures);//deprecate
    dock->setWidget(view);
    if (idx) {
        // QDockWidget *align = m_views->at(idx);
        m_window->tabifyDockWidget(m_align, dock);
    } else {
        m_window->addDockWidget(Qt::LeftDockWidgetArea, dock);
        m_align = dock;
    }
    connect(view, &AbstractView::transmitData, this, &ViewManager::transmitData);
    connect(view, &AbstractView::sendMessage, this, &ViewManager::dispatchMessage);

    m_views->append(view);
    m_docks->insert(view, dock);
}

void ViewManager::activateDock(AbstractView *view)
{
    if (m_docks->contains(view)){
        m_docks->value(view)->raise();
    }
}

AbstractView* ViewManager::findActiveView()
{
    QWidget* wid = QApplication::focusWidget();
    AbstractView* view = nullptr;
    while (view == nullptr && wid != nullptr) {
        view = dynamic_cast<AbstractView *>(wid);
        if (view) {
            break; // its a AbstractView
        }
        wid = dynamic_cast<QWidget *>(wid->parent());
    }
    return view;
}

void ViewManager::saveFile()
{
    QString filter;
    AbstractView *view = findActiveView();
    if (view && !view->saveFileFilter().isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(view, tr("Save"), *m_docPath,
            view->saveFileFilter(), &filter,
            QFileDialog::HideNameFilterDetails);
        if (fileName.isEmpty()) {
            return;
        }
        *m_docPath = QFileInfo(fileName).path();
        view->saveFile(fileName, filter);
    } else {
        QMessageBox::warning(m_window, tr("Warning"),
                             tr("This view does not support this operation."));
    }
}

void ViewManager::openFile()
{
    QString filter;
    AbstractView *view = findActiveView();
    if (view && !view->openFileFilter().isEmpty()) {
        QString fileName = QFileDialog::getOpenFileName(view, tr("Open"), *m_docPath,
            view->openFileFilter(), &filter,
            QFileDialog::HideNameFilterDetails);
        if (fileName.isEmpty()) {
            return;
        }
        *m_docPath = QFileInfo(fileName).path();
        view->openFile(fileName, filter);
    } else {
        QMessageBox::warning(m_window, tr("Warning"),
                             tr("This view does not support this operation."));
    }
}

QVector<AbstractView *> ViewManager::loadExtensions(const QString &path)
{
    QDir dir(path);
    QVector<AbstractView *> list;
    QStringList filenames = dir.entryList(QDir::Files);
    // for (QString fileName : std::as_const(filenames)) {
    for (QString fileName : filenames) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        AbstractView *view = dynamic_cast<AbstractView *>(loader.instance());
        if (view) {
            qDebug() << "file:" << fileName;
            view->setParent(m_window);
            list.append(view);
        }
    }
    // for (QString fileName : dir.entryList(QStringList("*.js"), QDir::Files)) {
    //     QString name = dir.absoluteFilePath(fileName);
    //     AbstractView *view = new ScriptExtensionView(name, m_window);
    //     if (view) {
    //         list.append(view);
    //     }
    // }
    return list;
}
