#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateFactory.hpp>

#include <CSP/Settings/Model.hpp>
#include <CSP/Settings/Presenter.hpp>
#include <CSP/Settings/View.hpp>

namespace CSP
{
namespace Settings
{
ISCORE_DECLARE_SETTINGS_FACTORY(Factory, Model, Presenter, View, "2f7e0691-08ee-427d-b21a-6b4edf5b0d5d")
}
}
