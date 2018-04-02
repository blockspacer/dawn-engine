/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#include "Common.h"
#include "scene/System.h"

namespace dw {
System::System(Context* context) : Object{context}, ontology_system_{nullptr} {
}

void System::beginProcessing() {
}

void System::setOntologyAdapter_internal(Ontology::System* system) {
    ontology_system_ = system;
    ontology_system_->setTypeSets(supported_components_, depending_systems_);
}
}  // namespace dw