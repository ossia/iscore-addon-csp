#include "DocumentPlugin.hpp"

#include <Scenario/Document/ScenarioDocument/ScenarioDocumentModel.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <core/document/Document.hpp>
#include <core/document/DocumentModel.hpp>

namespace CSP
{
DocumentPlugin::DocumentPlugin(iscore::Document& doc, QObject* parent):
    iscore::DocumentPlugin{doc, "CSPDocumentPlugin", parent}
{
    reload(doc.model());
}

void DocumentPlugin::reload(iscore::DocumentModel& document)
{
    auto scenar = dynamic_cast<Scenario::ScenarioDocumentModel*>(&document.modelDelegate());
    if(!scenar)
        return;

    auto& scenarioBase = scenar->baseScenario();
    m_cspScenario = new ScenarioModel(scenarioBase, &scenarioBase);
}

ScenarioModel* DocumentPlugin::getScenario() const
{
    return m_cspScenario;
}
}
