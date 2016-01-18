#pragma once

#include <Process/TimeValue.hpp>
#include <Scenario/Document/TimeNode/TimeNodeModel.hpp>
#include <iscore/tools/SettableIdentifier.hpp>
#include <CSP/Model/tools/ConstraintHolder.hpp>

#include <kiwi/kiwi.h>
#include <QVector>

#include "Scenario.hpp"


namespace Scenario
{
class TimeNodeModel;
}

namespace CSP
{
class DisplacementPolicy;
class TimeNodeModel : public ConstraintHolder
{
    friend class DisplacementPolicy;
    friend class FlexDisplacementPolicy;

public:
    TimeNodeModel(ScenarioModel& cspScenario, const Id<Scenario::TimeNodeModel>& timeNodeId);

    TimeNodeModel() = default;

    ~TimeNodeModel() = default;

    kiwi::Variable& getDate();

    /**
     * @brief dateChanged
     * call this function to check if csp date differ from iscore date
     * @return
     */
    bool dateChanged() const;

private:
    kiwi::Variable m_date{"date"};
    const TimeValue* m_iscoreDate;

    void onDateChanged(const TimeValue& date);
};
}
