#pragma once

#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Tools/dataStructures.hpp>

#include <Scenario/ExecutionChecker/CSPCoherencyCheckerInterface.hpp>

namespace CSP {

class ScenarioModel;

class ExecutionPolicy final : public Scenario::CSPCoherencyCheckerInterface
{
    public:
        ExecutionPolicy() = default;

        ExecutionPolicy(Scenario::ScenarioModel& scenario, Scenario::ElementsProperties& elementsProperties);

        void
        computeDisplacement(
                const Id<Scenario::TimeNodeModel>& positionnedElement) override;

    private:
        bool updateTnInit();
        void fixConstraints();
        void updateBranchInit();

        void tnUpdated();
        void tnWaiting();
        void cstrUpdatedBackward();

        void simplify();

        Scenario::ScenarioModel& m_scenario;
        CSP::ScenarioModel* m_cspScenario{};
        QVector<Id<Scenario::TimeNodeModel>> m_pastTimeNodes;

        Scenario::ElementsProperties& m_elementsProperties;

        QSet<Id<Scenario::TimeNodeModel>> m_tnToUpdate; // for this step
        Id<Scenario::TimeNodeModel> m_tnToFix{}; // at init

        QSet<Id<Scenario::TimeNodeModel>> m_tnNextStep; // can be computed as soon as possible
        QSet<Id<Scenario::TimeNodeModel>> m_waitingTn; // are waiting for some previous constraints
        QSet<Id<Scenario::ConstraintModel>> m_cstrToUpdateBack;


};

}
