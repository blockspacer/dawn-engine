/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2019 (git@dga.me.uk)
 */
#include "Base.h"
#include "UI.h"
#include "Layout.h"

namespace dw {

Layout::Layout(UI* im, Rocket::Core::ElementDocument* document)
    : mInterfaceMgr(im), mDocument(document) {
    document->Show();
}

Layout::~Layout() {
    // This was a tricky bug to fix.. When you remove a reference in
    // libRocket, it doesn't actually delete anything until
    // mContext->RemoveReference. This causes it to call Listener::OnDelete()
    // after the state object has been deleted, so lets clean up here...
    for (auto& t : mListeners) {
        mDocument->GetElementById(std::get<0>(t).c_str())
            ->RemoveEventListener(std::get<1>(t).c_str(), std::get<2>(t));
    }

    mDocument->RemoveReference();
    mDocument->Close();
}

void Layout::focusOn(const String& id) {
    if (!mDocument->GetElementById(id.c_str())->Focus()) {
        assert(true && "Unable to attain focus");
    }
}

void Layout::show(int showType) {
    mDocument->Show(showType);
}

void Layout::hide() {
    // Disable modality by calling "Show" again
    if (mDocument->IsModal())
        mDocument->Show(Rocket::Core::ElementDocument::NONE);

    // Remove focus
    mDocument->GetFocusLeafNode()->Blur();

    // Hide
    mDocument->Hide();
}

bool Layout::isVisible() const {
    return mDocument->IsVisible();
}

void Layout::bindEvent(const String& id, UIEvent event) {
    // Map event ID to string
    String eventID;
    switch (event) {
        case UI_MOUSE_ENTER:
            eventID = "mouseover";
            break;

        case UI_MOUSE_LEAVE:
            eventID = "mouseout";
            break;

        case UI_CLICK:
            eventID = "click";
            break;

        case UI_SUBMIT:
            eventID = "submit";
            break;

        default:
            break;
    }

    // Look up the element
    Rocket::Core::Element* elem = mDocument->GetElementById(id.c_str());
    assert(elem);

    // Add the Listener
    elem->AddEventListener(eventID.c_str(), mInterfaceMgr);
    mListeners.push_back(makeTuple(id, eventID, mInterfaceMgr));
}

Rocket::Core::Element* Layout::getElementById(Rocket::Core::String id) {
    return mDocument->GetElementById(id);
}

Rocket::Core::ElementDocument* Layout::getDocument() {
    return mDocument;
}
}  // namespace dw
