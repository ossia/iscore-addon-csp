#include "View.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <iscore/widgets/SignalUtils.hpp>

Q_DECLARE_METATYPE(CSP::Settings::Mode)

namespace CSP
{
namespace Settings
{

View::View():
    m_widg{new QWidget}
{
    auto lay = new QFormLayout;
    m_widg->setLayout(lay);

    m_cb = new QComboBox;
    m_cb->addItem(tr("Mode 1"), QVariant::fromValue(Mode::Mode1));
    m_cb->addItem(tr("Mode 2"), QVariant::fromValue(Mode::Mode2));
    m_cb->addItem(tr("Disabled"), QVariant::fromValue(Mode::Disabled));
    lay->addRow(tr("Mode"), m_cb);

    connect(m_cb, SignalUtils::QComboBox_currentIndexChanged_int(),
            this, [&] (int idx) {
        emit modeChanged(m_cb->itemData(idx).value<Mode>());
    });
}

void View::setMode(Mode val)
{
    switch(val)
    {
        case Mode::Mode1:
            m_cb->setCurrentIndex(0);
            break;
        case Mode::Mode2:
            m_cb->setCurrentIndex(1);
            break;
        case Mode::Disabled:
            m_cb->setCurrentIndex(2);
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
