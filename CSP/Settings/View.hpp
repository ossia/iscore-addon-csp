#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateViewInterface.hpp>
#include <CSP/Settings/Model.hpp>

class QComboBox;
class QCheckBox;
namespace CSP
{
namespace Settings
{

class View :
        public iscore::SettingsDelegateViewInterface
{
        Q_OBJECT
    public:
        View();

        void setEditionMode(EditionMode);
        void setExecutionMode(ExecutionMode);

    signals:
        void editionModeChanged(EditionMode);
        void executionModeChanged(ExecutionMode);

    private:
        QWidget* getWidget() override;
        QWidget* m_widg{};

        QComboBox* m_editionModeCB{};
        QCheckBox* m_execModeCB{};
};

}
}
