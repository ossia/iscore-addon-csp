#pragma once

#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Tools/dataStructures.hpp>

namespace CSP {

class ScenarioModel;

class ExecutionPolicy
{
    public:
        ExecutionPolicy() = default;

        ExecutionPolicy(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements);

        static
        void
        computeDisplacement(
                Scenario::ScenarioModel& scenario,
                const QVector<Id<Scenario::TimeNodeModel>>& positionnedElements,
                Scenario::ElementsProperties& elementsProperties);

    protected:
        static void refreshStays(ScenarioModel& cspScenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements);
};

}
