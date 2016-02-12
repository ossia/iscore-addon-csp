#include "Model.hpp"
#include <QSettings>

namespace CSP
{
namespace Settings
{

const QString Keys::mode = QStringLiteral("iscore_plugin_csp/Mode");


Model::Model()
{
    QSettings s;

    if(!s.contains(Keys::mode))
    {
        setFirstTimeSettings();
    }

    m_mode = static_cast<Mode>(s.value(Keys::mode).toInt());
}

Mode Model::getMode() const
{
    return m_mode;
}

void Model::setMode(Mode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;

    QSettings s;
    s.setValue(Keys::mode, static_cast<int>(m_mode));
    emit modeChanged(mode);
}

void Model::setFirstTimeSettings()
{
    QSettings s;
    s.setValue(Keys::mode, static_cast<int>(m_mode));
    m_mode = Mode::Mode1;
}

}
}
