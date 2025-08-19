#include "jioplugin.h"
#include <QDebug>
#include <QMessageBox>

JioPlugin::JioPlugin(QObject *parent)
    : QObject(parent)
{
}

QString JioPlugin::pluginName() const
{
    return "My Sample Plugin";
}

QString JioPlugin::pluginDescription() const
{
    return "A simple plugin that adds an item to the menu.";
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

    QAction* action2 = new QAction("Do Something 2", pluginMenu);
    connect(action2, &QAction::triggered, [this]() {
        QMessageBox::information(nullptr, "Plugin Action", "Action 2 from " + pluginName() + " triggered!");
    });
    pluginMenu->addAction(action2);

    return pluginMenu;
}

void JioPlugin::initialize()
{
    qDebug() << "JioPlugin initialized!";
}

void JioPlugin::onMyActionTriggered()
{
    QMessageBox::information(nullptr, "Plugin Action", "Action 1 from " + pluginName() + " triggered!");
}
