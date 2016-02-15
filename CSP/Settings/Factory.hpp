#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateFactoryInterface.hpp>

namespace CSP
{
namespace Settings
{

class Factory :
        public iscore::SettingsDelegateFactory
{
        ISCORE_CONCRETE_FACTORY_DECL("2f7e0691-08ee-427d-b21a-6b4edf5b0d5d")

        iscore::SettingsDelegateViewInterface *makeView() override;
        iscore::SettingsDelegatePresenterInterface* makePresenter_impl(
                iscore::SettingsDelegateModelInterface& m,
                iscore::SettingsDelegateViewInterface& v,
                QObject* parent) override;
        iscore::SettingsDelegateModelInterface *makeModel() override;
};

}
}
