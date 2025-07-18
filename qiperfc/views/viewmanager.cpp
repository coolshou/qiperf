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
    m_views = new QMap<QString, AbstractView *>;
    m_docks = new QMap<AbstractView *, QDockWidget *>;

    delete window->takeCentralWidget();
    window->setDockNestingEnabled(true);
    addView(m_throughputview, false);
}

ViewManager::~ViewManager()
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
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
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        view->loadConfig(config);
    }
}

void ViewManager::saveConfig(QSettings *config)
{
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        view->saveConfig(config);
    }
}

void ViewManager::loadSettings(QSettings *config)
{
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        view->loadSettings(config);
    }
}

void ViewManager::retranslate()
{
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        view->retranslate();
    }
}

void ViewManager::dispatchMessage(const QString &receiver, const QByteArray &message)
{
    AbstractView *sender = dynamic_cast<AbstractView *>(QObject::sender());
    // for (AbstractView *view : std::as_const(*m_views)) {
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        if (view->iid() == receiver || receiver.isEmpty()) {
            view->takeMessage(sender->iid(), message);
        }
    }
}

void ViewManager::receiveData(const QByteArray &array)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        if (view->isVisible()) {
            view->receiveData(array);
        }
    }
}

void ViewManager::setEnabled(bool enabled)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
        view->setEnabled(enabled);
    }
}

void ViewManager::clear(void)
{
    // for (AbstractView *view : std::as_const(*m_views)) {
    // for (AbstractView *view : *m_views) {
    for (auto key: m_views->keys()){
        AbstractView *view = m_views->value(key);
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
    // int idx = m_views->count();
    qint64 idx = m_views->keys().count();
    QString title = view->title();
    QDockWidget *dock = new QDockWidget(title, m_window);
    qDebug() << "ViewManager::addView dock title:" << dock->windowTitle();
    dock->installEventFilter(this);
    // connect(dock, &QDockWidget::visibilityChanged, this, &ViewManager::onVisibilityChanged);
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

    // m_views->append(view);
    m_views->insert(title, view);
    m_docks->insert(view, dock);
    // Access the tab widget
    // QTabWidget *tabWidget = findChild<QTabWidget *>();
    // if (tabWidget) {
    //     QTabBar *tabBar = tabWidget->tabBar();
    //     qDebug() << "tabBar:"  << QString::number(tabBar->count());
    //     // for (int i = 0; i < tabBar->count(); ++i) {
    //     //     QPushButton *closeButton = new QPushButton("×");
    //     //     closeButton->setFixedSize(16, 16);
    //     //     tabBar->setTabButton(i, QTabBar::RightSide, closeButton);

    //     //     connect(closeButton, &QPushButton::clicked, this, [=]() {
    //     //         QWidget *w = tabWidget->widget(i);
    //     //         if (w) {
    //     //             w->close();
    //     //         }
    //     //     });
    //     // }
    // }
}

void ViewManager::activateDock(AbstractView *view)
{
    if (m_docks->contains(view)){
        QDockWidget *dw = m_docks->value(view);
        if (!dw->isVisible()){
            dw->show();
        }
        dw->raise();
        // dw->setFocus();
        dw->setFocus();
        qInfo() << "activateDock";
        // view->setFocus();
    }else{
        qDebug() << "m_docks do not have " << view;
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

void ViewManager::close()
{
    // close all window
    // for (auto it = m_views->begin(); it != m_views->end(); /* don't increment here */) {
    //     auto view = m_views->value(it.key());
    //     if (m_docks->contains(view)){
    //         QDockWidget *dw = m_docks->value(view);
    //         dw->deleteLater();
    //         m_docks->remove(view);
    //     }
    //     view->close();
    //     view->deleteLater();
    //     // m_views->remove(it.key());
    //     it = m_views->erase(it);
    //     ++it;
    // }
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

void ViewManager::onVisibilityChanged(bool visible)
{
    // each time switch QDockWidget will also trigger this, not good for closeevent!!
    Q_UNUSED(visible)
    qDebug() << "onVisibilityChanged: " << visible;
}

bool ViewManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Close) {
        QDockWidget *dock = qobject_cast<QDockWidget*>(watched);
        if (dock) {
            // qDebug() << "QDockWidget receive close event!";
            QString key = dock->windowTitle();
            AbstractView *view = m_views->value(key);
            // //clear view
            view->close();
            view->deleteLater();
            dock->deleteLater();
            // m_views->erase()
            for (auto it = m_views->begin(); it != m_views->end(); /* don't increment here */) {
                if (it.key() == key) {
                    auto v = m_views->value(it.key());
                    for (auto itd = m_docks->begin(); itd != m_docks->end(); /* don't increment here */) {
                        if (itd.key() == v){
                            itd = m_docks->erase(itd);
                        }else{
                            ++itd;
                        }
                    }
                    it = m_views->erase(it);
                } else {
                    ++it;
                }
            }
            return true; // 事件已處理
        }
    }
    return QObject::eventFilter(watched, event);
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
