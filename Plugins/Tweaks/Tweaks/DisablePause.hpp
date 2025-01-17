#pragma once

#include "API/Types.hpp"
#include "Common.hpp"
#include "ViewPtr.hpp"
#include "Services/Hooks/Hooks.hpp"

namespace Tweaks {

class DisablePause
{
public:
    DisablePause(NWNXLib::ViewPtr<NWNXLib::Services::HooksProxy> hooker);

private:
    static void CServerExoAppInternal__SetPauseState_hook(CServerExoAppInternal*, uint8_t, int32_t);
    static NWNXLib::Hooking::FunctionHook* pSetPauseState_hook;
};

}
