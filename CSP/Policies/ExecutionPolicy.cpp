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
    if(ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
        while(timeRelationIterator.hasNext())
        {
            timeRelationIterator.next();

            auto& curTimeRelationId = timeRelationIterator.key();
            auto& curTimeRelation = timeRelationIterator.value();

            auto initialMin = scenario.constraint(curTimeRelationId).duration.minDuration();
            auto initialMax = scenario.constraint(curTimeRelationId).duration.maxDuration();

            // - remove old stays
            curTimeRelation->removeStays();

            //add new stays
            curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_min == initialMin.msec(), kiwi::strength::strong));
            if(!initialMax.isInfinite())
                curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_max == initialMax.msec(), kiwi::strength::strong));
        }

        // time relations stays
        auto& scenario = *cspScenario->getScenario();

        //time node stays
        // - in timenodes :
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
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
            curCspTimeNode->addStay(new kiwi::Constraint(
                                        curCspTimeNode->m_date == initialDate.msec(),
                                        kiwi::strength::weak));
        }
    }
}

void ExecutionPolicy::computeDisplacement(const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements,
                                          Scenario::ElementsProperties& elementsProperties)
{
    try{
    if(ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        refreshStays(*cspScenario, elementsProperties, positionnedElements);
    }

    updateConstraints(m_scenario, positionnedElements, elementsProperties);

    }
    catch(...)
    {
        qDebug() << "failed here";
    }
}

void ExecutionPolicy::refreshStays(
        ScenarioModel& cspScenario,
        Scenario::ElementsProperties& elementsProperties,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
{
    auto& scenario = *cspScenario.getScenario();

    //time node stays
    // - in timenodes :
    QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario.m_timeNodes);
    while (timeNodeIterator.hasNext())
    {
        timeNodeIterator.next();


        auto& curTimeNodeId = timeNodeIterator.key();
        auto& curCspTimeNode = timeNodeIterator.value();
        if(positionnedElements.contains(curTimeNodeId)) // last positioned tn
        {
            if(!elementsProperties.timenodes.contains(curTimeNodeId))
            {
                auto initialDate = scenario.timeNode(curTimeNodeId).date();
                elementsProperties.timenodes[curTimeNodeId].newDate = initialDate;
            }

            curCspTimeNode->removeStays();
            curCspTimeNode->addStay(new kiwi::Constraint(
                                        curCspTimeNode->m_date ==  elementsProperties.timenodes[curTimeNodeId].newDate.msec(),
                                        kiwi::strength::required));

            auto& tn = m_scenario.timeNode(curTimeNodeId);
            auto states = Scenario::states(tn, scenario);
            for(auto stId : states)
            {
                auto& state = m_scenario.state(stId);
                if(state.status() == Scenario::ExecutionStatus::Disposed)
                {
                    if(auto& cstr = state.nextConstraint())
                        if(cspScenario.m_timeRelations.contains(cstr))
                            cspScenario.m_timeRelations[cstr]->removeAllConstraints();
                }
            }
        } // remove past constraints
        else if(!positionnedElements.contains(curTimeNodeId))
        {
            auto& tn = m_scenario.timeNode(curTimeNodeId);
            auto states = Scenario::states(tn, scenario);
            for(auto stId : states)
            {
                auto& state = m_scenario.state(stId);
                if(state.status() == Scenario::ExecutionStatus::Disposed)
                {
                    if(auto& cstr = state.nextConstraint())
                        if(cspScenario.m_timeRelations.contains(cstr))
                            cspScenario.m_timeRelations[cstr]->removeAllConstraints();
                }
            }
        }

    }

}

}
