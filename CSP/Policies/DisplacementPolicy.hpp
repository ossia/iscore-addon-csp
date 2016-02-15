#pragma once

#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Tools/dataStructures.hpp>
#include <Scenario/Process/Algorithms/StandardDisplacementPolicy.hpp>

/* *****************************************************
 * This Displacement Policy allow to move
 * timenodes only inside the [min max] interval
 * *****************************************************/

namespace CSP
{
class ScenarioModel;

class DisplacementPolicy
{
    public:
        static void init(
                Scenario::ScenarioModel& scenario,
                const QVector<Id<Scenario::TimeNodeModel>>& draggedElements);

        static void computeDisplacement(
                Scenario::ScenarioModel& scenario,
                const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
                const TimeValue& deltaTime,
                Scenario::ElementsProperties& elementsProperties);

        static QString name() // TODO : usefull ?
        {
            return QString{"CSP"};
        }

        template<typename... Args>
        static void updatePositions(Args&&... args)
        {
            Scenario::CommonDisplacementPolicy::updatePositions(std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void revertPositions(Args&&... args)
        {
            Scenario::CommonDisplacementPolicy::revertPositions(std::forward<Args>(args)...);
        }

    protected:
        static void refreshStays(
                ScenarioModel& cspScenario,
                const QVector<Id<Scenario::TimeNodeModel> >& draggedElements);
};
}
