#pragma once

#include "API/Types.hpp"
#include "Common.hpp"
#include "ViewPtr.hpp"
#include "Services/Hooks/Hooks.hpp"

namespace Tweaks {

class StringToIntBaseToAuto
{
public:
    StringToIntBaseToAuto(NWNXLib::ViewPtr<NWNXLib::Services::HooksProxy> hooker);

private:
    static int32_t CNWVirtualMachineCommands__ExecuteCommandStringConversions_hook(CNWVirtualMachineCommands*, int32_t, int32_t);
};

}
