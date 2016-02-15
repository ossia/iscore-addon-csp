#pragma once

#include <QHash>

#include <iscore/tools/SettableIdentifier.hpp>
#include <CSP/Policies/DisplacementPolicy.hpp>

#include <kiwi/kiwi.h>


namespace Scenario
{
class ConstraintModel;
class TimeNodeModel;
class EventModel;
class StateModel;
class ScenarioModel;
class ScenarioInterface;
class BaseScenario;
}

namespace CSP
{
class TimeNodeModel;
class TimeRelationModel;
class ScenarioModel : public QObject, public Nano::Observer
{
    friend class DisplacementPolicy;
    friend class FlexDisplacementPolicy;

    Q_OBJECT
public:
    //using QObject::QObject;

    ScenarioModel(const Scenario::ScenarioModel& scenario, QObject *parent);
    ScenarioModel(const Scenario::BaseScenario& baseScenario, QObject *parent);

    ~ScenarioModel();

    void on_constraintCreated(const Scenario::ConstraintModel&);
    void on_stateCreated(const Scenario::StateModel&);
    void on_eventCreated(const Scenario::EventModel&);
    void on_timeNodeCreated(const Scenario::TimeNodeModel&);

    void on_constraintRemoved(const Scenario::ConstraintModel&);
    void on_stateRemoved(const Scenario::StateModel&);
    void on_eventRemoved(const Scenario::EventModel&);
    void on_timeNodeRemoved(const Scenario::TimeNodeModel&);


    const TimeNodeModel& getInsertTimenode(
            Scenario::ScenarioInterface& scenario,
            const Id<Scenario::TimeNodeModel>& timeNodeId);

    kiwi::Solver& getSolver();

    TimeNodeModel* getStartTimeNode() const;

    TimeNodeModel* getEndTimeNode() const;

    const Scenario::ScenarioInterface* getScenario() const;

    TimeNodeModel* insertTimenode(
            const Id<Scenario::TimeNodeModel>& timeNodeId);

    TimeRelationModel* getTimeRelation(
            const Id<Scenario::ConstraintModel>& ConstraintId);

    QHash<Id<Scenario::TimeNodeModel>,TimeNodeModel*> m_timeNodes;
    QHash<Id<Scenario::ConstraintModel>,TimeRelationModel*> m_timeRelations;

private:

    const Scenario::ScenarioInterface* m_scenario;

    TimeNodeModel* m_startTimeNode{};


    void computeAllConstraints();

    kiwi::Solver m_solver{};
};
}
