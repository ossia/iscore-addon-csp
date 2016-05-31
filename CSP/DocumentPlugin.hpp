#pragma once

#include <iscore/plugins/documentdelegate/plugin/DocumentDelegatePluginModel.hpp>
#include "Model/Scenario.hpp"

namespace iscore
{
class DocumentModel;
}

class OSSIABaseScenarioElement;

namespace CSP
{
class DocumentPlugin : public iscore::DocumentPlugin
{
        Q_OBJECT
    public:
        DocumentPlugin(const iscore::DocumentContext& doc, QObject* parent);

        void reload(iscore::DocumentModel& doc);

        ScenarioModel* getScenario() const;

    private:
        ScenarioModel* m_cspScenario;
};
}
