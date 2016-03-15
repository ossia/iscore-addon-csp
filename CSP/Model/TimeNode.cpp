#include "TimeNode.hpp"
#include "Scenario.hpp"

#include <Scenario/Process/ScenarioInterface.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <CSP/Model/Branch.hpp>

namespace CSP
{

TimeNodeModel::TimeNodeModel(
        ScenarioModel& cspScenario,
        const Id<Scenario::TimeNodeModel>& timeNodeId)
    :ConstraintHolder::ConstraintHolder(cspScenario.getSolver(), &cspScenario),
      m_id{timeNodeId}
{
    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeNode");

    const Scenario::ScenarioInterface* scenario = cspScenario.getScenario();
    auto& timeNodeModel = scenario->timeNode(timeNodeId);

    m_iscoreDate = &timeNodeModel.date();

    m_date.setValue(m_iscoreDate->msec());
    // apply model constraints

    // 1 - events cannot happen before the start node
    // except for start timenode
    if(timeNodeId.val() != 0)
    {
        auto constraintName = new kiwi::Constraint({m_date >= cspScenario.getStartTimeNode()->getDate()});\
        putConstraintInSolver(constraintName);
    }
    else// if it is indeed start node, constrain him the the start value
    {
        auto constraintName = new kiwi::Constraint({m_date == m_date.value()});\
        putConstraintInSolver(constraintName);
    }

    // watch over date edits
    con(timeNodeModel, &Scenario::TimeNodeModel::dateChanged, this, &TimeNodeModel::onDateChanged);


    m_prevConstraints.merge(Scenario::previousConstraints(timeNodeModel, *scenario));
    m_nextConstraints.merge(Scenario::nextConstraints(timeNodeModel, *scenario));
}

kiwi::Variable& TimeNodeModel::getDate()
{
    return m_date;
}

const TimeValue*TimeNodeModel::getIscoreDate()
{
    return m_iscoreDate;
}

bool TimeNodeModel::dateChanged() const
{
    return m_date.value() != m_iscoreDate->msec();
}

void TimeNodeModel::addNextTimeRelation(Id<Scenario::ConstraintModel> cstrId)
{
    for(auto it = m_nextConstraints.begin(); it != m_nextConstraints.end(); it++)
    {
        if(*it == cstrId)
            return;
    }
    m_nextConstraints.push_back(cstrId);
}

void TimeNodeModel::addPrevTimeRelation(Id<Scenario::ConstraintModel> cstrId)
{
    for(auto it = m_prevConstraints.begin(); it != m_prevConstraints.end(); it++)
    {
        if(*it == cstrId)
            return;
    }
    m_prevConstraints.push_back(cstrId);
}

void TimeNodeModel::removeNextTimeRelation(Id<Scenario::ConstraintModel> id)
{
    m_nextConstraints.remove(id);
}

void TimeNodeModel::removePrevTimeRelation(Id<Scenario::ConstraintModel> id)
{
   m_prevConstraints.remove(id);
}

void TimeNodeModel::onDateChanged(const TimeValue& date)
{
    m_date.setValue(date.msec());
}

}
