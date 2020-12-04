#pragma once
//#define _Using_Precompile_Omiko_
//#include "Master_pch.h"
//common precompiled

#if 0
#define Wasm_codes
#endif

#if 1
#define ROOT_MODEL_nests
#endif
#if 0
#define ROOT_MODEL_steam
#endif
#if 0
#define ROOT_MODEL_spread
#endif

#ifdef Wasm_codes
#include <emscripten/bind.h>
#include <emscripten.h>
#endif

#include "../Managed_cpp_Common01/Omiko.h" //common not precompiled
#include "../Managed_cpp_Common02/Omiko.h"
#include "Orchestry_Omiko.h" //app structure

int main(int argc, char*argv[])
{
	///There's code execution in the global which will clean up after main() ends
	//React();
}