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


    elementsProperties.timenodes[m_scenario.startTimeNode().id()].newAbsoluteMax = 0;

    bool b = false;

    do
    {
        qDebug() << "****** new ********* ";
        m_tnToUpdate.insert(m_scenario.startTimeNode().id());
        b = false;
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
                    updateTnInit(elementsProperties);
                    if(!m_cstrToUpdateBack.empty())
                        break;
                }
                if(!m_cstrToUpdateBack.empty())
                {
                    m_tnToUpdate.clear();
                    m_tnNextStep.clear();
                    break;
                }

                m_tnToUpdate = m_tnNextStep;
                m_tnNextStep.clear();
            }
/*            auto n = m_waitingTn.size();
            for(int i = 0; i<n; i++)
            {
                qDebug() << "check waiting " << *m_waitingTn.begin();
                tnWaiting(elementsProperties);
            }
*/          m_waitingTn.clear();
            while(!m_cstrToUpdateBack.empty())
            {
                qDebug() << "backward " << *m_cstrToUpdateBack.begin();
                b = cstrUpdatedBackwardInit(elementsProperties) || b;
            }
        }
        for(auto& t : m_scenario.timeNodes)
        {
            elementsProperties.timenodes[t.id()].token.disable();
        }
        for(auto& c : m_scenario.constraints)
        {
            elementsProperties.constraints[c.id()].token.disable();
        }
    }
    while(b);

    qDebug() << "\n ***** recap :";
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
    m_tnNextStep.clear();
    m_cstrToUpdateBack.clear();
    m_waitingTn.clear();

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

    /*
     * Update containers
     */
    auto it = m_waitingTn.find(timenode->id());
    if(it != m_waitingTn.end())
        m_waitingTn.erase(it); // tn is no more waiting

    m_tnToUpdate.erase(m_tnToUpdate.find(timenode->id())); // tn is computed now

    auto& tnProperties = elementsProperties.timenodes[timenode->id()];

    /* If :
     * - any tn with all prev tokens (start has no one, so it has all too)
     * - tn already has a token (when back computing or when compute waiting tn)
     */
    if(allTokens || tnProperties.hasToken())
    {
        /* **************************************************
         * UPDATE TIMENODE TOKEN
         * and update absoluteMin/Max in critical case
         */

        if(!tnProperties.hasToken())
            tnProperties.token.state = Scenario::TokenState::Forward ;

//            if(tnProperties.token.state == Scenario::TokenState::Backward)
        {
            for(auto cstrId : timenode->nextConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                if(cstrProp.hasToken())
                {
                    TimeRelationModel* cstr = m_cspScenario->m_timeRelations[cstrId];
                    // if current branch has an infinite max but join another with a finite max, the backward max token is -inf
                    if(cstrProp.token.deltaMax == -Infinity)
                    {
                        tnProperties.newAbsoluteMax =  std::min(tnProperties.newAbsoluteMax, // may have many backward token
                                                                elementsProperties.timenodes[cstr->endTn()].newAbsoluteMax - cstrProp.min);
                        ISCORE_ASSERT(tnProperties.newAbsoluteMin - 200 < tnProperties.newAbsoluteMax);
                    }
                }
            }
        }

        if(allTokens)
            for(auto cstrId : timenode->prevConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                auto constraint = m_cspScenario->m_timeRelations[cstrId];

                tnProperties.newAbsoluteMin = std::max(
                        tnProperties.newAbsoluteMin,
                        elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin + cstrProp.min);

                tnProperties.newAbsoluteMax = std::min(
                        tnProperties.newAbsoluteMax,
                        elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + cstrProp.max);

                ISCORE_ASSERT(tnProperties.newAbsoluteMin - 200 < tnProperties.newAbsoluteMax);

                // clear old token
                if(cstrProp.token.state == Scenario::TokenState::Forward)
                {
                    cstrProp.token.disable();
                }
            }

        /* **************************************************
         * APPLY DELTAS
         * to real absolute min and max
         * we check coherency before because tn can be previously correctly set by a backward action
         */
        qDebug() << "tn before :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;
        if(tnProperties.token.deltaMax == -Infinity)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
            // max is handled before
        }
        else if(tnProperties.newAbsoluteMin + tnProperties.token.deltaMin <= tnProperties.newAbsoluteMax + tnProperties.token.deltaMax)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
            tnProperties.newAbsoluteMax += tnProperties.token.deltaMax;
        }
        // TODO what if else ? Is it possible ?
        else
        {
            qDebug() << "hmm, it's embarrassing ... (try to set min > max for tn)";
            ISCORE_ABORT;
        }
        qDebug() << "tn :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;
        ISCORE_ASSERT(tnProperties.newAbsoluteMin - 200 < tnProperties.newAbsoluteMax);


        /***************************************************
         * TRANSMIT TO CONSTRAINTS
         */

        bool coherent = true;
        // previous constraints
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
                coherent = false;
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
                    coherent = false;
                }
            }
        }

        if(coherent)
            // next constraints "commit" changes to all next constraints
            for(auto cstrId : timenode->nextConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                cstrProp.token.state = Scenario::TokenState::Forward;
                if(!cstrProp.hasToken())
                {
                    cstrProp.token.deltaMin = tnProperties.token.deltaMin;
                    cstrProp.token.deltaMax = tnProperties.token.deltaMax;
                }
                else
                {
                    cstrProp.token.deltaMin = 0;
                    cstrProp.token.deltaMax = 0;
                }
                auto& endTn = Scenario::endTimeNode(m_scenario.constraint(cstrId), m_scenario);
                qDebug() << "send " << cstrProp.token.deltaMin << cstrProp.token.deltaMax << " to " <<  endTn.id();

                if(m_tnNextStep.find(endTn.id()) == m_tnNextStep.end()) // insert unique
                    m_tnNextStep.insert(endTn.id());
            }

        // Clear timenode token
        tnProperties.token.disable();
    }
    else
    {
        // tn must wait
        if(m_waitingTn.find(timenode->id()) == m_waitingTn.end())
            m_waitingTn.insert(timenode->id());
    }
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
     * - any tn with all prev tokens (start has no one, so it has all too)
     * - happened tn (like last happened tn)
     * - tn already has a token (when back computing or when compute waiting tn)
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
         * UPDATE TIMENODE TOKEN
         * and update absoluteMin/Max in critical case
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
//            if(tnProperties.token.state == Scenario::TokenState::Backward)
            {
                for(auto cstrId : timenode->nextConstraints())
                {
                    auto& cstrProp = elementsProperties.constraints[cstrId];
                    if(cstrProp.hasToken())
                    {
                        TimeRelationModel* cstr = m_cspScenario->m_timeRelations[cstrId];
                        // if current branch has an infinite max but join another with a finite max, the backward max token is -inf
                        if(cstrProp.token.deltaMax == -Infinity)
                            tnProperties.newAbsoluteMax =  std::min(tnProperties.newAbsoluteMax, // may have many backward token
                                                                    elementsProperties.timenodes[cstr->endTn()].newAbsoluteMax - cstrProp.min);
                    }
                }
            }

            for(auto cstrId : timenode->prevConstraints())
            {
                auto& cstrProp = elementsProperties.constraints[cstrId];
                auto constraint = m_cspScenario->m_timeRelations[cstrId];
                // take the more restrive deltaMin and deltaMax
                if(cstrProp.hasToken())
                {
                    qDebug() << "receive " << cstrProp.token.deltaMin<< cstrProp.token.deltaMax << " from " << cstrId;
                    tnProperties.token.deltaMin = std::max(tnProperties.token.deltaMin,
                                                            cstrProp.token.deltaMin);

                    tnProperties.token.deltaMax = std::min(tnProperties.token.deltaMax,
                                                            cstrProp.token.deltaMax);

                    if(tnProperties.token.deltaMax == -Infinity) // using deltaMax will not be satisfying, so we compute "by hand"
                        tnProperties.newAbsoluteMax = std::min(tnProperties.newAbsoluteMax,
                                                               elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + cstrProp.max);
                }

                // TODO : create specific algo for init
                /* = if init phase, init newAbsolute values */
                if(m_pastTimeNodes.size() == 0)
                {
                    tnProperties.newAbsoluteMin = std::max(
                            tnProperties.newAbsoluteMin,
                            elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin + cstrProp.min);

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

        /* **************************************************
         * APPLY DELTAS
         * to real absolute min and max
         * we check coherency before because tn can be previously correctly set by a backward action
         */
        qDebug() << "tn before :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;
        if(tnProperties.token.deltaMax == -Infinity)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
            // max is handled before
        }
        else if(tnProperties.newAbsoluteMin + tnProperties.token.deltaMin <= tnProperties.newAbsoluteMax + tnProperties.token.deltaMax)
        {
            tnProperties.newAbsoluteMin += tnProperties.token.deltaMin;
            tnProperties.newAbsoluteMax += tnProperties.token.deltaMax;
        }
        // TODO what if else ? Is it possible ?
        else
        {
            qDebug() << "hmm, it's embarrassing ... (try to set min > max for tn)";
        }
        qDebug() << "tn :" << timenode->id() << tnProperties.newAbsoluteMin << tnProperties.newAbsoluteMax;

        if(!m_pastTimeNodes.empty())
            ISCORE_ASSERT(tnProperties.newAbsoluteMin - 200 <= tnProperties.newAbsoluteMax);
        /***************************************************
         * TRANSMIT TO CONSTRAINTS
         */

        // don't transmit nul deltas if execution started,
        // but before execution nul deltas mean init phase
        // TODO move init in another function
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
                if(m_tnNextStep.find(endTn.id()) == m_tnNextStep.end()) // insert unique
                    m_tnNextStep.insert(endTn.id());
            }
            else
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
        if(m_waitingTn.find(timenode->id()) == m_waitingTn.end())
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
// true if constraint is updated
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
            cstrProperties.max = elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax
                                 - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin;
        }

        qDebug() << "cstr :" << constraint->id() << cstrProperties.min << cstrProperties.max;

        // order is ensure by previous coherency
        ISCORE_ASSERT(cstrProperties.min -200 <= cstrProperties.max); // TODO rustine
     }
    // if constraint min - max can not absorbe deltas, transmit to startTn
    else if(delta < cstrProperties.token.deltaMin ||
            delta < -cstrProperties.token.deltaMax)
    {
        auto& tnProp = elementsProperties.timenodes[constraint->startTn()];
        tnProp.token.state = Scenario::TokenState::Backward;

        tnProp.token.deltaMin = std::max(tnProp.token.deltaMin, std::max(0., cstrProperties.token.deltaMin - delta));
        tnProp.token.deltaMax = std::min(tnProp.token.deltaMax, std::min(0., cstrProperties.token.deltaMax + delta));

        if(m_waitingTn.find(constraint->startTn()) == m_waitingTn.end())
            m_waitingTn.insert(constraint->startTn());
    }
    else
    {
        qDebug() << constraint->id() << "aborsbing deltas";
/*        cstrProperties.min = std::max(cstrProperties.min,
                    elementsProperties.timenodes[constraint->endTn()].newAbsoluteMin - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax);

            cstrProperties.max = std::min(cstrProperties.max,
                        elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin);
*/
    }
}

bool ExecutionPolicy::cstrUpdatedBackwardInit(Scenario::ElementsProperties& elementsProperties)
{
    bool cstrModified = false;
    TimeRelationModel* constraint = m_cspScenario->m_timeRelations[*m_cstrToUpdateBack.begin()];
    m_cstrToUpdateBack.erase(m_cstrToUpdateBack.begin());

    TimeNodeModel* startTn = m_cspScenario->m_timeNodes[constraint->startTn()];

    auto& cstrProperties = elementsProperties.constraints[constraint->id()];
    auto& startTnProp = elementsProperties.timenodes[constraint->startTn()];

    auto tnDelta = startTnProp.newAbsoluteMax - startTnProp.newAbsoluteMin;
    auto tokenDelta = cstrProperties.token.deltaMin - cstrProperties.token.deltaMax;

    // if tn can not absorbe delta, the current constraint has to be resize to send smaller token
    if(tnDelta < tokenDelta)
    {
        startTnProp.token.state = Scenario::TokenState::Backward;

        startTnProp.token.deltaMin = cstrProperties.token.deltaMin * tnDelta/tokenDelta;
        startTnProp.token.deltaMax = cstrProperties.token.deltaMax * tnDelta/tokenDelta;

        cstrProperties.min += cstrProperties.token.deltaMin - startTnProp.token.deltaMin;

        if(cstrProperties.token.deltaMax != -Infinity) // since startTn is fixed, token inf means constraintMax inf
            cstrProperties.max += cstrProperties.token.deltaMax - startTnProp.token.deltaMax;

        else // inf token : inf constraintMax
        {
            cstrProperties.max = elementsProperties.timenodes[constraint->endTn()].newAbsoluteMax
                                 - elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin;
        }

        qDebug() << "cstr :" << constraint->id() << cstrProperties.min << cstrProperties.max;

        // order is ensure by previous coherency
        ISCORE_ASSERT(cstrProperties.min -200 <= cstrProperties.max); // TODO rustine

        cstrModified = true;
     }
    else
    {
        startTnProp.token.state = Scenario::TokenState::Backward;

        startTnProp.token.deltaMin = std::max(startTnProp.token.deltaMin, std::max(0., cstrProperties.token.deltaMin));
        startTnProp.token.deltaMax = std::min(startTnProp.token.deltaMax, std::min(0., cstrProperties.token.deltaMax));

        if(m_waitingTn.find(constraint->startTn()) == m_waitingTn.end())
            m_waitingTn.insert(constraint->startTn());
        cstrModified = true;
    }

    if(cstrModified)
        for(auto cstr : startTn->prevConstraints())
        {
            m_cstrToUpdateBack.insert(cstr);
            elementsProperties.constraints[cstr].token.deltaMin = startTnProp.token.deltaMin;
            elementsProperties.constraints[cstr].token.deltaMax = startTnProp.token.deltaMax;
        }

    return true;
}


}
