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


namespace CSP
{

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
    }

    m_tnToUpdate.insert(m_scenario.startTimeNode().id());

    elementsProperties.timenodes[m_scenario.startTimeNode().id()].maxInfinite = false;
    elementsProperties.timenodes[m_scenario.startTimeNode().id()].dateMax = 0;

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

    qDebug() << "new computation";
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
        if(! elementsProperties.constraints[cstrId].token)
        {
            allTokens = false;

            break;
        }
    }

    /* If :
     * - any tn with all prev tokens
     * - first tn (= the only one with no prev cstr)
     * - happened tn (like last happened tn)
     * - tn already has a token (when back computing)
     */
    if(allTokens || m_pastTimeNodes.contains(timenode->id()) || elementsProperties.timenodes[timenode->id()].token)
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

        if(elementsProperties.timenodes[timenode->id()].token == nullptr)
            elementsProperties.timenodes[timenode->id()].token = new Scenario::Token{};

        if(m_pastTimeNodes.contains(timenode->id())) // if happened we compute the delta to send in tokens
        {
            auto actualDate = elementsProperties.timenodes[timenode->id()].dateMin; //OSSIA set it in timenode callback
            elementsProperties.timenodes[timenode->id()].token->deltaMin =
                    actualDate - elementsProperties.timenodes[timenode->id()].newAbsoluteMin;

            elementsProperties.timenodes[timenode->id()].token->deltaMax =
                    actualDate - elementsProperties.timenodes[timenode->id()].newAbsoluteMax;
        }

        else // tn not happened
        {
            for(auto cstrId : timenode->prevConstraints())
            {
                // take the more restrive deltaMin and deltaMax
                if(elementsProperties.constraints[cstrId].token)
                {
                    elementsProperties.timenodes[timenode->id()].token->deltaMin = std::max(
                                                                    elementsProperties.timenodes[timenode->id()].token->deltaMin,
                                                                    elementsProperties.constraints[cstrId].token->deltaMin);


                    qDebug() << "receive" << timenode->id() << elementsProperties.constraints[cstrId].token->deltaMax << elementsProperties.timenodes[timenode->id()].token->deltaMax;
                    elementsProperties.timenodes[timenode->id()].token->deltaMax = std::min(
                                                            elementsProperties.timenodes[timenode->id()].token->deltaMax,
                                                            elementsProperties.constraints[cstrId].token->deltaMax);

                }

                /* = if init phase, init newAbsolute values */
                if(m_pastTimeNodes.size() == 0)
                {
                    auto constraint = m_cspScenario->m_timeRelations[cstrId];
                    elementsProperties.timenodes[timenode->id()].newAbsoluteMin = std::max(
                            elementsProperties.timenodes[timenode->id()].newAbsoluteMin,
                            elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin + elementsProperties.constraints[cstrId].min);

                    // infinity is not a constraint
                    if(!elementsProperties.constraints[cstrId].maxInfinite && !elementsProperties.timenodes[constraint->startTn()].maxInfinite)
                    {
                        if(!elementsProperties.timenodes[timenode->id()].maxInfinite)
                        {
                            elementsProperties.timenodes[timenode->id()].newAbsoluteMax = std::min(
                                    elementsProperties.timenodes[timenode->id()].newAbsoluteMax,
                                    elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + elementsProperties.constraints[cstrId].max);
                        }
                        else
                        {
                            elementsProperties.timenodes[timenode->id()].maxInfinite = false;
                            elementsProperties.timenodes[timenode->id()].newAbsoluteMax =
                                    elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + elementsProperties.constraints[cstrId].max;
                        }
                    }
                }
                // clear old token
                if(elementsProperties.constraints[cstrId].token && !elementsProperties.constraints[cstrId].token->back)
                {
                    delete elementsProperties.constraints[cstrId].token;
                    elementsProperties.constraints[cstrId].token = nullptr;
                }
            }
        }

        // now apply this deltas to real absolute min and max
        elementsProperties.timenodes[timenode->id()].newAbsoluteMin += elementsProperties.timenodes[timenode->id()].token->deltaMin;
        elementsProperties.timenodes[timenode->id()].newAbsoluteMax += elementsProperties.timenodes[timenode->id()].token->deltaMax;

        qDebug() << "tn :" << timenode->id() << elementsProperties.timenodes[timenode->id()].newAbsoluteMin << elementsProperties.timenodes[timenode->id()].newAbsoluteMax;

        /***************************************************
         * TRANSMIT TO CONSTRAINTS
         * assume a coherent system ?
         */

        // don't transmit nul deltas if execution started
        if(!m_pastTimeNodes.empty() && elementsProperties.timenodes[timenode->id()].token->deltaMin == 0
                && elementsProperties.timenodes[timenode->id()].token->deltaMax == 0)
            return;

        // previous constraints
        if(!m_pastTimeNodes.contains(timenode->id()))
        {
            for(auto cstrId : timenode->prevConstraints())
            {
                auto& startTn = Scenario::startTimeNode(m_scenario.constraint(cstrId), m_scenario);

                // ------------------------------------------
                // if timenode min/max are more restrictive than this branch
                // token is send back
                auto deltaMin = elementsProperties.timenodes[timenode->id()].newAbsoluteMin -
                                (elementsProperties.constraints[cstrId].min + elementsProperties.timenodes[startTn.id()].newAbsoluteMin);
                if(deltaMin > 110)
                {
                    elementsProperties.constraints[cstrId].token = new Scenario::Token{};
                    elementsProperties.constraints[cstrId].token->back = true;
                    elementsProperties.constraints[cstrId].token->deltaMin = deltaMin;

                    m_cstrToUpdateBack.insert(cstrId);
                }

                auto deltaMax = elementsProperties.timenodes[timenode->id()].newAbsoluteMax -
                                (elementsProperties.constraints[cstrId].max + elementsProperties.timenodes[startTn.id()].newAbsoluteMax);
                if(deltaMax < -110)
                {
                    if(!elementsProperties.constraints[cstrId].token)
                        elementsProperties.constraints[cstrId].token = new Scenario::Token{};

                    elementsProperties.constraints[cstrId].token->back = true;
                    elementsProperties.constraints[cstrId].token->deltaMax = deltaMax;

                    m_cstrToUpdateBack.insert(cstrId);
                }
            }
        }

        // next constraints
        for(auto cstrId : timenode->nextConstraints())
        {
            if(!elementsProperties.constraints[cstrId].token)
            {
                elementsProperties.constraints[cstrId].token = new Scenario::Token{};
                elementsProperties.constraints[cstrId].token->deltaMin = elementsProperties.timenodes[timenode->id()].token->deltaMin;
                elementsProperties.constraints[cstrId].token->deltaMax = elementsProperties.timenodes[timenode->id()].token->deltaMax;

                auto& endTn = Scenario::endTimeNode(m_scenario.constraint(cstrId), m_scenario);
                m_tnNextStep.insert(endTn.id());
            }
            else if (elementsProperties.constraints[cstrId].token->back)
            {
                delete elementsProperties.constraints[cstrId].token;
                elementsProperties.constraints[cstrId].token = nullptr;// TODO
            }
        }

        // Clear timenode token

        delete elementsProperties.timenodes[timenode->id()].token;
        elementsProperties.timenodes[timenode->id()].token = nullptr;
    }
    else
    {
        // tn must wait
        m_waitingTn.insert(timenode->id());
    }

}

void ExecutionPolicy::tnWaiting(Scenario::ElementsProperties& elementsProperties)
{
    TimeNodeModel* timenode{};

    timenode = m_cspScenario->m_timeNodes[*m_waitingTn.begin()];
    m_waitingTn.erase(m_waitingTn.begin()); // tn is no more waiting

    if(elementsProperties.timenodes[timenode->id()].token == nullptr)
        elementsProperties.timenodes[timenode->id()].token = new Scenario::Token{};

    for(auto cstrId : timenode->prevConstraints())
    {
        if(elementsProperties.constraints[cstrId].token != nullptr)
        {
            elementsProperties.timenodes[timenode->id()].token->deltaMin = std::max(
                                                    elementsProperties.constraints[cstrId].token->deltaMin,
                                                    elementsProperties.timenodes[timenode->id()].token->deltaMin);

            elementsProperties.timenodes[timenode->id()].token->deltaMax = std::min(
                                                    elementsProperties.constraints[cstrId].token->deltaMax,
                                                    elementsProperties.timenodes[timenode->id()].token->deltaMax);
        }
    }

    m_tnToUpdate.insert(timenode->id());
}

void ExecutionPolicy::cstrUpdatedBackward(Scenario::ElementsProperties& elementsProperties)
{
    TimeRelationModel* constraint = m_cspScenario->m_timeRelations[*m_cstrToUpdateBack.begin()];
    m_cstrToUpdateBack.erase(m_cstrToUpdateBack.begin());

    auto delta = elementsProperties.constraints[constraint->id()].max - elementsProperties.constraints[constraint->id()].min;

    // if constraint is already playing, we have to update its min & max
    if(m_pastTimeNodes.contains(constraint->startTn()) || constraint->startTn() == m_cspScenario->getStartTimeNode()->id())
    {
        elementsProperties.constraints[constraint->id()].min += elementsProperties.constraints[constraint->id()].token->deltaMin;
        elementsProperties.constraints[constraint->id()].max += elementsProperties.constraints[constraint->id()].token->deltaMax;

        qDebug() << "cstr :" << constraint->id() << elementsProperties.constraints[constraint->id()].min << elementsProperties.constraints[constraint->id()].max;
        ISCORE_ASSERT(elementsProperties.constraints[constraint->id()].min <= elementsProperties.constraints[constraint->id()].max);
     }
    // if constraint min - max can not absorbe deltas, transmit to startTn
    else if(delta < elementsProperties.constraints[constraint->id()].token->deltaMin ||
            delta < -elementsProperties.constraints[constraint->id()].token->deltaMin)
    {
        elementsProperties.timenodes[constraint->startTn()].token = new Scenario::Token{};
        elementsProperties.timenodes[constraint->startTn()].token->deltaMin = std::max(0., elementsProperties.constraints[constraint->id()].token->deltaMin - delta);
        elementsProperties.timenodes[constraint->startTn()].token->deltaMax = std::min(0., elementsProperties.constraints[constraint->id()].token->deltaMax + delta);

        m_waitingTn.insert(constraint->startTn());
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
