#include <CSP/Policies/FlexDisplacementPolicy.hpp>
#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>

#include <CSP/DisplacementComputer.hpp>

namespace CSP
{
FlexDisplacementPolicy::FlexDisplacementPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel> >& draggedElements)
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

void FlexDisplacementPolicy::computeDisplacement(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
        const TimeValue& deltaTime,
        Scenario::ElementsProperties& elementsProperties)
{
    compute(scenario, draggedElements, deltaTime, elementsProperties);
}

void FlexDisplacementPolicy::refreshStays(
        ScenarioModel& cspScenario,
        const QVector<Id<Scenario::TimeNodeModel> >& draggedElements)
{
    auto& scenario = *cspScenario.getScenario();
    // time relations stays
    QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario.m_timeRelations);
    while(timeRelationIterator.hasNext())
    {
        timeRelationIterator.next();

        auto& curTimeRelationId = timeRelationIterator.key();
        auto& curTimeRelation = timeRelationIterator.value();

        auto& curConstraint = scenario.constraint(curTimeRelationId);
        auto initialDefault = curConstraint.duration.defaultDuration().msec();

        // - remove old stays
        curTimeRelation->removeStays();

        //ad new stays
        // - if constraint preceed dragged element
        auto& endTimeNodeId = endTimeNode(curConstraint, scenario).id();
        auto endTimenode = cspScenario.m_timeNodes[endTimeNodeId];

        auto distanceFromMinToDate = initialDefault - curTimeRelation->m_iscoreMin.msec();
        auto distanceFromMinToMax = curTimeRelation->m_iscoreMax.msec() - curTimeRelation->m_iscoreMin.msec();

        auto& startTimeNodeId = startTimeNode(curConstraint, scenario).id();
        auto startTimenode = cspScenario.m_timeNodes[startTimeNodeId];

        // ensure than [min - max] interval stays the same
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_max == curTimeRelation->m_min + distanceFromMinToMax,
                                                     kiwi::strength::required));
        // Try to keep min and max around default duration
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_min  == endTimenode->m_date - startTimenode->m_date - distanceFromMinToDate,
                                                      kiwi::strength::weak));
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
        auto initialDate = cspScenario.getScenario()->timeNode(curTimeNodeId).date();

        // - remove old stays
        curCspTimeNode->removeStays();

        // - add new stays
        curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::medium));
    }
}
}
