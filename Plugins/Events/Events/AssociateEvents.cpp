#include "Events/AssociateEvents.hpp"
#include "API/CNWSCreature.hpp"
#include "API/Functions.hpp"
#include "Events.hpp"
#include "Utils.hpp"

namespace Events {

using namespace NWNXLib;

AssociateEvents::AssociateEvents(ViewPtr<Services::HooksProxy> hooker)
{
    Events::InitOnFirstSubscribe("NWNX_ON_ADD_ASSOCIATE_.*", [hooker]() {
        hooker->RequestSharedHook<API::Functions::_ZN12CNWSCreature12AddAssociateEjt, void,
            CNWSCreature*, API::Types::ObjectID, uint16_t>(&AddAssociateHook);
    });

    Events::InitOnFirstSubscribe("NWNX_ON_REMOVE_ASSOCIATE_.*", [hooker]() {
        hooker->RequestSharedHook<API::Functions::_ZN12CNWSCreature15RemoveAssociateEj, void,
            CNWSCreature*, API::Types::ObjectID>(&RemoveAssociateHook);
    });
}

void AssociateEvents::AddAssociateHook(Services::Hooks::CallType type, CNWSCreature* thisPtr, API::Types::ObjectID assocId, uint16_t)
{
    const bool before = type == Services::Hooks::CallType::BEFORE_ORIGINAL;
    Events::PushEventData("ASSOCIATE_OBJECT_ID", NWNXLib::Utils::ObjectIDToString(assocId));
    Events::SignalEvent(before ? "NWNX_ON_ADD_ASSOCIATE_BEFORE" : "NWNX_ON_ADD_ASSOCIATE_AFTER", thisPtr->m_idSelf);
}

void AssociateEvents::RemoveAssociateHook(Services::Hooks::CallType type, CNWSCreature* thisPtr, API::Types::ObjectID assocId)
{
    const bool before = type == Services::Hooks::CallType::BEFORE_ORIGINAL;
    Events::PushEventData("ASSOCIATE_OBJECT_ID", NWNXLib::Utils::ObjectIDToString(assocId));
    Events::SignalEvent(before ? "NWNX_ON_REMOVE_ASSOCIATE_BEFORE" : "NWNX_ON_REMOVE_ASSOCIATE_AFTER", thisPtr->m_idSelf);
}

}
