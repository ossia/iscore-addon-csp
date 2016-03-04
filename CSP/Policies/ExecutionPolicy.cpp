#include "ExecutionPolicy.hpp"

#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>

#include <CSP/DisplacementComputer.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/Event/ExecutionStatus.hpp>

#include <limits>


namespace CSP
{
double Infinity = std::numeric_limits<double>::infinity();

ExecutionPolicy::ExecutionPolicy(Scenario::ScenarioModel& scenario, Scenario::ElementsProperties& elementsProperties):
    m_scenario{scenario}
{
    m_cspScenario = m_scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly);

    for(auto& cstr : m_scenario.constraints)
    {
        elementsProperties.constraints[cstr.id()].min = m_scenario.constraint(cstr.id()).duration.minDuration().msec();
        elementsProperties.constraints[cstr.id()].maxInfinite = m_scenario.constraint(cstr.id()).duration.maxDuration().isInfinite();
        if(!elementsProperties.constraints[cstr.id()].maxInfinite)
            elementsProperties.constraints[cstr.id()].max = m_scenario.constraint(cstr.id()).duration.maxDuration().msec();
        else
            elementsProperties.constraints[cstr.id()].max = Infinity;
    }

    m_tnToUpdate.insert(m_scenario.startTimeNode().id());

    elementsProperties.timenodes[m_scenario.startTimeNode().id()].newAbsoluteMax = 0;

    while(m_tnToUpdate.size() != 0 || m_waitingTn.size() != 0 || m_cstrToUpdateBack.size() != 0)
    {
        qDebug() << " ---- main loop ----";
        while(!m_tnToUpdate.empty())
        {
            qDebug() << " -- updateTn loop --" << m_tnToUpdate.size();
            auto n = m_tnToUpdate.size();
            for(int i = 0; i<n; i++)
            {
                qDebug() << "updating " << *m_tnToUpdate.begin();
                tnUpdated(elementsProperties);
            }
            m_tnToUpdate = m_tnNextStep;
            m_tnNextStep.clear();
        }
        auto n = m_waitingTn.size();
        for(int i = 0; i<n; i++)
        {
            qDebug() << "check waiting " << *m_waitingTn.begin();
            tnWaiting(elementsProperties);
        }
        n = m_cstrToUpdateBack.size();
        for(int i = 0; i<n; i++)
        {
            qDebug() << "backward " << *m_cstrToUpdateBack.begin();
            cstrUpdatedBackward(elementsProperties);
        }
    }

    qDebug() << "recap :";
    for(auto& cstr : m_scenario.constraints)
    {
        qDebug() << cstr.id() << elementsProperties.constraints[cstr.id()].min << elementsProperties.constraints[cstr.id()].max;
    }

    qDebug() << "fin init ************" ;
}

void ExecutionPolicy::computeDisplacement(const Id<Scenario::TimeNodeModel>& positionnedElement,
                                          Scenario::ElementsProperties& elementsProperties)
{
//*
    m_tnToUpdate.clear();
    m_pastTimeNodes.push_back(positionnedElement);
    m_tnToUpdate.insert(positionnedElement);

    qDebug() << "\n ***** new computation : TN" << positionnedElement << " *****";
    while(m_tnToUpdate.size() != 0 || m_waitingTn.size() != 0 || m_cstrToUpdateBack.size() != 0)
    {
        qDebug() << " ---- main loop ----";
        while(!m_tnToUpdate.empty())
        {
            qDebug() << " -- updateTn loop --";
            auto n = m_tnToUpdate.size();
            for(int i = 0; i<n; i++)
            {
                qDebug() << "updating " << *m_tnToUpdate.begin();
                tnUpdated(elementsProperties);
            }
            m_tnToUpdate = m_tnNextStep;
            m_tnNextStep.clear();
        }
        auto n = m_waitingTn.size();
        for(int i = 0; i<n; i++)
        {
            qDebug() << "check waiting " << *m_waitingTn.begin();
            tnWaiting(elementsProperties);
        }
        n = m_cstrToUpdateBack.size();
        for(int i = 0; i<n; i++)
        {
            qDebug() << "backward " << *m_cstrToUpdateBack.begin();
            cstrUpdatedBackward(elementsProperties);
        }
    }
//*/
}

void ExecutionPolicy::refreshStays(ScenarioModel& cspScenario,
                                   Scenario::ElementsProperties& elementsProperties,
                                   const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
{

}

void ExecutionPolicy::updateTnInit(Scenario::ElementsProperties& elementsProperties)
{

}

void ExecutionPolicy::tnUpdated(Scenario::ElementsProperties& elementsProperties)
{
    TimeNodeModel* timenode = m_cspScenario->m_timeNodes[*m_tnToUpdate.begin()];
    bool allTokens = true;

    // only the first tn has no prev cstr, so allTokens will be true in this case
    for(auto cstrId : timenode->prevConstraints())
    {
        if(!elementsProperties.constraints[cstrId].hasToken())
        {
            allTokens = false;

            break;
        }
    }

    auto& tnProperties = elementsProperties.timenodes[timenode->id()];

    /* If :
     * - any tn with all prev tokens
     * - first tn (= the only one with no prev cstr)
     * - happened tn (like last happened tn)
     * - tn already has a token (when back computing)
     */
    if(allTokens || m_pastTimeNodes.contains(timenode->id()) || tnProperties.hasToken())
    {
        /*
         * Update containers
         */
        auto it = m_waitingTn.find(timenode->id());
        if(it != m_waitingTn.end())
            m_waitingTn.erase(it); // tn is no more waiting
        m_tnToUpdate.erase(m_tnToUpdate.begin()); // tn is computed now


        /* **************************************************
         * INIT TIMENODE VALUES
         * - create a token
         * - compute new min and max in token
         * - update newAbsoluteMin/Max
         */

        if(!tnProperties.hasToken())
            tnProperties.token.state = Scenario::TokenState::Forward ;

        if(m_pastTimeNodes.contains(timenode->id())) // if happened we compute the delta to send in tokens
        {
            auto actualDate = tnProperties.dateMin; //OSSIA set it in timenode callback

            tnProperties.token.deltaMin = actualDate - tnProperties.newAbsoluteMin;
            tnProperties.token.deltaMax = actualDate - tnProperties.newAbsoluteMax;

        }

        else // tn not happened
        {
            if(tnProperties.token.state == Scenario::TokenState::Backward)
            {
                for(auto cstrId : timenode->nextConstraints())
                {
                    auto& cstrProp = elementsProperties.constraints[cstrId];
                    if(cstrProp.hasToken())
                    {
                        TimeRelationModel* cstr = m_cspScenario->m_timeRelations[cstrId];
                        if(cstrProp.token.deltaMax == -Infinity)
                            tnProperties.newAbsoluteMax =  elementsProperties.timenodes[cstr->endTn()].newAbsoluteMax - cstrProp.min;
                    }
                }
            }

            for(auto cstrId : timenode->prevConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                // take the more restrive deltaMin and deltaMax
                if(cstrProp.hasToken())
                {
                    qDebug() << "receive " << cstrProp.token.deltaMin<< cstrProp.token.deltaMax << " from " << cstrId;
                    tnProperties.token.deltaMin = std::max(tnProperties.token.deltaMin,
                                                            cstrProp.token.deltaMin);

                    tnProperties.token.deltaMax = std::min(tnProperties.token.deltaMax,
                                                            cstrProp.token.deltaMax);
                }

                /* = if init phase, init newAbsolute values */
                if(m_pastTimeNodes.size() == 0)
                {
                    auto constraint = m_cspScenario->m_timeRelations[cstrId];

                    tnProperties.newAbsoluteMin = std::max(
                            tnProperties.newAbsoluteMin,
                            elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin + cstrProp.min);

                    if(tnProperties.newAbsoluteMax == Infinity)
                        tnProperties.newAbsoluteMax = elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + cstrProp.max;
                    else if(elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + cstrProp.max != Infinity)
                        tnProperties.newAbsoluteMax = std::min(
                                tnProperties.newAbsoluteMax,
                                elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + cstrProp.max);

                }
                // clear old token
                if(cstrProp.token.state == Scenario::TokenState::Forward)
                {
                    cstrProp.token.disable();
                }
            }
        }

        // now apply this deltas to real absolute min and max
        // we check coherency before because tn can be previously correctly set by a backward action

        qDebug() << "tn before :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;
        if(tnProperties.token.deltaMax == -Infinity)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
        }
        else if(tnProperties.newAbsoluteMin + tnProperties.token.deltaMin <= tnProperties.newAbsoluteMax + tnProperties.token.deltaMax)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
            tnProperties.newAbsoluteMax += tnProperties.token.deltaMax;
        }
        qDebug() << "tn :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;

        /***************************************************
         * TRANSMIT TO CONSTRAINTS
         * assume a coherent system ?
         */

        // don't transmit nul deltas if execution started
        if(!m_pastTimeNodes.empty() && tnProperties.token.deltaMin == 0
                && tnProperties.token.deltaMax == 0)
            return;

        // previous constraints
        if(!m_pastTimeNodes.contains(timenode->id()))
        {
            for(auto cstrId : timenode->prevConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                auto& startTn = Scenario::startTimeNode(m_scenario.constraint(cstrId), m_scenario);

                // ------------------------------------------
                // if timenode min/max are more restrictive than this branch
                // token is send back
                auto deltaMin = tnProperties.newAbsoluteMin -
                                (cstrProp.min + elementsProperties.timenodes[startTn.id()].newAbsoluteMin);
                if(deltaMin > 0)
                {
                    cstrProp.token.state = Scenario::TokenState::Backward;
                    cstrProp.token.deltaMin = deltaMin;

                    m_cstrToUpdateBack.insert(cstrId);
                }

                if(tnProperties.newAbsoluteMax != Infinity) // if infinity, no constraint to send back
                {
                    auto deltaMax = tnProperties.newAbsoluteMax -
                                (cstrProp.max + elementsProperties.timenodes[startTn.id()].newAbsoluteMax);

                    if(deltaMax < 0)
                    {
                        cstrProp.token.state = Scenario::TokenState::Backward;
                        cstrProp.token.deltaMax = deltaMax;

                        m_cstrToUpdateBack.insert(cstrId);
                    }
                }
            }
        }

        // next constraints
        for(auto cstrId : timenode->nextConstraints())
        {
            auto& cstrProp = elementsProperties.constraints[cstrId];
            if(!cstrProp.hasToken())
            {
                cstrProp.token.state = Scenario::TokenState::Forward;
                cstrProp.token.deltaMin = tnProperties.token.deltaMin;
                cstrProp.token.deltaMax = tnProperties.token.deltaMax;

                auto& endTn = Scenario::endTimeNode(m_scenario.constraint(cstrId), m_scenario);
                qDebug() << "send " << cstrProp.token.deltaMin << cstrProp.token.deltaMax << " to " <<  endTn.id();
                m_tnNextStep.insert(endTn.id());
            }
            else if (cstrProp.token.state == Scenario::TokenState::Backward)
            {
                cstrProp.token.disable();
            }
        }

        // Clear timenode token
        tnProperties.token.disable();
    }
    else
    {
        // tn must wait
        m_waitingTn.insert(timenode->id());
    }

}

void ExecutionPolicy::tnWaiting(Scenario::ElementsProperties& elementsProperties)
{
    TimeNodeModel* timenode = m_cspScenario->m_timeNodes[*m_waitingTn.begin()];

    m_waitingTn.erase(m_waitingTn.begin()); // tn is no more waiting

    auto& tnProperties = elementsProperties.timenodes[timenode->id()];

    if(!tnProperties.hasToken())
        tnProperties.token.state = Scenario::TokenState::Forward;

    for(auto cstrId : timenode->prevConstraints())
    {
        if(elementsProperties.constraints[cstrId].hasToken())
        {
            tnProperties.token.deltaMin = std::max(
                                                    elementsProperties.constraints[cstrId].token.deltaMin,
                                                    tnProperties.token.deltaMin);

            tnProperties.token.deltaMax = std::min(
                                                    elementsProperties.constraints[cstrId].token.deltaMax,
                                                    tnProperties.token.deltaMax);
        }
    }

    m_tnToUpdate.insert(timenode->id());
}

void ExecutionPolicy::cstrUpdatedBackward(Scenario::ElementsProperties& elementsProperties)
{
    TimeRelationModel* constraint = m_cspScenario->m_timeRelations[*m_cstrToUpdateBack.begin()];
    m_cstrToUpdateBack.erase(m_cstrToUpdateBack.begin());

    auto& cstrProperties = elementsProperties.constraints[constraint->id()];

    auto delta = cstrProperties.max - cstrProperties.min;

    // if constraint is already playing, we have to update its min & max
    if(m_pastTimeNodes.contains(constraint->startTn()) || constraint->startTn() == m_cspScenario->getStartTimeNode()->id())
    {
        cstrProperties.min += cstrProperties.token.deltaMin;

        if(cstrProperties.token.deltaMax != -Infinity) // since startTn is fixed, token inf means constraintMax inf
            cstrProperties.max += cstrProperties.token.deltaMax;
        else // inf token : inf constraintMax
        {
            cstrProperties.max = elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin;
        }

        qDebug() << "cstr :" << constraint->id() << cstrProperties.min << cstrProperties.max;

        ISCORE_ASSERT(cstrProperties.min -200 <= cstrProperties.max); // TODO rustine
     }
    // if constraint min - max can not absorbe deltas, transmit to startTn
    else if(delta < cstrProperties.token.deltaMin ||
            delta < -cstrProperties.token.deltaMax)
    {
        auto& tnProp = elementsProperties.timenodes[constraint->startTn()];
        tnProp.token.state = Scenario::TokenState::Backward;
        tnProp.token.deltaMin = std::max(0., cstrProperties.token.deltaMin - delta);

        if(cstrProperties.token.deltaMax == -Infinity)
            tnProp.token.deltaMax = cstrProperties.token.deltaMax;
        else
            tnProp.token.deltaMax = std::min(0., cstrProperties.token.deltaMax + delta);

        m_waitingTn.insert(constraint->startTn());
    }
    else
    {
        qDebug() << constraint->id() << "aborsbing deltas";
/*        cstrProperties.min = std::max(cstrProperties.min,
                    elementsProperties.timenodes[constraint->endTn()].newAbsoluteMin - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax);
        if(cstrProperties.max == Infinity)
            cstrProperties.max = elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax
                                 - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin;
        else
            cstrProperties.max = std::min(cstrProperties.max,
                        elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin);
*/
    }
}


}


// old KIWI part
/*
namespace CSP
{
ExecutionPolicy::ExecutionPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements):
    m_scenario(scenario)
{
    // We set the system with i-score values
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
            QVector<Id<Scenario::StateModel>> states; //= Scenario::states(tn, scenario);
            for(auto& evId : tn.events())
            {
                auto& ev = scenario.event(evId);
                states.append(ev.states());
            }
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
            QVector<Id<Scenario::StateModel>> states; //= Scenario::states(tn, scenario);
            for(auto& evId : tn.events())
            {
                auto& ev = scenario.event(evId);
                states.append(ev.states());
            }
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
*/
