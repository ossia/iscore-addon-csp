#include <iscore_addon_csp.hpp>
#include <CSP/CSPApplicationPlugin.hpp>
#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/MoveEventCSPFlexFactory.hpp>

#include <iscore/plugins/customfactory/FactorySetup.hpp>

iscore_addon_csp::iscore_addon_csp() :
    QObject {}
{
}

iscore::GUIApplicationContextPlugin* iscore_addon_csp::make_applicationPlugin(
        const iscore::ApplicationContext& app)
{
    return new CSP::CSPApplicationPlugin{app};
}

std::vector<std::unique_ptr<iscore::FactoryInterfaceBase>>
iscore_addon_csp::factories(
        const iscore::ApplicationContext& ctx,
        const iscore::FactoryBaseKey& key) const
{
    return instantiate_factories<
            iscore::ApplicationContext,
    TL<
        FW<Scenario::Command::MoveEventFactoryInterface,
            CSP::MoveEventCSPFactory,
            CSP::MoveEventCSPFlexFactory>
      >
    >(ctx, key);
}
