#include "TimeNode.hpp"
#include "Scenario.hpp"

#include <kiwi/kiwi.h>
#include <Scenario/Process/ScenarioInterface.hpp>

namespace CSP
{

TimeNodeModel::TimeNodeModel(
        ScenarioModel& cspScenario,
        const Id<Scenario::TimeNodeModel>& timeNodeId)
    :CSPConstraintHolder::CSPConstraintHolder(cspScenario.getSolver(), &cspScenario)
{
    auto& solver = cspScenario.getSolver();

    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeNode");

    auto& timeNodeModel = cspScenario.getScenario()->timeNode(timeNodeId);

    m_iscoreDate = &timeNodeModel.date();

    m_date.setValue(m_iscoreDate->msec());

    // apply model constraints

    // 1 - events cannot happen before the start node
    // except for start timenode
    if(timeNodeId.val() != 0)
    {
        PUT_CONSTRAINT(constraintNodeAfterStart, m_date >= cspScenario.getStartTimeNode()->getDate());
    }else// if it is indeed start node, constrain him the the start value
    {
        PUT_CONSTRAINT(cStartDontMove, m_date == m_date.value());
    }

    // watch over date edits
    con(timeNodeModel, &Scenario::TimeNodeModel::dateChanged, this, &TimeNodeModel::onDateChanged);
}

kiwi::Variable& TimeNodeModel::getDate()
{
    return m_date;
}

bool TimeNodeModel::dateChanged() const
{
    return m_date.value() != m_iscoreDate->msec();
}

void TimeNodeModel::onDateChanged(const TimeValue& date)
{
    m_date.setValue(date.msec());
}

}
