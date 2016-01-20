#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/Policies/DisplacementPolicy.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEvent.hpp>

namespace CSP
{
Scenario::Command::SerializableMoveEvent* MoveEventCSPFactory::make(
        Path<Scenario::ScenarioModel> &&scenarioPath,
        const Id<Scenario::EventModel> &eventId,
        const TimeValue &newDate,
        ExpandMode mode)
{
    return new Scenario::Command::MoveEvent<DisplacementPolicy>(std::move(scenarioPath), eventId, newDate, mode);
}

Scenario::Command::SerializableMoveEvent* MoveEventCSPFactory::make()
{
    return new Scenario::Command::MoveEvent<DisplacementPolicy>();
}

const Scenario::Command::MoveEventFactoryKey& MoveEventCSPFactory::key_impl() const
{
    static const Scenario::Command::MoveEventFactoryKey str{"CSP"};
    return str;
}
}
