#pragma once

//Macro doesn't have to be valid, it's just when they are used
constexpr auto Thread_pool_cores = 9;
constexpr auto tpc = Thread_pool_cores;

#ifdef Deprecated_iYIhv7irlXIcY
#define CORE_HttpGetString(_STR_) (*CORE_).c._Module_.Libcurl_custom.HttpGetString(_STR_)
//The order is sorted and hardcoded according to the module's position in the Orchestry tree now
#endif
#define Core (*Core_)
#define Core_Dev Core.n.c