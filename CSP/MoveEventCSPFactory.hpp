#pragma once

#include <Scenario/Commands/Scenario/Displacement/MoveEventFactoryInterface.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEventList.hpp>

namespace CSP
{
class MoveEventCSPFactory : public Scenario::Command::MoveEventFactoryInterface
{
        ISCORE_CONCRETE_FACTORY_DECL("29b0eb02-9dbb-476a-a8e4-866702c6db2f")
    public:
        Scenario::Command::SerializableMoveEvent* make(
                Path<Scenario::ScenarioModel> &&scenarioPath,
                const Id<Scenario::EventModel> &eventId,
                const TimeValue &newDate,
                ExpandMode mode) override;

        Scenario::Command::SerializableMoveEvent* make() override;

        // Priority is called to choose the policy.
        // The choosen policy is the one with the greatest priority
        // in the asked context (="strategy")
        int priority(MoveEventFactoryInterface::Strategy strategy) const override
        {
            switch(strategy)
            {
                case MoveEventFactoryInterface::Strategy::MOVING_BOUNDED:
                    return 10;
                    break;
                default:
                    return -1;// not suited for other strategies
                    break;
            }
        }
};
}
