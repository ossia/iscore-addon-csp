#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateModelInterface.hpp>


namespace CSP
{
namespace Settings
{
enum class Mode
{
    Mode1, Mode2, Disabled
};

struct Keys
{
        static const QString mode;
};
class Model :
        public iscore::SettingsDelegateModelInterface
{
        Q_OBJECT
        Q_PROPERTY(Mode mode READ getMode WRITE setMode NOTIFY ModeChanged)

    public:
        Model();

        Mode getMode() const;
        void setMode(Mode getMode);

    signals:
        void ModeChanged(Mode getMode);

    private:
        void setFirstTimeSettings() override;
        Mode m_mode;
};

ISCORE_SETTINGS_PARAMETER(Model, Mode)

}
}
