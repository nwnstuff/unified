#include "Tweaks/DisablePause.hpp"

#include "Services/Hooks/Hooks.hpp"
#include "Utils.hpp"

#include "API/CAppManager.hpp"
#include "API/CServerExoAppInternal.hpp"
#include "API/CServerExoApp.hpp"
#include "API/Functions.hpp"
#include "API/Globals.hpp"

namespace Tweaks {

using namespace NWNXLib;
using namespace NWNXLib::API;

NWNXLib::Hooking::FunctionHook* DisablePause::pSetPauseState_hook;
DisablePause::DisablePause(ViewPtr<Services::HooksProxy> hooker)
{
    hooker->RequestExclusiveHook<Functions::_ZN13CServerExoApp13SetPauseStateEhi>
                                    (&CServerExoAppInternal__SetPauseState_hook);

    pSetPauseState_hook = hooker->FindHookByAddress(Functions::_ZN13CServerExoApp13SetPauseStateEhi);
}

void DisablePause::CServerExoAppInternal__SetPauseState_hook(CServerExoAppInternal* thisPtr, uint8_t nState, int32_t bPause)
{
    // nState=1 - timestop
    // nState=2 - DM pause
    if (nState != 2)
        pSetPauseState_hook->CallOriginal<void>(thisPtr, nState, bPause);
}

}
