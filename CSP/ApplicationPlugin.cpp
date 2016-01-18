#include "ApplicationPlugin.hpp"
#include "DocumentPlugin.hpp"

#include <Process/Process.hpp>
#include <core/document/Document.hpp>
#include <core/document/DocumentModel.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>

namespace CSP
{
ApplicationPlugin::ApplicationPlugin(const iscore::ApplicationContext& pres) :
    iscore::GUIApplicationContextPlugin {pres, "CSPApplicationPlugin", nullptr}
{
}

iscore::DocumentPluginModel* ApplicationPlugin::loadDocumentPlugin(
        const QString& name,
        const VisitorVariant& var,
        iscore::Document *parent)
{
    // We don't have anything to load; it's easier to just recreate.
    return nullptr;
}

void
ApplicationPlugin::on_newDocument(iscore::Document* document)
{
    if(document)
    {
        document->model().addPluginModel(new DocumentPlugin{*document, &document->model()});
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
