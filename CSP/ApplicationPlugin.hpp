#pragma once
#include <iscore/plugins/application/GUIApplicationContextPlugin.hpp>
#include <iscore/plugins/documentdelegate/plugin/DocumentDelegatePluginModel.hpp>

namespace CSP
{
class ApplicationPlugin final : public iscore::GUIApplicationContextPlugin
{
    public:
        ApplicationPlugin(const iscore::GUIApplicationContext& pres);
        ~ApplicationPlugin() = default;

    private:
        void on_newDocument(iscore::Document* doc) override;
        void on_loadedDocument(iscore::Document* doc) override;
};
}
