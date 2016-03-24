#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateModelInterface.hpp>


namespace CSP
{
namespace Settings
{
enum class EditionMode
{
    Mode1, Mode2, Disabled
};
enum class ExecutionMode
{
    Active, Inactive
};

struct Keys
{
        static const QString editionMode;
        static const QString executionMode;
};
class Model :
        public iscore::SettingsDelegateModelInterface
{
        Q_OBJECT
        Q_PROPERTY(EditionMode editionMode READ getEditionMode WRITE setEditionMode NOTIFY editionModeChanged)
        Q_PROPERTY(ExecutionMode executionMode READ getExecutionMode WRITE setExecutionMode NOTIFY executionModeChanged)

    public:
        Model();

        EditionMode getEditionMode() const;
        void setEditionMode(EditionMode editionMode);


        ExecutionMode getExecutionMode() const;
        void setExecutionMode(ExecutionMode executionMode);

    signals:
        void editionModeChanged(EditionMode editionMode);
        void executionModeChanged(ExecutionMode executionMode);

    private:
        void setFirstTimeSettings() override;
        EditionMode m_editionMode;
        ExecutionMode m_executionMode;
};

ISCORE_SETTINGS_PARAMETER(Model, EditionMode)
ISCORE_SETTINGS_PARAMETER(Model, ExecutionMode)

}
}
