#pragma once

#include <Scenario/Commands/Scenario/Displacement/MoveEventFactoryInterface.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEventList.hpp>

#include <CSP/Settings/Model.hpp>

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
        int priority(const iscore::ApplicationContext& ctx, MoveEventFactoryInterface::Strategy s) const override
        {
            auto mode = ctx.settings<CSP::Settings::Model>().getMode();

            if(s == MoveEventFactoryInterface::Strategy::MOVE
                && mode == CSP::Settings::Mode::Mode2)
            {
                return 10;
            }
            else
                return -1;// not suited for other strategies
        }
};
}
