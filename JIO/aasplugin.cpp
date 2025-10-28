#include "aasplugin.h"
#include <QDebug>
#include <QMessageBox>

AasPlugin::AasPlugin(QObject *parent)
    : QObject(parent)
{
}

QString AasPlugin::pluginName() const
{
    return "JIO";
}

QString AasPlugin::pluginDescription() const
{
    return "JIO project control tools";
}

QString AasPlugin::pluginVersion() const
{
    return "1.0.0";
}

QMenu* AasPlugin::createPluginMenu(QWidget* parent)
{
    QMenu* pluginMenu = new QMenu("Plugin Actions", parent);

    QAction* action1 = new QAction("Do Something 1", pluginMenu);
    connect(action1, &QAction::triggered, this, &AasPlugin::onMyActionTriggered);
    pluginMenu->addAction(action1);

    return pluginMenu;
}

QAction *AasPlugin::createPluginAction(QWidget *parent)
{
    QAction* action1 = new QAction(QIcon(":/AIP/JIO"), "JIO", parent);
    connect(action1, &QAction::triggered, this, &AasPlugin::onAction1Triggered);
    return action1;
}

void AasPlugin::initialize()
{
    qDebug() << "AasPlugin initialized!";
}

void AasPlugin::setConfig(QSettings *cfg)
{
    mCfg = cfg;
}

void AasPlugin::onMyActionTriggered()
{
    QMessageBox::information(nullptr, "Plugin Action", "Action 1 from " + pluginName() + " triggered!");
}

void AasPlugin::onAction1Triggered(bool checked)
{
    Q_UNUSED(checked)
    dlg_jio = new DlgJIO(mCfg);
    connect(this, &AasPlugin::destroyed, dlg_jio, &DlgJIO::close);
    dlg_jio->clearData();
    dlg_jio->show();
}
