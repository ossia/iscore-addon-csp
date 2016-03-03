#pragma once

#include <Process/TimeValue.hpp>
#include <Scenario/Document/TimeNode/TimeNodeModel.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <iscore/tools/SettableIdentifier.hpp>
#include <CSP/Model/tools/ConstraintHolder.hpp>

#include <kiwi/kiwi.h>
#include <QVector>

#include "Scenario.hpp"

namespace CSP
{
class DisplacementPolicy;
class TimeNodeModel : public ConstraintHolder
{
    friend class DisplacementPolicy;
    friend class FlexDisplacementPolicy;
    friend class ExecutionPolicy;

public:
    TimeNodeModel(ScenarioModel& cspScenario, const Id<Scenario::TimeNodeModel>& timeNodeId);

    TimeNodeModel() = default;

    ~TimeNodeModel() = default;

    kiwi::Variable& getDate();
    const TimeValue* getIscoreDate();

    /**
     * @brief dateChanged
     * call this function to check if csp date differ from iscore date
     * @return
     */
    bool dateChanged() const;

    const auto nextConstraints() const { return m_nextConstraints;}
    const auto prevConstraints() const { return m_prevConstraints;}
    const Id<Scenario::TimeNodeModel>& id() const {return m_id;}

private:
    kiwi::Variable m_date{"date"};
    const TimeValue* m_iscoreDate;

    void onDateChanged(const TimeValue& date);

    // use during scenario execution
    std::list<Id<Scenario::ConstraintModel>> m_nextConstraints;
    std::list<Id<Scenario::ConstraintModel>> m_prevConstraints;
    const Id<Scenario::TimeNodeModel>& m_id;

};
}
