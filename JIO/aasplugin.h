#ifndef AASPLUGIN_H
#define AASPLUGIN_H

#include "plugininterface.h"
#include <QObject>
#include <QAction>
#include <QSettings>

#include "dlgjio.h"

class AasPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    // Declare that this class implements the PluginInterface
    Q_INTERFACES(PluginInterface)
    // This macro is used by Qt's plugin system to identify your plugin
    Q_PLUGIN_METADATA(IID "com.alphanetworks.JIO.PluginInterface/1.0" FILE "AasPlugin.json")

public:
    explicit AasPlugin(QObject *parent = nullptr);
    ~AasPlugin() override = default;

    QString pluginName() const override;
    QString pluginDescription() const override;
    QString pluginVersion() const override;
    QMenu* createPluginMenu(QWidget* parent) override;
    QAction *createPluginAction(QWidget* parent) override;
    void initialize() override;
    void setConfig(QSettings *cfg) override;
private slots:
    void onMyActionTriggered();
    void onAction1Triggered(bool checked);
private:
    QSettings *mCfg;
    DlgJIO *dlg_jio;
};

#endif // AASPLUGIN_H
