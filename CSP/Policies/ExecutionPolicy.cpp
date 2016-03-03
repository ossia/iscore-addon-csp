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
        elementsProperties.constraints[cstr.id()].max = m_scenario.constraint(cstr.id()).duration.maxDuration().msec();
    }

    m_tnToUpdate.insert(m_cspScenario->getStartTimeNode()->id());
    while(m_tnToUpdate.size() != 0 || m_waitingTn.size() != 0 || m_cstrToUpdateBack.size() != 0)
    {
        qDebug() << "loop";
        auto n = m_tnToUpdate.size();
        for(int i = 0; i < n; i++)
        {
            qDebug() << "to update" << *m_tnToUpdate.begin();
            tnUpdated(elementsProperties);
        }
    }

}

void ExecutionPolicy::computeDisplacement(const Id<Scenario::TimeNodeModel>& positionnedElement,
                                          Scenario::ElementsProperties& elementsProperties)
{
    m_tnToUpdate.clear();
    m_pastTimeNodes.push_back(positionnedElement);
    m_tnToUpdate.insert(positionnedElement);

    qDebug() << "new computation";
    while(m_tnToUpdate.size() != 0 || m_waitingTn.size() != 0 || m_cstrToUpdateBack.size() != 0)
    {
        qDebug() << "loop";
        auto n = m_tnToUpdate.size();
        for(int i = 0; i < n; i++)
        {
            qDebug() << "to update" << *m_tnToUpdate.begin();
            tnUpdated(elementsProperties);
        }

        n = m_waitingTn.size();

        for(int i = 0; i < n; i++)
        {
            qDebug() << "waiting" << *m_waitingTn.begin();
            tnWaiting(elementsProperties);
        }
        n = m_cstrToUpdateBack.size();
        for(int i = 0; i < n; i++)
        {
            qDebug() << "constraint" << *m_cstrToUpdateBack.begin();
            cstrUpdatedBackward(elementsProperties);
        }
    }
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
    TimeNodeModel* timenode{};
    bool allTokens = true;

    timenode = m_cspScenario->m_timeNodes[*m_tnToUpdate.begin()];

    for(auto cstrId : timenode->prevConstraints())
    {
        if(! elementsProperties.constraints[cstrId].token)
        {
            allTokens = false;

            break;
        }
    }

    if(allTokens || m_pastTimeNodes.contains(timenode->id()))
    {
        // tn is no more waiting
        auto it = m_waitingTn.find(timenode->id());
        if(it != m_waitingTn.end())
            m_waitingTn.erase(it);
        m_tnToUpdate.erase(m_tnToUpdate.begin());

        // ***************************************************
        // INIT TIMENODE VALUES

        elementsProperties.timenodes[timenode->id()].token = new Scenario::Token{};


        if(m_pastTimeNodes.contains(timenode->id())) // if happened we compute the delta to send in tokens
        {
            elementsProperties.timenodes[timenode->id()].token->deltaMin = elementsProperties.timenodes[timenode->id()].dateMin
                                                                            - elementsProperties.timenodes[timenode->id()].newAbsoluteMin;

            elementsProperties.timenodes[timenode->id()].token->deltaMax = elementsProperties.timenodes[timenode->id()].dateMax
                                                                            - elementsProperties.timenodes[timenode->id()].newAbsoluteMax;

            elementsProperties.timenodes[timenode->id()].newAbsoluteMin = elementsProperties.timenodes[timenode->id()].dateMin;
            elementsProperties.timenodes[timenode->id()].newAbsoluteMax = elementsProperties.timenodes[timenode->id()].dateMax;
        }
        else
        {
            // take the more restrive deltaMin and deltaMax
            for(auto cstrId : timenode->prevConstraints())
            {
                elementsProperties.timenodes[timenode->id()].token->deltaMin = std::max(
                                                                elementsProperties.timenodes[timenode->id()].token->deltaMin,
                                                                elementsProperties.constraints[cstrId].token->deltaMin);

                elementsProperties.timenodes[timenode->id()].token->deltaMax = std::min(
                                                        elementsProperties.timenodes[timenode->id()].token->deltaMax,
                                                        elementsProperties.constraints[cstrId].token->deltaMax);

                if(m_pastTimeNodes.size() == 0) // = if init phase
                {
                    if(elementsProperties.timenodes[timenode->id()].newAbsoluteMin == 0)
                    {
                        auto constraint = m_cspScenario->m_timeRelations[cstrId];
                        elementsProperties.timenodes[timenode->id()].newAbsoluteMin =
                                elementsProperties.timenodes[constraint->startTn()].newAbsoluteMin + elementsProperties.constraints[cstrId].min;
                        elementsProperties.timenodes[timenode->id()].newAbsoluteMax =
                                elementsProperties.timenodes[constraint->startTn()].newAbsoluteMax + elementsProperties.constraints[cstrId].max;
                    }
                }
            }
            // now apply this deltas to real absolute min and max
            elementsProperties.timenodes[timenode->id()].newAbsoluteMin += elementsProperties.timenodes[timenode->id()].token->deltaMin;
            elementsProperties.timenodes[timenode->id()].newAbsoluteMax += elementsProperties.timenodes[timenode->id()].token->deltaMax;
        }

        // ***************************************************
        // TRANSMIT TO CONSTRAINTS
        // assume a coherent system

        // don't transmit nul deltas if execution started
        if(!m_pastTimeNodes.empty() && elementsProperties.timenodes[timenode->id()].token->deltaMin == 0
                && elementsProperties.timenodes[timenode->id()].token->deltaMax == 0)
            return;

        // previous constraints
        for(auto cstrId : timenode->prevConstraints())
        {
            // clear old token
            delete elementsProperties.constraints[cstrId].token;
            elementsProperties.constraints[cstrId].token = nullptr;

            auto& startTn = Scenario::startTimeNode(m_scenario.constraint(cstrId), m_scenario);

            // ------------------------------------------
            // if timenode min/max are more restrictive than this branch
            // token is send back
            if(elementsProperties.constraints[cstrId].min + elementsProperties.timenodes[startTn.id()].newAbsoluteMin
                    < elementsProperties.timenodes[timenode->id()].newAbsoluteMin)
            {
                elementsProperties.constraints[cstrId].token = new Scenario::Token{};
                elementsProperties.constraints[cstrId].token->deltaMin = elementsProperties.timenodes[timenode->id()].token->deltaMin;

                m_cstrToUpdateBack.insert(cstrId);
            }
            if(elementsProperties.constraints[cstrId].max + elementsProperties.timenodes[startTn.id()].newAbsoluteMax
                    > elementsProperties.timenodes[timenode->id()].newAbsoluteMax)
            {
                elementsProperties.constraints[cstrId].token = new Scenario::Token{};
                elementsProperties.constraints[cstrId].token->deltaMax = elementsProperties.timenodes[timenode->id()].token->deltaMax;

                m_cstrToUpdateBack.insert(cstrId);
            }
        }

        // next constraints
        for(auto cstrId : timenode->nextConstraints())
        {
            elementsProperties.constraints[cstrId].token = new Scenario::Token{};
            elementsProperties.constraints[cstrId].token->deltaMin = elementsProperties.timenodes[timenode->id()].token->deltaMin;
            elementsProperties.constraints[cstrId].token->deltaMax = elementsProperties.timenodes[timenode->id()].token->deltaMax;

            auto& endTn = Scenario::endTimeNode(m_scenario.constraint(cstrId), m_scenario);
            m_tnToUpdate.insert(endTn.id());
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

    //send tokens in constraint who haven't already one
    for(auto cstrId : timenode->prevConstraints())
    {
        if(!elementsProperties.constraints[cstrId].token)
        {
            elementsProperties.constraints[cstrId].token = new Scenario::Token{};
            elementsProperties.constraints[cstrId].token->deltaMin = elementsProperties.timenodes[timenode->id()].token->deltaMin;
            elementsProperties.constraints[cstrId].token->deltaMax = elementsProperties.timenodes[timenode->id()].token->deltaMax;

            m_cstrToUpdateBack.insert(cstrId);
        }
    }
    for(auto cstrId : timenode->nextConstraints())
    {
        if(!elementsProperties.constraints[cstrId].token)
        {
            elementsProperties.constraints[cstrId].token = new Scenario::Token{};
            elementsProperties.constraints[cstrId].token->deltaMin = elementsProperties.timenodes[timenode->id()].token->deltaMin;
            elementsProperties.constraints[cstrId].token->deltaMax = elementsProperties.timenodes[timenode->id()].token->deltaMax;

            m_tnToUpdate.insert(m_cspScenario->m_timeRelations[cstrId]->startTn());
        }
    }
}

void ExecutionPolicy::cstrUpdatedBackward(Scenario::ElementsProperties& elementsProperties)
{
    TimeRelationModel* constraint = m_cspScenario->m_timeRelations[*m_cstrToUpdateBack.begin()];

    auto& iscoreCstr = m_scenario.constraint(constraint->id());

    auto delta = elementsProperties.constraints[constraint->id()].max - elementsProperties.constraints[constraint->id()].min;

    // if constraint is already playing, we have to update its min & max
    if(m_scenario.state(iscoreCstr.startState()).status() == Scenario::ExecutionStatus::Happened)
    {
        elementsProperties.constraints[constraint->id()].min += elementsProperties.constraints[constraint->id()].token->deltaMin;
        elementsProperties.constraints[constraint->id()].max += elementsProperties.constraints[constraint->id()].token->deltaMax;

        qDebug() << constraint->id() << elementsProperties.constraints[constraint->id()].token->deltaMin << elementsProperties.constraints[constraint->id()].token->deltaMax;
        qDebug() << elementsProperties.constraints[constraint->id()].min << elementsProperties.constraints[constraint->id()].max;
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
