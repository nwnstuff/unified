#include "Feedback.hpp"

#include "API/CAppManager.hpp"
#include "API/CServerExoApp.hpp"
#include "API/Constants.hpp"
#include "API/Globals.hpp"
#include "API/Functions.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSPlayer.hpp"
#include "Services/Events/Events.hpp"
#include "Services/PerObjectStorage/PerObjectStorage.hpp"
#include "ViewPtr.hpp"


using namespace NWNXLib;
using namespace NWNXLib::API;

static ViewPtr<Feedback::Feedback> g_plugin;

NWNX_PLUGIN_ENTRY Plugin::Info* PluginInfo()
{
    return new Plugin::Info
    {
        "Feedback",
        "Allows combatlog, feedback and journal updated messages to be hidden globally or per player",
        "Daz",
        "daztek@gmail.com",
        1,
        true
    };
}

NWNX_PLUGIN_ENTRY Plugin* PluginLoad(Plugin::CreateParams params)
{
    g_plugin = new Feedback::Feedback(params);
    return g_plugin;
}


namespace Feedback {

static NWNXLib::Hooking::FunctionHook* m_SendFeedbackMessageHook = nullptr;
static NWNXLib::Hooking::FunctionHook* m_SendServerToPlayerCCMessageHook = nullptr;
static NWNXLib::Hooking::FunctionHook* m_SendServerToPlayerJournalUpdatedHook = nullptr;

const int32_t FEEDBACK_MESSAGE = 0;
const int32_t COMBATLOG_MESSAGE = 1;
const int32_t JOURNALUPDATED_MESSAGE = 2;

Feedback::Feedback(const Plugin::CreateParams& params)
    : Plugin(params)
{
#define REGISTER(func) \
    GetServices()->m_events->RegisterEvent(#func, \
        [this](ArgumentStack&& args){ return func(std::move(args)); })

    REGISTER(GetMessageHidden);
    REGISTER(SetMessageHidden);
    REGISTER(SetFeedbackMode);

#undef REGISTER

    GetServices()->m_hooks->RequestExclusiveHook<API::Functions::_ZN12CNWSCreature19SendFeedbackMessageEtP16CNWCCMessageDataP10CNWSPlayer>(&SendFeedbackMessageHook);
    m_SendFeedbackMessageHook = GetServices()->m_hooks->FindHookByAddress(API::Functions::_ZN12CNWSCreature19SendFeedbackMessageEtP16CNWCCMessageDataP10CNWSPlayer);

    GetServices()->m_hooks->RequestExclusiveHook<API::Functions::_ZN11CNWSMessage27SendServerToPlayerCCMessageEjhP16CNWCCMessageDataP20CNWSCombatAttackData>(&SendServerToPlayerCCMessageHook);
    m_SendServerToPlayerCCMessageHook = GetServices()->m_hooks->FindHookByAddress(API::Functions::_ZN11CNWSMessage27SendServerToPlayerCCMessageEjhP16CNWCCMessageDataP20CNWSCombatAttackData);

    GetServices()->m_hooks->RequestExclusiveHook<API::Functions::_ZN11CNWSMessage32SendServerToPlayerJournalUpdatedEP10CNWSPlayerii13CExoLocString>(&SendServerToPlayerJournalUpdatedHook);
    m_SendServerToPlayerJournalUpdatedHook = GetServices()->m_hooks->FindHookByAddress(API::Functions::_ZN11CNWSMessage32SendServerToPlayerJournalUpdatedEP10CNWSPlayerii13CExoLocString);
}

Feedback::~Feedback()
{
}

void Feedback::SendFeedbackMessageHook(
    CNWSCreature *pCreature,
    uint16_t nFeedbackID,
    CNWCCMessageData *pData,
    CNWSPlayer *pPlayer)
{
    auto personalState = GetPersonalState(pCreature->m_idSelf, FEEDBACK_MESSAGE, nFeedbackID);
    auto bSuppressFeedback = (personalState == -1) ? GetGlobalState(FEEDBACK_MESSAGE, nFeedbackID) : personalState;

    if (g_plugin->m_FeedbackMessageWhitelist == bSuppressFeedback)
    {
        m_SendFeedbackMessageHook->CallOriginal<void>(pCreature, nFeedbackID, pData, pPlayer);
    }
}

int32_t Feedback::SendServerToPlayerCCMessageHook(
    CNWSMessage *pMessage,
    uint32_t nPlayerId,
    uint8_t nMinor,
    CNWCCMessageData *pMessageData,
    CNWSCombatAttackData *pAttackData)
{
    uint32_t oidPlayer = static_cast<CNWSPlayer*>(Globals::AppManager()->m_pServerExoApp->GetClientObjectByPlayerId(nPlayerId, 0))->m_oidPCObject;

    auto personalState = GetPersonalState(oidPlayer, COMBATLOG_MESSAGE, nMinor);
    auto bSuppressFeedback = (personalState == -1) ? GetGlobalState(COMBATLOG_MESSAGE, nMinor) : personalState;

    return g_plugin->m_CombatMessageWhitelist != bSuppressFeedback ? false :
                    m_SendServerToPlayerCCMessageHook->CallOriginal<int32_t>(pMessage, nPlayerId, nMinor, pMessageData, pAttackData);
}

int32_t Feedback::SendServerToPlayerJournalUpdatedHook(
    CNWSMessage *pMessage,
    CNWSPlayer *pPlayer,
    int32_t bQuest,
    int32_t bCompleted,
    CExoLocString locName)
{
    auto personalState = GetPersonalState(pPlayer->m_oidNWSObject, JOURNALUPDATED_MESSAGE, 0);
    auto bSuppressFeedback = (personalState == -1) ? GetGlobalState(JOURNALUPDATED_MESSAGE, 0) : personalState;

    return bSuppressFeedback ? false :
                    m_SendServerToPlayerJournalUpdatedHook->CallOriginal<int32_t>(pMessage, pPlayer, bQuest, bCompleted, locName);
}

bool Feedback::GetGlobalState(int32_t messageType, int32_t messageId)
{
    int32_t realMessageId = messageType * 10000 + messageId;

    return g_plugin->m_GlobalHiddenMessageSet.find(realMessageId) != g_plugin->m_GlobalHiddenMessageSet.end();
}

int32_t Feedback::GetPersonalState(Types::ObjectID playerId, int32_t messageType, int32_t messageId)
{
    int32_t value = -1;

    std::string varName = std::to_string(messageType) + ":" + std::to_string(messageId);

    if (auto personalState = g_plugin->GetServices()->m_perObjectStorage->Get<int>(playerId, varName))
    {
        value = !!*personalState;
    }

    return value;
}

ArgumentStack Feedback::GetMessageHidden(ArgumentStack&& args)
{
    ArgumentStack stack;

    const auto playerId = Services::Events::ExtractArgument<Types::ObjectID>(args);
    const auto messageType = Services::Events::ExtractArgument<int32_t>(args);
    const auto messageId = Services::Events::ExtractArgument<int32_t>(args);

    int32_t retVal = (playerId == Constants::OBJECT_INVALID) ? GetGlobalState(messageType, messageId) :
                                                               GetPersonalState(playerId, messageType, messageId);

    Services::Events::InsertArgument(stack, retVal);

    return stack;
}

ArgumentStack Feedback::SetMessageHidden(ArgumentStack&& args)
{
    ArgumentStack stack;

    const auto playerId = Services::Events::ExtractArgument<Types::ObjectID>(args);
    const auto messageType = Services::Events::ExtractArgument<int32_t>(args);
    const auto messageId = Services::Events::ExtractArgument<int32_t>(args);
    const auto state = Services::Events::ExtractArgument<int32_t>(args);

    ASSERT_OR_THROW(messageId >= 0);

    if (playerId == Constants::OBJECT_INVALID)
    {
        int32_t realMessageId = messageType * 10000 + messageId;

        if (!!state)
        {
            g_plugin->m_GlobalHiddenMessageSet.insert(realMessageId);
        }
        else
        {
            auto index = g_plugin->m_GlobalHiddenMessageSet.find(messageId);
            if (index != g_plugin->m_GlobalHiddenMessageSet.end())
            {
                g_plugin->m_GlobalHiddenMessageSet.erase(index);
            }
        }
    }
    else
    {
        std::string varName = std::to_string(messageType) + ":" + std::to_string(messageId);

        if (state == -1)
        {
            g_plugin->GetServices()->m_perObjectStorage->Remove(playerId, varName);
        }
        else
        {
            g_plugin->GetServices()->m_perObjectStorage->Set(playerId, varName, !!state);
        }
    }

    return stack;
}

ArgumentStack Feedback::SetFeedbackMode(ArgumentStack&& args)
{
    ArgumentStack stack;

    const auto messageType = Services::Events::ExtractArgument<int32_t>(args);
    const auto state = Services::Events::ExtractArgument<int32_t>(args);

    if (!messageType)
    {
        g_plugin->m_FeedbackMessageWhitelist = !!state;
    }
    else
    {
        g_plugin->m_CombatMessageWhitelist = !!state;
    }

    return stack;
}

}
