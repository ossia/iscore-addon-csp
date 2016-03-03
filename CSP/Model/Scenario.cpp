#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Process/ScenarioInterface.hpp>
#include <kiwi/kiwi.h>
#include <QtAlgorithms>

namespace CSP
{
ScenarioModel::ScenarioModel(
        const Scenario::ScenarioModel& scenario,
        QObject *parent):
    QObject::QObject(parent),
    m_scenario(&scenario)
{
    this->setObjectName("CSPScenario");

    // ensure that start timenode is stored first of all
    m_startTimeNode = insertTimenode(scenario.startTimeNode().id());

    // insert existing timenodes
    for(auto& timeNodeModel : scenario.timeNodes)
    {
        on_timeNodeCreated(timeNodeModel);
    }

    // insert existing constraints
    for(auto& constraintModel : scenario.constraints)
    {
        on_constraintCreated(constraintModel);
    }

    // Link with i-score
    scenario.constraints.added.connect<ScenarioModel, &ScenarioModel::on_constraintCreated>(this);
    scenario.constraints.removed.connect<ScenarioModel, &ScenarioModel::on_constraintRemoved>(this);

    scenario.states.added.connect<ScenarioModel, &ScenarioModel::on_stateCreated>(this);
    scenario.states.removed.connect<ScenarioModel, &ScenarioModel::on_stateRemoved>(this);

    scenario.events.added.connect<ScenarioModel, &ScenarioModel::on_eventCreated>(this);
    scenario.events.removed.connect<ScenarioModel, &ScenarioModel::on_eventRemoved>(this);

    scenario.timeNodes.added.connect<ScenarioModel, &ScenarioModel::on_timeNodeCreated>(this);
    scenario.timeNodes.removed.connect<ScenarioModel, &ScenarioModel::on_timeNodeRemoved>(this);
}

ScenarioModel::ScenarioModel(const Scenario::BaseScenario& baseScenario, QObject *parent)
    :QObject::QObject(parent), m_scenario(&baseScenario)
{
    this->setObjectName("CSPScenario");

    // ensure that start then end timenode are stored first of all
    m_startTimeNode = insertTimenode(baseScenario.startTimeNode().id());

    // insert existing timenodes
    on_timeNodeCreated(baseScenario.startTimeNode());

    // insert existing constraints
    on_constraintCreated(baseScenario.constraint());
}

ScenarioModel::~ScenarioModel()
{
    qDeleteAll(m_timeNodes);
    qDeleteAll(m_timeRelations);
}

kiwi::Solver&
ScenarioModel::getSolver()
{
    return m_solver;
}

TimeNodeModel *ScenarioModel::getStartTimeNode() const
{
    return m_startTimeNode;
}

const Scenario::ScenarioInterface *ScenarioModel::getScenario() const
{
    return m_scenario;
}

TimeNodeModel* ScenarioModel::insertTimenode(
        const Id<Scenario::TimeNodeModel> &timeNodeId)
{
    // if timenode not already here, put it in
    if(! m_timeNodes.contains(timeNodeId))
    {
        auto cspTimenode = new TimeNodeModel(*this, timeNodeId);
        m_timeNodes.insert(timeNodeId, cspTimenode);
        return cspTimenode;
    }else
    {
        return m_timeNodes[timeNodeId];
    }
}

TimeRelationModel *ScenarioModel::getTimeRelation(
        const Id<Scenario::ConstraintModel> &ConstraintId)
{
    return m_timeRelations[ConstraintId];
}

void
ScenarioModel::computeAllConstraints()
{

}

void
ScenarioModel::on_constraintCreated(
        const Scenario::ConstraintModel& constraintModel)
{
    //create the corresponding time relation
    m_timeRelations.insert(constraintModel.id(), new TimeRelationModel{*this, constraintModel.id()});
}

void
ScenarioModel::on_constraintRemoved(
        const Scenario::ConstraintModel& constraint)
{
    delete(m_timeRelations.take(constraint.id()));
}


void
ScenarioModel::on_stateCreated(
        const Scenario::StateModel& state)
{}

void
ScenarioModel::on_stateRemoved(
        const Scenario::StateModel& state)
{}


void
ScenarioModel::on_eventCreated(
        const Scenario::EventModel& event)
{}

void
ScenarioModel::on_eventRemoved(
        const Scenario::EventModel& event)
{}


void
ScenarioModel::on_timeNodeCreated(
        const Scenario::TimeNodeModel& timeNodeModel)
{
    insertTimenode(timeNodeModel.id());
}

void
ScenarioModel::on_timeNodeRemoved(
        const Scenario::TimeNodeModel& timeNode)
{
    delete(m_timeNodes.take(timeNode.id()));
}

const
TimeNodeModel&
ScenarioModel::getInsertTimenode(
        Scenario::ScenarioInterface& scenario,
        const Id<Scenario::TimeNodeModel>& timeNodeId)
{
    insertTimenode(timeNodeId);

    return *m_timeNodes[timeNodeId];
}
}
