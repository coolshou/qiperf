#include "jioplugin.h"
#include <QDebug>
#include <QMessageBox>

JioPlugin::JioPlugin(QObject *parent)
    : QObject(parent)
{
}

QString JioPlugin::pluginName() const
{
    return "JIO";
}

QString JioPlugin::pluginDescription() const
{
    return "JIO project control tools";
}

QString JioPlugin::pluginVersion() const
{
    return "1.0.0";
}

QMenu* JioPlugin::createPluginMenu(QWidget* parent)
{
    QMenu* pluginMenu = new QMenu("Plugin Actions", parent);

    QAction* action1 = new QAction("Do Something 1", pluginMenu);
    connect(action1, &QAction::triggered, this, &JioPlugin::onMyActionTriggered);
    pluginMenu->addAction(action1);

    return pluginMenu;
}

QAction *JioPlugin::createPluginAction(QWidget *parent)
{
    QAction* action1 = new QAction(QIcon(":/AIP/JIO"), "JIO", parent);
    connect(action1, &QAction::triggered, this, &JioPlugin::onAction1Triggered);
    return action1;
}

void JioPlugin::initialize()
{
    qDebug() << "JioPlugin initialized!";
}

void JioPlugin::setConfig(QSettings *cfg)
{
    mCfg = cfg;
}

void JioPlugin::onMyActionTriggered()
{
    QMessageBox::information(nullptr, "Plugin Action", "Action 1 from " + pluginName() + " triggered!");
}

void JioPlugin::onAction1Triggered(bool checked)
{
    Q_UNUSED(checked)
    dlg_jio = new DlgJIO(mCfg);
    connect(this, &JioPlugin::destroyed, dlg_jio, &DlgJIO::close);
    dlg_jio->clearData();
    dlg_jio->show();
}
