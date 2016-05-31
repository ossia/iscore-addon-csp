#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/Policies/DisplacementPolicy.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEvent.hpp>

namespace CSP
{
Scenario::Command::SerializableMoveEvent* MoveEventCSPFactory::make(
        Path<Scenario::ScenarioModel>&& scenarioPath,
        Id<Scenario::EventModel> eventId,
        TimeValue newDate,
        ExpandMode mode)
{
    return new Scenario::Command::MoveEvent<DisplacementPolicy>{
                std::move(scenarioPath),
                std::move(eventId),
                newDate,
                mode};
}

Scenario::Command::SerializableMoveEvent* MoveEventCSPFactory::make()
{
    return new Scenario::Command::MoveEvent<DisplacementPolicy>();
}
}
