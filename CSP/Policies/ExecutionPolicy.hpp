#pragma once

#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Tools/dataStructures.hpp>

#include <Scenario/ExecutionChecker/CSPCoherencyCheckerInterface.hpp>

namespace CSP {

class ScenarioModel;

class ExecutionPolicy final : public Scenario::CSPCoherencyCheckerInterface
{
    public:
        ExecutionPolicy() = default;

        ExecutionPolicy(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements);

        void
        computeDisplacement(
                const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements,
                Scenario::ElementsProperties& elementsProperties) override;

    protected:
        void refreshStays(ScenarioModel& cspScenario,
                                 Scenario::ElementsProperties& elementsProperties,
                                 const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements);

    private:
        Scenario::ScenarioModel& m_scenario;
};

}
