#include "View.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QCheckBox>

Q_DECLARE_METATYPE(CSP::Settings::EditionMode)
Q_DECLARE_METATYPE(CSP::Settings::ExecutionMode)

namespace CSP
{
namespace Settings
{

View::View():
    m_widg{new QWidget}
{
    auto lay = new QFormLayout;
    m_widg->setLayout(lay);

    m_editionModeCB = new QComboBox;
    m_editionModeCB->addItem(tr("Mode 1"), QVariant::fromValue(EditionMode::Mode1));
    m_editionModeCB->addItem(tr("Mode 2"), QVariant::fromValue(EditionMode::Mode2));
    m_editionModeCB->addItem(tr("Disabled"), QVariant::fromValue(EditionMode::Disabled));
    lay->addRow(tr("Edition Mode"), m_editionModeCB);

    connect(m_editionModeCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&] (int idx) {
        emit editionModeChanged(m_editionModeCB->itemData(idx).value<EditionMode>());
    });

    m_execModeCB = new QCheckBox;
    lay->addRow(tr("Active on execution"), m_execModeCB);
    connect(m_execModeCB, &QCheckBox::toggled,
            this, [&] (bool b)
    {
        ExecutionMode newMode = b ? ExecutionMode::Active : ExecutionMode::Inactive;
        emit executionModeChanged(newMode);
    });
}

void View::setEditionMode(EditionMode val)
{
    switch(val)
    {
        case EditionMode::Mode1:
            m_editionModeCB->setCurrentIndex(0);
            break;
        case EditionMode::Mode2:
            m_editionModeCB->setCurrentIndex(1);
            break;
        case EditionMode::Disabled:
            m_editionModeCB->setCurrentIndex(2);
            break;
        default:
            return;
    }
}

void View::setExecutionMode(ExecutionMode val)
{
    switch(val)
    {
        case ExecutionMode::Active:
            m_execModeCB->setChecked(true);
            break;
        case ExecutionMode::Inactive:
            m_execModeCB->setChecked(false);
            break;
        default:
            return;
    }
}

QWidget *View::getWidget()
{
    return m_widg;
}

}
}
