#pragma once

#include <Scenario/ExecutionChecker/CoherencyCheckerFactoryInterface.hpp>

namespace CSP
{
class ExecutionPolicyFactory  final : public Scenario::CoherencyCheckerFactoryInterface
{
        ISCORE_CONCRETE_FACTORY_DECL("d8410e26-774e-41e2-bfa9-37aedb8f8640")
    public:
             virtual Scenario::CSPCoherencyCheckerInterface* make(
                         Scenario::ScenarioModel& scenario,
                         const iscore::ApplicationContext& ctx,
                         Scenario::ElementsProperties& elementsProperties) override;
};
}
