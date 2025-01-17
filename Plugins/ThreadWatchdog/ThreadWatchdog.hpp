#pragma once

#include "Plugin.hpp"
#include "Services/Hooks/Hooks.hpp"
#include <memory>
#include <thread>

namespace ThreadWatchdog {

class ThreadWatchdog : public NWNXLib::Plugin
{
public:
    ThreadWatchdog(const Plugin::CreateParams& params);
    virtual ~ThreadWatchdog();

private:
    static void MainLoopUpdate(NWNXLib::Services::Hooks::CallType type, CServerExoAppInternal*);
    std::unique_ptr<std::thread> m_watchdog;
};

}
