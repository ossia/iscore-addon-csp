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

}

void ExecutionPolicy::computeDisplacement(const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements,
                                          Scenario::ElementsProperties& elementsProperties)
{
    qDebug() << "\n ***** New Computation *****";
    try{
        if(ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
        {
            auto& solver = cspScenario->getSolver();

            if(!positionnedElements.empty())
            {
                auto& lastTnId = positionnedElements.back();
                auto tn = cspScenario->m_timeNodes.value(lastTnId);
                if(tn->getDateMax().value() - tn->getDateMin().value() < 1)
                {
                    // no need to recompute : we already knew its date
                    qDebug() << "bring no more info, skipping";
//                    return;
                }
            }

            QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();

                auto& curTimeRelationId = timeRelationIterator.key();
                auto& curTimeRelation = timeRelationIterator.value();

                auto initialMin = m_scenario.constraint(curTimeRelationId).duration.minDuration();
                auto initialMax = m_scenario.constraint(curTimeRelationId).duration.maxDuration();

                solver.addEditVariable(curTimeRelation->m_min, Strength::Required);
                solver.addEditVariable(curTimeRelation->m_max, Strength::Required);

                solver.suggestValue(curTimeRelation->m_min, initialMin.msec());
                solver.suggestValue(curTimeRelation->m_max, initialMax.msec());

                // - remove old stays
                curTimeRelation->removeStays();
            }

            // time relations stays
            auto maxDate = m_scenario.duration().msec();

            //time node stays
            // - in timenodes :
            QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();

                auto& timenodeId = timeNodeIterator.key();
                auto& curCspTimeNode = timeNodeIterator.value();
                // - remove old stays
                curCspTimeNode->removeStays();

                // - add new stays

                if(positionnedElements.contains(timenodeId) || timenodeId == Id<Scenario::TimeNodeModel>(0))
                {
                    solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Required);
                    solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Required);

                    solver.suggestValue(curCspTimeNode->getDateMin(), elementsProperties.timenodes[timenodeId].date);
                    solver.suggestValue(curCspTimeNode->getDateMax(), elementsProperties.timenodes[timenodeId].date);
                }
                else
                {
                    solver.addEditVariable( curCspTimeNode->getDateMax(), Strength::Weak);
                    solver.addEditVariable( curCspTimeNode->getDateMin(), Strength::Weak);

                    solver.suggestValue(curCspTimeNode->getDateMax(), maxDate);
                    solver.suggestValue(curCspTimeNode->getDateMin(), 0.0);
                }
            }

            solver.updateVariables();

            timeRelationIterator.toFront();
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();

                auto& relationId = timeRelationIterator.key();
                auto& curTimeRelation = timeRelationIterator.value();

                solver.removeEditVariable(curTimeRelation->m_min);
                solver.removeEditVariable(curTimeRelation->m_max);

                curTimeRelation->removeStays();

                auto& iscore_cstr = m_scenario.constraint(relationId);
                auto& startTnId = Scenario::startTimeNode(iscore_cstr, m_scenario).id();
                auto& endTnId = Scenario::endTimeNode(iscore_cstr, m_scenario).id();

                auto startCspTn = cspScenario->m_timeNodes.value(startTnId);
                auto endCspTn = cspScenario->m_timeNodes.value(endTnId);

                auto& cstrProp = elementsProperties.constraints[relationId];

                cstrProp.newMin.setMSecs(endCspTn->getDateMin().value() - startCspTn->getDateMax().value() - 1);
                cstrProp.newMax.setMSecs(endCspTn->getDateMax().value() - startCspTn->getDateMin().value() + 1);

                qDebug() << "cstr " << relationId << cstrProp.newMin.msec() << cstrProp.newMax.msec();
            }
            timeNodeIterator.toFront();
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();

                auto& curTimeNodeId = timeNodeIterator.key();
                auto& curCspTimeNode = timeNodeIterator.value();

                qDebug() << "tn " << curTimeNodeId << curCspTimeNode->m_date_min.value() << curCspTimeNode->m_date_max.value();

                // - remove old stays
                curCspTimeNode->removeStays();
                solver.removeEditVariable(curCspTimeNode->getDateMax());
                solver.removeEditVariable(curCspTimeNode->getDateMin());
            }
        }

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
                                        Strength::Required));

            auto& tn = m_scenario.timeNode(curTimeNodeId);
            auto states = Scenario::states(tn, m_scenario);
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
            auto states = Scenario::states(tn, m_scenario);
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
