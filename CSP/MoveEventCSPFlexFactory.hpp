#pragma once

#include <Scenario/Commands/Scenario/Displacement/MoveEventFactoryInterface.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEventList.hpp>

#include <CSP/Settings/Model.hpp>

namespace CSP
{
class MoveEventCSPFlexFactory : public Scenario::Command::MoveEventFactoryInterface
{
        ISCORE_CONCRETE_FACTORY_DECL("849ecd64-546f-4fc1-9ddf-51cfa7f39881")
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
        int priority(const iscore::ApplicationContext& ctx,  MoveEventFactoryInterface::Strategy s) const override
        {
            auto mode = ctx.settings<CSP::Settings::Model>().getEditionMode();

            if(s == MoveEventFactoryInterface::Strategy::MOVE
                && mode == CSP::Settings::EditionMode::Mode1)
            {
                return 10;
            }
            else
                return -1; // not suited for other strategies
        }

};
}
