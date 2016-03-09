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
                const Id<Scenario::TimeNodeModel>& positionnedElement,
                Scenario::ElementsProperties& elementsProperties) override;

    protected:
        void refreshStays(ScenarioModel& cspScenario,
                                 Scenario::ElementsProperties& elementsProperties,
                                 const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements);

    private:
        void updateTnInit(Scenario::ElementsProperties& elementsProperties);
        void tnUpdated(Scenario::ElementsProperties& elementsProperties);
        void tnWaiting(Scenario::ElementsProperties& elementsProperties);
        void cstrUpdatedBackward(Scenario::ElementsProperties& elementsProperties);
        bool cstrUpdatedBackwardInit(Scenario::ElementsProperties& elementsProperties);

        Scenario::ScenarioModel& m_scenario;
        CSP::ScenarioModel* m_cspScenario{};
        QVector<Id<Scenario::TimeNodeModel>> m_pastTimeNodes;

        QSet<Id<Scenario::TimeNodeModel>> m_tnToUpdate; // for this step
        QSet<Id<Scenario::TimeNodeModel>> m_tnNextStep; // can be computed as soon as possible
        QSet<Id<Scenario::TimeNodeModel>> m_waitingTn; // are waiting for some previous constraints
        QSet<Id<Scenario::ConstraintModel>> m_cstrToUpdateBack;
};

}
