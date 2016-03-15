#include "TimeRelation.hpp"
#include "Scenario.hpp"
#include <Scenario/Process/ScenarioInterface.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/Branch.hpp>
#include <kiwi/kiwi.h>
#include <Scenario/Process/Algorithms/Accessors.hpp>

namespace CSP
{
TimeRelationModel::TimeRelationModel(
        ScenarioModel& cspScenario,
        const Id<Scenario::ConstraintModel>& constraintId):
    ConstraintHolder::ConstraintHolder(cspScenario.getSolver(), &cspScenario),
    m_id{constraintId}
{
    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeRelation");

    auto& scenario = *cspScenario.getScenario();
    auto& constraint = scenario.constraint(constraintId);

    m_iscoreMin = constraint.duration.minDuration();
    m_iscoreMax = constraint.duration.maxDuration();

    m_min.setValue(m_iscoreMin.msec());

    if(m_iscoreMax.isInfinite())
    {
        // TODO check this
        m_max.setValue(std::numeric_limits<double>::infinity());
    }
    else
    {
        m_max.setValue(m_iscoreMax.msec());
    }

    // weight
    //solver.addEditVariable(m_min, kiwi::strength::strong);
    //solver.addEditVariable(m_max, kiwi::strength::medium);

    auto& prevTimenodeModel = startTimeNode(constraint, scenario);
    auto& nextTimenodeModel = endTimeNode(constraint, scenario);

    m_start = prevTimenodeModel.id();
    m_end = nextTimenodeModel.id();

    //retrieve/create prev and next timenode
    auto prevCSPTimenode = cspScenario.insertTimenode(prevTimenodeModel.id());
    auto nextCSPTimenode = cspScenario.insertTimenode(nextTimenodeModel.id());

    // apply model constraints
    // 1 - min >= 0
    auto cMinGreaterThanZero = new kiwi::Constraint({m_min >= 0});\
    putConstraintInSolver(cMinGreaterThanZero);

    // 2 - min inferior to max
    auto cMinLowerThanMax = new kiwi::Constraint({m_min <= m_max});\
    putConstraintInSolver(cMinLowerThanMax);

    // 3 - date of end timenode inside min and max
    auto cNextDateMin = new kiwi::Constraint({nextCSPTimenode->getDate() >= (prevCSPTimenode->getDate() + m_min)});\
    putConstraintInSolver(cNextDateMin);

    auto cNextDateMax = new kiwi::Constraint({nextCSPTimenode->getDate() <= (prevCSPTimenode->getDate() + m_max)});\
    putConstraintInSolver(cNextDateMax);


    // if there are sub scenarios, store them
    for(auto& process : constraint.processes)
    {
        if(auto* scenar = dynamic_cast<Scenario::ScenarioModel*>(&process))
        {
            m_subScenarios.insert(scenar->id(), new ScenarioModel(*scenar, scenar));
        }
    }

    // watch over durations edits
    con(constraint.duration, &Scenario::ConstraintDurations::minDurationChanged,
        this, &TimeRelationModel::onMinDurationChanged);
    con(constraint.duration, &Scenario::ConstraintDurations::maxDurationChanged,
        this, &TimeRelationModel::onMaxDurationChanged);

    // watch over potential subscenarios
    constraint.processes.added.connect<TimeRelationModel, &TimeRelationModel::onProcessCreated>(this);
    constraint.processes.removed.connect<TimeRelationModel, &TimeRelationModel::onProcessRemoved>(this);
}

TimeRelationModel::~TimeRelationModel()
{
}

kiwi::Variable& TimeRelationModel::getMin()
{
    return m_min;
}

kiwi::Variable& TimeRelationModel::getMax()
{
    return m_max;
}

const TimeValue&TimeRelationModel::getIscoreMin()
{
    return m_iscoreMin;
}

const TimeValue&TimeRelationModel::getIscoreMax()
{
    return m_iscoreMax;
}

bool TimeRelationModel::minChanged() const
{
    return m_min.value() != m_iscoreMin.msec();
}

bool TimeRelationModel::maxChanged() const
{
    return m_max.value() != m_iscoreMax.msec();
}

void TimeRelationModel::onMinDurationChanged(const TimeValue& min)
{
    m_min.setValue(min.msec());
    m_iscoreMin = min;
}

void TimeRelationModel::onMaxDurationChanged(const TimeValue& max)
{
    m_iscoreMax = max;
    if(max.isInfinite())
    {
        //TODO : ??? remove constraints on max?
        m_max.setValue(std::numeric_limits<double>::infinity());
    }else
    {
        m_max.setValue(max.msec());
    }
}

void TimeRelationModel::onProcessCreated(const Process::ProcessModel& process)
{
    if(auto scenario = dynamic_cast<const Scenario::ScenarioModel*>(&process))
    {
        m_subScenarios.insert(scenario->id(), new ScenarioModel(*scenario, const_cast<Scenario::ScenarioModel*>(scenario)));
    }
}

void TimeRelationModel::onProcessRemoved(const Process::ProcessModel& process)
{
    if(auto scenario = dynamic_cast<const Scenario::ScenarioModel*>(&process))
    {
        delete(m_subScenarios.take(scenario->id()));
    }
}
}
