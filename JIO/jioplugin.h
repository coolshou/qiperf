#ifndef JIOPLUGIN_H
#define JIOPLUGIN_H

#include "plugininterface.h"
#include <QObject>
#include <QAction>

class JioPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    // Declare that this class implements the PluginInterface
    Q_INTERFACES(PluginInterface)
    // This macro is used by Qt's plugin system to identify your plugin
    Q_PLUGIN_METADATA(IID "com.alphanetworks.JIO.PluginInterface/1.0" FILE "jioplugin.json")

public:
    explicit JioPlugin(QObject *parent = nullptr);
    ~JioPlugin() override = default;

    QString pluginName() const override;
    QString pluginDescription() const override;
    QString pluginVersion() const override;
    QMenu* createPluginMenu(QWidget* parent) override;
    void initialize() override;

private slots:
    void onMyActionTriggered();
};

#endif // JIOPLUGIN_H
