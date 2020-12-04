#define U_are_compiling_for_windows
#define _WIN32_WINNT 0x0A00 //As required by some library, they wanna know which platform you are on, selected for windows 10
//https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-160

#include "../Managed_cpp_General_core/core.cpp.hpp"