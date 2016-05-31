#include "ExecutionPolicyFactory.hpp"

#include <CSP/Policies/ExecutionPolicy.hpp>

namespace CSP {

Scenario::CSPCoherencyCheckerInterface*ExecutionPolicyFactory::make(
        Scenario::ScenarioModel& scenario,
        const iscore::ApplicationContext& ctx,
        Scenario::ElementsProperties& elementsProperties)
{
    return new CSP::ExecutionPolicy{scenario, elementsProperties.timenodes.keys().toVector()};
}

}
