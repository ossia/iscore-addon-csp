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
    friend class ExecutionPolicy;

public:
    TimeNodeModel(ScenarioModel& cspScenario, const Id<Scenario::TimeNodeModel>& timeNodeId);

    TimeNodeModel() = default;

    ~TimeNodeModel() = default;

    kiwi::Variable& getDate();
    const TimeValue* getIscoreDate();

    kiwi::Variable& getDateMin();
    kiwi::Variable& getDateMax();

    /**
     * @brief dateChanged
     * call this function to check if csp date differ from iscore date
     * @return
     */
    bool dateChanged() const;
    void restoreConstraints() override;

private:
    kiwi::Variable m_date{"date"};
    const TimeValue* m_iscoreDate;

    kiwi::Variable m_date_min{"dmin"};
    kiwi::Variable m_date_max{"dmax"};

    Id<Scenario::TimeNodeModel> m_id{};

    void onDateChanged(const TimeValue& date);
};
}
