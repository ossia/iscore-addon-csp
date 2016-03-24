#include "Model.hpp"
#include <QSettings>

namespace CSP
{
namespace Settings
{

const QString Keys::editionMode = QStringLiteral("iscore_plugin_csp/EditionMode");
const QString Keys::executionMode = QStringLiteral("iscore_plugin_csp/ExecutionMode");

Model::Model()
{
    QSettings s;

    if(!s.contains(Keys::editionMode))
    {
        setFirstTimeSettings();
    }

    m_editionMode = static_cast<EditionMode>(s.value(Keys::editionMode).toInt());
    m_executionMode = static_cast<ExecutionMode>(s.value(Keys::executionMode).toInt());
}

EditionMode Model::getEditionMode() const
{
    return m_editionMode;
}

void Model::setEditionMode(EditionMode editionMode)
{
    if (m_editionMode == editionMode)
        return;

    m_editionMode = editionMode;

    QSettings s;
    s.setValue(Keys::editionMode, static_cast<int>(m_editionMode));
    emit editionModeChanged(editionMode);
}

ExecutionMode Model::getExecutionMode() const
{
    return m_executionMode;
}

void Model::setExecutionMode(ExecutionMode executionMode)
{
    if (m_executionMode == executionMode)
        return;

    m_executionMode = executionMode;
    QSettings s;
    s.setValue(Keys::executionMode, static_cast<int>(m_executionMode));
    emit executionModeChanged(executionMode);
}

void Model::setFirstTimeSettings()
{
    m_editionMode = EditionMode::Disabled;

    QSettings s;
    s.setValue(Keys::editionMode, static_cast<int>(EditionMode::Disabled));
    s.setValue(Keys::executionMode, static_cast<int>(ExecutionMode::Inactive));
}

}
}
