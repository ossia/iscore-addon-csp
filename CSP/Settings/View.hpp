#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateViewInterface.hpp>
#include <CSP/Settings/Model.hpp>

class QComboBox;
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

        void setMode(Mode);
    signals:
        void modeChanged(Mode);

    private:
        QWidget* getWidget() override;
        QWidget* m_widg{};

        QComboBox* m_cb{};

};

}
}
