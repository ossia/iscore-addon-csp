#pragma once

#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Tools/dataStructures.hpp>

#include <Scenario/ExecutionChecker/CSPCoherencyCheckerInterface.hpp>

namespace CSP {

class ScenarioModel;

class ExecutionPolicy final : public CSPCoherencyCheckerInterface
{
    public:
        ExecutionPolicy() = default;

        ExecutionPolicy(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements);

        void
        computeDisplacement(
                Scenario::ScenarioModel& scenario,
                const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements,
                Scenario::ElementsProperties& elementsProperties) override;

    protected:
        static void refreshStays(ScenarioModel& cspScenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements);
};

}
