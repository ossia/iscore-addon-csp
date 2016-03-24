#include "ExecutionPolicyFactory.hpp"

#include <CSP/Policies/ExecutionPolicy.hpp>
#include <CSP/Settings/Model.hpp>

namespace CSP {

Scenario::CSPCoherencyCheckerInterface* ExecutionPolicyFactory::make(Scenario::ScenarioModel& scenario,
                                                                    const iscore::ApplicationContext& ctx,
                                                          Scenario::ElementsProperties& elementsProperties)
{
    if(ctx.settings<CSP::Settings::Model>().getExecutionMode() == Settings::ExecutionMode::Active)
        return new CSP::ExecutionPolicy{scenario, elementsProperties};

    return nullptr;
}

}
