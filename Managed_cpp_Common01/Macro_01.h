#pragma once
#define fcout(_STR_) std::cout<<_STR_<<std::endl
#define fwcout(_STR_) std::wcout<<_STR_<<std::endl

std::mutex mtx_J3BskLkF;
#define fcout_mtx(_STR_) mtx_J3BskLkF.lock(); std::cout<<_STR_<<std::endl; mtx_J3BskLkF.unlock()
#define fwcout_mtx(_STR_) mtx_J3BskLkF.lock();std::wcout<<_STR_<<std::endl; mtx_J3BskLkF.unlock()

#define L1_duo_MX L1_taf_napkin_core_duo_MX
#define L2_duo_MX L2_taf_napkin_core_duo_MX
#define L1_duo_INX L1_taf_napkin_core_duo_INX
#define L2_duo_INX L2_taf_napkin_core_duo_INX

#define L1_async L1_taf_napkin_nit
#define L2_async L2_taf_napkin_nit
#define L1_async_titans L1_taf_nit
#define L2_async_titans L2_taf_nit

#define LX_async_manual_switch_on(_A) _A.execute_all()
#define LX_async_conclude(_A) _A.finish_all(); _A.deactivate()
//If you use manual switch you must you conclude other wise it will execute twice

#define ct_start auto ERmUUBKcjfnt0awudh_begining = std::chrono::high_resolution_clock::now(); {
#define ct_end }auto ERmUUBKcjfnt0awudh_end = std::chrono::high_resolution_clock::now(); fcout((std::chrono::duration_cast<std::chrono::microseconds>(ERmUUBKcjfnt0awudh_end-ERmUUBKcjfnt0awudh_begining)).count());