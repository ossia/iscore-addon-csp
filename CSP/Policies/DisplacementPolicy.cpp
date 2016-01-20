#include <CSP/Policies/DisplacementPolicy.hpp>
#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <CSP/DisplacementComputer.hpp>

namespace CSP
{
DisplacementPolicy::DisplacementPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel>>& draggedElements)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        // add stays to all elements
        refreshStays(*cspScenario, draggedElements);

    }else
    {
        ISCORE_BREAKPOINT;
        throw std::runtime_error("No CSP scenario found for this model");
    }
}

void DisplacementPolicy::computeDisplacement(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
        const TimeValue& deltaTime,
        Scenario::ElementsProperties& elementsProperties)
{
    compute(scenario, draggedElements, deltaTime, elementsProperties);
}

void DisplacementPolicy::refreshStays(
        ScenarioModel& cspScenario,
        const QVector<Id<Scenario::TimeNodeModel> >& draggedElements)
{
    // time relations stays
    auto& scenario = *cspScenario.getScenario();
    QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario.m_timeRelations);
    while(timeRelationIterator.hasNext())
    {
        timeRelationIterator.next();

        auto& curTimeRelationId = timeRelationIterator.key();
        auto& curTimeRelation = timeRelationIterator.value();

        auto initialMin = scenario.constraint(curTimeRelationId).duration.minDuration();
        auto initialMax = scenario.constraint(curTimeRelationId).duration.maxDuration();

        // - remove old stays
        curTimeRelation->removeStays();

        //ad new stays
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_min == initialMin.msec(), kiwi::strength::required));
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_max == initialMax.msec(), kiwi::strength::required));
    }

    //time node stays
    // - in timenodes :
    QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario.m_timeNodes);
    while (timeNodeIterator.hasNext())
    {
        timeNodeIterator.next();

        auto& curTimeNodeId = timeNodeIterator.key();
        auto& curCspTimeNode = timeNodeIterator.value();

        // try to stay on initial value
        auto initialDate = scenario.timeNode(curTimeNodeId).date();

        // - remove old stays
        curCspTimeNode->removeStays();

        // - add new stays
        curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::medium));
    }
}
}
