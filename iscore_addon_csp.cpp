#include <iscore_addon_csp.hpp>
#include <CSP/CSPApplicationPlugin.hpp>
#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/MoveEventCSPFlexFactory.hpp>

#include <iscore/plugins/customfactory/FactorySetup.hpp>

iscore_plugin_csp::iscore_plugin_csp() :
    QObject {}
{
}

iscore::GUIApplicationContextPlugin* iscore_plugin_csp::make_applicationPlugin(
        const iscore::ApplicationContext& app)
{
    return new CSPApplicationPlugin{app};
}

std::vector<std::unique_ptr<iscore::FactoryInterfaceBase>>
iscore_plugin_csp::factories(
        const iscore::ApplicationContext& ctx,
        const iscore::FactoryBaseKey& key) const
{
    return instantiate_factories<
            iscore::ApplicationContext,
    TL<
        FW<Scenario::Command::MoveEventFactoryInterface,
            MoveEventCSPFactory,
            MoveEventCSPFlexFactory>
      >
    >(ctx, key);
}
