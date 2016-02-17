#pragma once

namespace Scenario{
    class ScenarioModel;
    class TimeNodeModel;
    struct ElementsProperties;
}

namespace CSP {

void compute(
    Scenario::ScenarioModel& scenario,
    const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
    const TimeValue& deltaTime,
    Scenario::ElementsProperties& elementsProperties);

void updateConstraints(Scenario::ScenarioModel& scenario,
                       const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements,
                       Scenario::ElementsProperties& elementsProperties);

}
