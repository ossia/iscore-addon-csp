#include <CSP/MoveEventCSPFlexFactory.hpp>
#include <CSP/Policies/FlexDisplacementPolicy.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEvent.hpp>

namespace CSP
{
Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make(
        Path<Scenario::ScenarioModel> &&scenarioPath,
        Id<Scenario::EventModel> eventId,
        TimeValue newDate,
        ExpandMode mode)
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>{
        std::move(scenarioPath),
                std::move(eventId),
                newDate,
                mode};
}

Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make()
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>();
}
}
