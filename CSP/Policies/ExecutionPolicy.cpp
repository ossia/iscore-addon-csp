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
        Scenario::ElementsProperties& elementsProperties):
    m_scenario(scenario),
    m_elementsProperties{elementsProperties}
{
    ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly);
    auto& solver = cspScenario->getSolver();
//    solver.reset(); // Reset all

    for(auto& cstr : m_scenario.constraints)
    {
        auto& curTimeRelationId = cstr.id();
        auto curTimeRelation = cspScenario->m_timeRelations.value(curTimeRelationId);
        auto& cstrProp = m_elementsProperties.constraints[curTimeRelationId];

        cstrProp.newMin = cstr.duration.minDuration();
        cstrProp.newMax = cstr.duration.maxDuration();

        // - remove old stays
        curTimeRelation->removeStays();
        curTimeRelation->restoreConstraints();

        auto& prevTimenodeModel = startTimeNode(cstr, m_scenario);
        auto& nextTimenodeModel = endTimeNode(cstr, m_scenario);

        auto prevCSPTimenode = cspScenario->m_timeNodes.value(prevTimenodeModel.id());
        auto nextCSPTimenode = cspScenario->m_timeNodes.value(nextTimenodeModel.id());

        curTimeRelation->addStay(new kiwi::Constraint{curTimeRelation->m_min >= cstrProp.newMin.msec()});
        curTimeRelation->addStay(new kiwi::Constraint{curTimeRelation->m_max <= cstrProp.newMax.msec()});

        curTimeRelation->addStay(new kiwi::Constraint{nextCSPTimenode->getDateMin() >= prevCSPTimenode->getDateMin() + curTimeRelation->m_min });
        curTimeRelation->addStay(new kiwi::Constraint{nextCSPTimenode->getDateMin() <= prevCSPTimenode->getDateMin() + curTimeRelation->m_max });

        curTimeRelation->addStay(new kiwi::Constraint{nextCSPTimenode->getDateMax() >= prevCSPTimenode->getDateMax() + curTimeRelation->m_min });
        curTimeRelation->addStay(new kiwi::Constraint{nextCSPTimenode->getDateMax() <= prevCSPTimenode->getDateMax() + curTimeRelation->m_max });

/*        curTimeRelation->addStay(new kiwi::Constraint{nextCSPTimenode->getDateMax() - nextCSPTimenode->getDateMin()
                                                                    <= prevCSPTimenode->getDateMax() - prevCSPTimenode->getDateMin()
                                                                    + curTimeRelation->m_max - curTimeRelation->m_min});
*/
    }
    for(auto& tn : cspScenario->m_timeNodes)
    {
        tn->removeStays();
        tn->restoreConstraints();
    }
}

bool ExecutionPolicy::computeDisplacement(const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements,
                                          Scenario::ElementsProperties& elementsProperties)
{
    qDebug() << "\n ***** New Computation *****";
    try
    {
        if(ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
        {
            if(!positionnedElements.empty())
            {
                auto& lastTnId = positionnedElements.back();
                auto tn = cspScenario->m_timeNodes.value(lastTnId);
                qDebug() << "fix " << lastTnId;
/*
                if(tn->getDateMax().value() - tn->getDateMin().value() < 1)
                {
                    // no need to recompute : we already knew its date
                    qDebug() << "bring no more info, skipping";
                    return false;
                }
*/
            }
            auto& solver = cspScenario->getSolver();

            /* ****************************
             * FIRST WAVE : take cstr as user said, fix tn
             * ****************************/

            QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();

                auto& curTimeRelationId = timeRelationIterator.key();
                auto& curTimeRelation = timeRelationIterator.value();

                auto& cstrProp = m_elementsProperties.constraints[curTimeRelationId];

                if(solver.hasEditVariable(curTimeRelation->m_min)) // we have to check, else kiwi crash half the time
                    solver.removeEditVariable(curTimeRelation->m_min);
                solver.addEditVariable(curTimeRelation->m_min, Strength::Strong);

                if(solver.hasEditVariable(curTimeRelation->m_max))
                    solver.removeEditVariable(curTimeRelation->m_max);
                solver.addEditVariable(curTimeRelation->m_max, Strength::Strong);

                solver.suggestValue(curTimeRelation->m_min, cstrProp.newMin.msec()); // if happened, cstrProp is up to date and min=max=real
                solver.suggestValue(curTimeRelation->m_max, cstrProp.newMax.msec());
            }
            auto dateMax = m_scenario.duration().msec();
            QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();

                auto& curCspTimeNode = timeNodeIterator.value();
                auto& curId = timeNodeIterator.key();

                if(curId != Id<Scenario::TimeNodeModel>(0))
                {
                    if(solver.hasEditVariable(curCspTimeNode->getDateMin()))
                        solver.removeEditVariable(curCspTimeNode->getDateMin());
                    solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Weak);

                    if(solver.hasEditVariable(curCspTimeNode->getDateMax()))
                        solver.removeEditVariable(curCspTimeNode->getDateMax());
                    solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Weak);

                    solver.suggestValue(curCspTimeNode->getDateMin(), 0.0);
                    solver.suggestValue(curCspTimeNode->getDateMax(), dateMax);
                }
                else
                {
                    //init with start Tn = 0

                    if(solver.hasEditVariable(curCspTimeNode->getDateMin()))
                        solver.removeEditVariable(curCspTimeNode->getDateMin());
                    solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Required);

                    if(solver.hasEditVariable(curCspTimeNode->getDateMax()))
                        solver.removeEditVariable(curCspTimeNode->getDateMax());
                    solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Required);

                    solver.suggestValue(curCspTimeNode->getDateMin(), 0.0);
                    solver.suggestValue(curCspTimeNode->getDateMax(), 0.0);
                }

                // - remove old stays
                curCspTimeNode->removeStays();
            }

            // ---------------------
            // First Computation

            qDebug() << "UPDATE";
            solver.updateVariables();
            qDebug() << "UPDATE OK";
            /* ****************************
             * SECOND WAVE : tn are now fixed, cstr updated
             * ****************************/

            timeNodeIterator.toFront();
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();

                auto& curTimeNodeId = timeNodeIterator.key();
                auto& curCspTimeNode = timeNodeIterator.value();
/*
                solver.removeEditVariable(curCspTimeNode->getDateMin());
                solver.removeEditVariable(curCspTimeNode->getDateMax());

                solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Required);
                solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Required);

                solver.suggestValue(curCspTimeNode->getDateMin(), curCspTimeNode->getDateMin().value());
                solver.suggestValue(curCspTimeNode->getDateMax(), curCspTimeNode->getDateMax().value());
//*/
                qDebug() << "tn " << curTimeNodeId << curCspTimeNode->m_date_min.value() << curCspTimeNode->m_date_max.value();

            }

            timeRelationIterator.toFront();
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();
                auto& curTimeRelation = timeRelationIterator.value();

                solver.removeEditVariable(curTimeRelation->m_min);
                solver.removeEditVariable(curTimeRelation->m_max);
            }

            // -----------------
            // Second Computation
//
//*
//            solver.updateVariables();

            timeNodeIterator.toFront();
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();
                auto& curCspTimeNode = timeNodeIterator.value();

                solver.removeEditVariable(curCspTimeNode->getDateMin());
                solver.removeEditVariable(curCspTimeNode->getDateMax());
            }
//*/
            timeRelationIterator.toFront();
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();

                auto& relationId = timeRelationIterator.key();
                auto& curTimeRelation = timeRelationIterator.value();
                auto& cstrProp = m_elementsProperties.constraints[relationId];

                cstrProp.newMin.setMSecs(curTimeRelation->m_min.value());
                cstrProp.newMax.setMSecs(curTimeRelation->m_max.value());

                qDebug() << "cstr " << relationId << cstrProp.newMin.msec() << cstrProp.newMax.msec();
            }
        }

    }
    catch(kiwi::InternalSolverError e)
    {
        qDebug() << "kiwi error : " << e.what();
    }
    return true;
}

void ExecutionPolicy::refreshStays(
        ScenarioModel& cspScenario,
        Scenario::ElementsProperties& elementsProperties,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
{

}

}
