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
    con(v, &View::editionModeChanged,
        this, [&] (auto val) {
        if(val != m.getEditionMode())
        {
            m_disp.submitCommand<SetEditionMode>(this->model(this), val);
        }
    });

    con(m, &Model::editionModeChanged, &v, &View::setEditionMode);
    v.setEditionMode(m.getEditionMode());

    con(v, &View::executionModeChanged,
        this, [&] (auto val) {
        if(val != m.getExecutionMode())
        {
            m_disp.submitCommand<SetExecutionMode>(this->model(this), val);
        }
    });

    con(m, &Model::executionModeChanged, &v, &View::setExecutionMode);
    v.setExecutionMode(m.getExecutionMode());
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
