#include <CSP/MoveEventCSPFlexFactory.hpp>
#include <CSP/Policies/FlexDisplacementPolicy.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEvent.hpp>

namespace CSP
{
Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make(
        Path<Scenario::ScenarioModel> &&scenarioPath,
        const Id<Scenario::EventModel> &eventId,
        const TimeValue &newDate,
        ExpandMode mode)
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>(std::move(scenarioPath), eventId, newDate, mode);
}

Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make()
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>();
}

auto MoveEventCSPFlexFactory::concreteFactoryKey() const -> const ConcreteFactoryKey&
{
    // TODO why was CSP CSPFlex and CSPFlex CSP
    static const ConcreteFactoryKey str{"849ecd64-546f-4fc1-9ddf-51cfa7f39881"};
    return str;
}
}
