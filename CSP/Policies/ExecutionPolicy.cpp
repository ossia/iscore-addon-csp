#include "ExecutionPolicy.hpp"

#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>

#include <CSP/DisplacementComputer.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/Event/ExecutionStatus.hpp>

namespace CSP
{
ExecutionPolicy::ExecutionPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements):
    m_scenario(scenario)
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

void ExecutionPolicy::computeDisplacement(const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements,
                                          Scenario::ElementsProperties& elementsProperties)
{
    try{
    if(ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        refreshStays(*cspScenario, positionnedElements);
    }
    }
    catch(...)
    {qDebug() << "failed to init solver";}

    updateConstraints(m_scenario, positionnedElements, elementsProperties);

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

        auto& startSt =  Scenario::startState(scenario.constraint(curTimeRelationId), scenario);

        if (startSt.status() == Scenario::ExecutionStatus::Disposed)
        {
            curTimeRelation->removeAllConstraints();
            continue;
        }
        // - remove old stays
        curTimeRelation->removeStays();

        //add new stays
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

        curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::weak));
    }

}

}
