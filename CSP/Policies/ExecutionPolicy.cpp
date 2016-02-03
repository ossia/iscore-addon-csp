#include "ExecutionPolicy.hpp"

#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>

namespace CSP
{
ExecutionPolicy::ExecutionPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        // add stays to all elements
        refreshStays(*cspScenario, positionnedElements);

    }else
    {
        ISCORE_BREAKPOINT;
        throw std::runtime_error("No CSP scenario found for this model");
    }
}

void ExecutionPolicy::refreshStays(
        ScenarioModel& cspScenario,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
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
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_min == initialMin.msec(), kiwi::strength::strong));
        if(!initialMax.isInfinite())
            curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_max == initialMax.msec(), kiwi::strength::strong));
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
        // already past timenodes don(t have to move
        if(positionnedElements.contains(curTimeNodeId))
            curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::required));

        curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::medium));
    }

}

}
