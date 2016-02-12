#include "Presenter.hpp"
#include "Model.hpp"
#include "View.hpp"
#include <iscore/command/SerializableCommand.hpp>
#include <iscore/command/Dispatchers/ICommandDispatcher.hpp>
#include <QApplication>
#include <QStyle>
#include <iscore/command/SettingsCommand.hpp>

namespace CSP
{
namespace Settings
{
Presenter::Presenter(
        Model& m,
        View& v,
        QObject *parent):
    iscore::SettingsDelegatePresenterInterface{m, v, parent}
{
    con(v, &View::modeChanged,
        this, [&] (auto val) {
        if(val != m.getMode())
        {
            m_disp.submitCommand<SetMode>(this->model(this), val);
        }
    });

    con(m, &Model::modeChanged, &v, &View::setMode);
    v.setMode(m.getMode());
}

QString Presenter::settingsName()
{
    return tr("CSP");
}

QIcon Presenter::settingsIcon()
{
    return QApplication::style()->standardIcon(QStyle::SP_BrowserReload);
}


}
}
