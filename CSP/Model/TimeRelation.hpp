#pragma once

#include <kiwi/kiwi.h>
#include <QVector>
#include <iscore/tools/SettableIdentifier.hpp>
#include <Process/TimeValue.hpp>
#include <CSP/Model/tools/ConstraintHolder.hpp>

#include <CSP/Model/Scenario.hpp>


namespace Scenario
{
class ConstraintModel;
}

namespace CSP
{
class DisplacementPolicy;
class TimeRelationModel : public CSPConstraintHolder, public Nano::Observer
{
    friend class DisplacementPolicy;
    friend class FlexDisplacementPolicy;

public:
    TimeRelationModel(ScenarioModel& scenario, const Id<Scenario::ConstraintModel>& constraintId);

    TimeRelationModel() = default;

    ~TimeRelationModel();

    kiwi::Variable& getMin();

    kiwi::Variable& getMax();

    /**
     * @brief minChanged
     * call this function to check if csp min differ from iscore min
     * @return
     */
    bool minChanged() const;

    /**
     * @brief maxChanged
     * call this function to check if csp max differ from iscore max
     * @return
     */
    bool maxChanged() const;

private:
    kiwi::Variable m_min{"min"};
    TimeValue m_iscoreMin{};

    kiwi::Variable m_max{"max"};
    TimeValue m_iscoreMax{};

    kiwi::Constraint m_cstrRigidity{kiwi::Constraint(m_min == m_max)};// TODO ask JM if it is safe to do so

    //void onDefaultDurationChanged(const TimeValue& arg);
    void onMinDurationChanged(const TimeValue& min);
    void onMaxDurationChanged(const TimeValue& max);

    void onProcessCreated(const Process::ProcessModel& process);
    void onProcessRemoved(const Process::ProcessModel& process);

    QHash<Id<Process::ProcessModel>, ScenarioModel*> m_subScenarios;
};
}
