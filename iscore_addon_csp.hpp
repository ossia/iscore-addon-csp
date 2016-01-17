#pragma once
#include <QObject>
#include <iscore/plugins/qt_interfaces/GUIApplicationContextPlugin_QtInterface.hpp>
#include <iscore/plugins/qt_interfaces/FactoryInterface_QtInterface.hpp>

/**
 * @brief The iscore_addon_csp class
 * In this plugin, we prefer the name time relation for what other call constraints.
 */
class iscore_addon_csp:
    public QObject,
    public iscore::GUIApplicationContextPlugin_QtInterface,
    public iscore::FactoryInterface_QtInterface
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID GUIApplicationContextPlugin_QtInterface_iid)
        Q_INTERFACES(
            iscore::GUIApplicationContextPlugin_QtInterface
            iscore::FactoryInterface_QtInterface
        )

    public:
        iscore_addon_csp();
        virtual ~iscore_addon_csp() = default;

        iscore::GUIApplicationContextPlugin* make_applicationPlugin(
                const iscore::ApplicationContext& app) override;

        std::vector<std::unique_ptr<iscore::FactoryInterfaceBase>> factories(
                const iscore::ApplicationContext&,
                const iscore::FactoryBaseKey& factoryName) const override;
};
