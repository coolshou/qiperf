#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin> // Required for Q_DECLARE_INTERFACE
#include <QString>
#include <QMenu> // If your plugin will add menus directly

class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    // Common plugin methods
    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
    virtual QString pluginVersion() const = 0;

    // Method to integrate with the main application's menu
    virtual QMenu* createPluginMenu(QWidget* parent) = 0; // Or return a list of QActions, etc.
    virtual void initialize() = 0; // For any setup the plugin needs

};

// Declare the interface to Qt's meta-object system
// The second argument is a unique identifier for your interface
Q_DECLARE_INTERFACE(PluginInterface, "com.yourcompany.YourApp.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
