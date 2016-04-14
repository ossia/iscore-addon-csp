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
    solver.reset(); // Reset all

    for(auto& cstr : m_scenario.constraints)
    {
        auto& curTimeRelationId = cstr.id();
        auto curTimeRelation = cspScenario->m_timeRelations.value(curTimeRelationId);
        auto& cstrProp = m_elementsProperties.constraints[curTimeRelationId];

        cstrProp.newMin = cstr.duration.minDuration();
        cstrProp.newMax = cstr.duration.maxDuration();

        // - restore constraints
        curTimeRelation->restoreConstraints();
        curTimeRelation->removeStays();

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
    }
    for(auto& tn : cspScenario->m_timeNodes)
    {
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
                qDebug() << "fix " << lastTnId;
            }
            auto& solver = cspScenario->getSolver();
//            solver.reset();

            /* ****************************
             * FIRST WAVE : take cstr as user said, compute TN values
             * ****************************/

            QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();

                auto& curTimeRelationId = timeRelationIterator.key();
                auto& curTimeRelation = timeRelationIterator.value();
                auto& cstrProp = m_elementsProperties.constraints[curTimeRelationId];

                if(cstrProp.status == Scenario::ExecutionStatus::Happened) // cstr bring no more info
                {
                    curTimeRelation->removeAllConstraints();
                    continue;
                }

                // Restore correctly all constraints
                curTimeRelation->restoreConstraints();
                for(auto s : curTimeRelation->m_stays) // we have to restore stays, because of solver.reset()
                {
                    if(!solver.hasConstraint(*s))
                        solver.addConstraint(*s);
                }

                // Add edit variables
                solver.addEditVariable(curTimeRelation->m_min, Strength::Required);
                solver.addEditVariable(curTimeRelation->m_max, Strength::Required);

                // Suggest new values
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

                // Check constraints
                curCspTimeNode->restoreConstraints();

                // Add variables and suggest values
                if(curId != Id<Scenario::TimeNodeModel>(0) && !positionnedElements.contains(curId))
                {
                    solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Weak);
                    solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Weak);

                    solver.suggestValue(curCspTimeNode->getDateMin(), 0.0);
                    solver.suggestValue(curCspTimeNode->getDateMax(), dateMax);
                }
                else
                {
                    //init with start Tn = 0
                    solver.addEditVariable(curCspTimeNode->getDateMin(), Strength::Required);
                    solver.addEditVariable(curCspTimeNode->getDateMax(), Strength::Required);

                    solver.suggestValue(curCspTimeNode->getDateMin(), m_elementsProperties.timenodes[curId].date_min);
                    solver.suggestValue(curCspTimeNode->getDateMax(), m_elementsProperties.timenodes[curId].date_max);
                }
            }

            // ---------------------
            // First Computation

            qDebug() << "updating ...";
            solver.updateVariables();
            qDebug() << "UPDATE OK !";


            /* ****************************
             * SECOND WAVE : tn are now fixed, we can update constraints
             * ****************************/


            // some clean up

            timeNodeIterator.toFront();
            while (timeNodeIterator.hasNext())
            {
                timeNodeIterator.next();

                auto& curTimeNodeId = timeNodeIterator.key();
                auto& curCspTimeNode = timeNodeIterator.value();

                solver.removeEditVariable(curCspTimeNode->getDateMin());
                solver.removeEditVariable(curCspTimeNode->getDateMax());

                qDebug() << "tn " << curTimeNodeId << curCspTimeNode->m_date_min.value() << curCspTimeNode->m_date_max.value();

                m_elementsProperties.timenodes[curTimeNodeId].date_min = curCspTimeNode->m_date_min.value();
                m_elementsProperties.timenodes[curTimeNodeId].date_max = curCspTimeNode->m_date_max.value();
            }

            timeRelationIterator.toFront();
            while(timeRelationIterator.hasNext())
            {
                timeRelationIterator.next();
                auto& curTimeRelation = timeRelationIterator.value();
                auto& cstrId = timeRelationIterator.key();

                // REMOVE OLD
                solver.removeEditVariable(curTimeRelation->m_min);
                solver.removeEditVariable(curTimeRelation->m_max);

// Uncomment to try to compute Constraint Duration
//*

                // ADD NEW var and constraints
                auto& cstr = m_scenario.constraint(cstrId);
                auto& prevTimenodeModel = startTimeNode(cstr, m_scenario);
                auto& nextTimenodeModel = endTimeNode(cstr, m_scenario);

                auto prevCSPTimenode = cspScenario->m_timeNodes.value(prevTimenodeModel.id());
                auto nextCSPTimenode = cspScenario->m_timeNodes.value(nextTimenodeModel.id());

                auto c1 = kiwi::Constraint{curTimeRelation->m_min  == nextCSPTimenode->m_date_min - prevCSPTimenode->m_date_max};
                auto c2 = kiwi::Constraint{curTimeRelation->m_max  == nextCSPTimenode->m_date_max - prevCSPTimenode->m_date_min};

                solver.addConstraint(c1);
                solver.addConstraint(c2);

                solver.addEditVariable(nextCSPTimenode->m_date_min, Strength::Strong);
                solver.addEditVariable(nextCSPTimenode->m_date_max, Strength::Strong);
                solver.addEditVariable(prevCSPTimenode->m_date_min, Strength::Weak);
                solver.addEditVariable(prevCSPTimenode->m_date_max, Strength::Weak);

                solver.suggestValue(nextCSPTimenode->m_date_min, m_elementsProperties.timenodes[nextTimenodeModel.id()].date_min);
                solver.suggestValue(nextCSPTimenode->m_date_max, m_elementsProperties.timenodes[nextTimenodeModel.id()].date_max);
                solver.suggestValue(prevCSPTimenode->m_date_min, m_elementsProperties.timenodes[prevTimenodeModel.id()].date_min);
                solver.suggestValue(prevCSPTimenode->m_date_max, m_elementsProperties.timenodes[prevTimenodeModel.id()].date_max);

                // UPDATE VAR
                solver.updateVariables();

                // REMOVE
                solver.removeConstraint(c1);
                solver.removeConstraint(c2);

                solver.removeEditVariable(nextCSPTimenode->m_date_min);
                solver.removeEditVariable(nextCSPTimenode->m_date_max);
                solver.removeEditVariable(prevCSPTimenode->m_date_min);
                solver.removeEditVariable(prevCSPTimenode->m_date_max);

                // UPDATE Relation
                auto& cstrProp = m_elementsProperties.constraints[cstrId];

                auto min = std::max(cstrProp.newMin.msec(), curTimeRelation->m_min.value());
                auto max = std::min(cstrProp.newMax.msec(), curTimeRelation->m_max.value());
                cstrProp.newMin.setMSecs(min);
                cstrProp.newMax.setMSecs(max);
                qDebug() << "cstr " << cstrId << cstrProp.newMin.msec() << cstrProp.newMax.msec();
    //*/
            }
        }
    }
    catch(kiwi::InternalSolverError e)
    {
        qDebug() << "kiwi error : " << e.what();
        ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly);
        auto& solver = cspScenario->getSolver();
        solver.reset();
    }
    catch(...)
    {
        qDebug() << "kiwi error";
        ScenarioModel* cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly);
        auto& solver = cspScenario->getSolver();
        solver.reset();
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
