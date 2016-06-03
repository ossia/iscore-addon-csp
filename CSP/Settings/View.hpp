#pragma once
#include <iscore/plugins/settingsdelegate/SettingsDelegateView.hpp>
#include <CSP/Settings/Model.hpp>

class QComboBox;
namespace CSP
{
namespace Settings
{

class View :
        public iscore::SettingsDelegateView
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
