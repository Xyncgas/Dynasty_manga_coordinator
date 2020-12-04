#pragma once
//The app enviornment, not the compiler / std / program enviornment
#ifdef Deprecated_iYIhv7irlXIcY
struct osyfoEqSHakZX//dummy name
{
    struct _Module_osyfoEqSHakZX//dummy name
	{
		Common_functions::Libcurl_custom Libcurl_custom;
	};
	_Module_osyfoEqSHakZX _Module_;//So basically this struct is the module of this app
};

auto CORE_ =
std::make_shared<
	LF::el_nests<osyfoEqSHakZX,LF::el_end>
	>();
#endif
//#define CORE_HttpGetString(_STR_) (*CORE_).c._Module_.Libcurl_custom.HttpGetString(_STR_)
//The order is sorted and hardcoded according to the module's position in the Orchestry tree now.
//Moved to Macros.h
struct Core_Components_L1
{
	struct _MvmiYLPCCOFnt
	{
		/*
		struct global_dynamic_csp_shared_ {
			std::array<L1_thread_version_dynamic_container, Thread_pool_cores> Dynamic_L1_async_titans_collection;
			std::array<L1_thread_version_dynamic_container_lite, Thread_pool_cores> Dynamic_L1_async_collection;
			std::array<L2_thread_version_dynamic_container, Thread_pool_cores> Dynamic_L2_async_titans_collection;
			std::array<L2_thread_version_dynamic_container_lite, Thread_pool_cores> Dynamic_L2_async_collection;
		};
		*///use dynamic_csp.h in your program's header for global enviornment instead
		std::array<LF::csp_dynamic::L1_thread_version_dynamic_container, 1> global_dynamic_csp;
		_MvmiYLPCCOFnt()
		{
			//Initializers for fine tuning the modules for run time (program) that wasn't able to be done inside these modules, before their initialization.
			global_dynamic_csp[0].set_default_number_for_threads(Thread_pool_cores);//set default threads
		}
	};
	_MvmiYLPCCOFnt _Initialized_Moduels_;//Sometimes for example a function can be called directly, and sometimes you might wanna use it as a struct, and maybe Initialize it first for the program here (for example something like libcurl)
};

struct Core_Dev_Env
{
	struct Lg4ZXz3WGB4ZC
	{
		Models::Common02::Dynasty_manga_::Dev_env Dynasty_Manga_Dev;
	};
	Lg4ZXz3WGB4ZC _Dev_;
};

auto Core_ =
std::make_shared<
	LF::el_nests<Core_Components_L1,//For now, it's just one level, and fit stuffs inside and using it like a name space, in the future probably have more levels?
	//LF::el_nests<Core_Dev_Env,//You can Remove (maybe comment) out this line for production
	LF::el_end>
>();