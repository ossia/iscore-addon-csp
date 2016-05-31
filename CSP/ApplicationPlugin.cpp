#include "ApplicationPlugin.hpp"
#include "DocumentPlugin.hpp"

#include <Process/Process.hpp>
#include <core/document/Document.hpp>
#include <core/document/DocumentModel.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>

namespace CSP
{
ApplicationPlugin::ApplicationPlugin(const iscore::GUIApplicationContext& pres) :
    iscore::GUIApplicationContextPlugin {pres}
{
}

void
ApplicationPlugin::on_newDocument(iscore::Document* document)
{
    if(document)
    {
        document->model().addPluginModel(new DocumentPlugin{document->context(), &document->model()});
    }
}

void
ApplicationPlugin::on_loadedDocument(iscore::Document* document)
{
    if(auto pluginModel = document->context().findPlugin<DocumentPlugin>())
    {
        pluginModel->reload(document->model());
    }
    else
    {
        on_newDocument(document);
    }
}
}
