#include <iscore_addon_csp.hpp>
#include <CSP/ApplicationPlugin.hpp>
#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/MoveEventCSPFlexFactory.hpp>
#include <CSP/ExecutionPolicyFactory.hpp>
#include <CSP/Settings/Factory.hpp>
#include <iscore/plugins/customfactory/FactorySetup.hpp>

iscore_addon_csp::iscore_addon_csp() :
    QObject {}
{
}

iscore_addon_csp::~iscore_addon_csp()
{

}

iscore::GUIApplicationContextPlugin* iscore_addon_csp::make_applicationPlugin(
        const iscore::ApplicationContext& app)
{
    return new CSP::ApplicationPlugin{app};
}

std::vector<std::unique_ptr<iscore::FactoryInterfaceBase>>
iscore_addon_csp::factories(
        const iscore::ApplicationContext& ctx,
        const iscore::AbstractFactoryKey& key) const
{
    return instantiate_factories<
            iscore::ApplicationContext,
    TL<
        FW<Scenario::Command::MoveEventFactoryInterface,
            CSP::MoveEventCSPFactory,
            CSP::MoveEventCSPFlexFactory>,
        FW<iscore::SettingsDelegateFactory,
            CSP::Settings::Factory>,
        FW<Scenario::CoherencyCheckerFactoryInterface,
            CSP::ExecutionPolicyFactory>
      >
    >(ctx, key);
}

iscore::Version iscore_addon_csp::version() const
{
    return iscore::Version{1};
}

UuidKey<iscore::Plugin> iscore_addon_csp::key() const
{
    return "8953366a-947c-453c-9e8e-395e0e39be25";
}
