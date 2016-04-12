#include "TimeNode.hpp"
#include "Scenario.hpp"

#include <kiwi/kiwi.h>
#include <Scenario/Process/ScenarioInterface.hpp>

namespace CSP
{

TimeNodeModel::TimeNodeModel(
        ScenarioModel& cspScenario,
        const Id<Scenario::TimeNodeModel>& timeNodeId)
    :ConstraintHolder::ConstraintHolder(cspScenario.getSolver(), &cspScenario)
{
    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeNode");

    auto& timeNodeModel = cspScenario.getScenario()->timeNode(timeNodeId);

    m_iscoreDate = &timeNodeModel.date();

    m_date.setValue(m_iscoreDate->msec());
    m_id = timeNodeId;

    if(m_id == Id<Scenario::TimeNodeModel>{0})
    {
        putConstraintInSolver(new kiwi::Constraint{m_date == 0});
        putConstraintInSolver(new kiwi::Constraint{m_date_min == 0});
        putConstraintInSolver(new kiwi::Constraint{m_date_max == 0});
    }
    else
    {
        putConstraintInSolver(new kiwi::Constraint{m_date >= 0});
        putConstraintInSolver(new kiwi::Constraint{m_date_min >= 0});
        putConstraintInSolver(new kiwi::Constraint{m_date_max >= m_date_min});
    }

    // watch over date edits
    con(timeNodeModel, &Scenario::TimeNodeModel::dateChanged, this, &TimeNodeModel::onDateChanged);
}

kiwi::Variable& TimeNodeModel::getDate()
{
    return m_date;
}

const TimeValue*TimeNodeModel::getIscoreDate()
{
    return m_iscoreDate;
}

kiwi::Variable&TimeNodeModel::getDateMin()
{
    return m_date_min;
}

kiwi::Variable&TimeNodeModel::getDateMax()
{
    return m_date_max;
}

bool TimeNodeModel::dateChanged() const
{
    return m_date.value() != m_iscoreDate->msec();
}

void TimeNodeModel::restoreConstraints()
{
    for(auto constraint : m_constraints)
    {
        if(!m_solver.hasConstraint(*constraint))
        {
           m_solver.addConstraint(*constraint);
        }
    }
}

void TimeNodeModel::onDateChanged(const TimeValue& date)
{
    m_date.setValue(date.msec());
}

}
