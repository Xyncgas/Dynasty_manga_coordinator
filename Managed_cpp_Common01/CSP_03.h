#pragma once
#if 0
template<const size_t Program_desinged_threads>
//in practice please use a macro for keeping track of position number and looking at tsp_array_size and perhaps putting it in a c++ header file yea
//Because csp is kinda expansive to initiate (either the compile time version and the run time version), and it's really giving efficiency in terms of allowing more worker/threads to be computing and enabling this field for async programming model too, instead of using less power to do more computations, I'm current implementing, these machines to be statically assigned and available for reusing.
//which I think it's totally possible to determine the number of csp needed at compile time because you were using them in the codes everywhere anyways, but now you are just pinning things down and sharing them
struct global_dynamic_csp_shared_ {
	//dynamic rt containers inside array
	std::array<L1_thread_version_dynamic_container, Program_desinged_threads> Dynamic_L1_async_titans_collection;
	//in theory you can keep using the same csp instance, untill you can not anymore, like threads. Although you might spawn infinite threads and there is not enough memory for all the instances you need, and therefore this wouldn't suit well for fine grained performance and inconsistant/short lived operations, since the memory is going to be fragmented and a lot allocation would be used in rt so use the module like limited resource and a global vector for you to push all the async.
	std::array<L1_thread_version_dynamic_container_lite, Program_desinged_threads> Dynamic_L1_async_collection;
	std::array<L2_thread_version_dynamic_container, Program_desinged_threads> Dynamic_L2_async_titans_collection;
	std::array<L2_thread_version_dynamic_container_lite, Program_desinged_threads> Dynamic_L2_async_collection;
	//still, there is a problem that you are splitting many tasks at rt to run and I thought it was like in different thread but apparently it's not and thus a little bit useless to do that and thus a little frustrated, and I am going to provide a struct that doesn't split the tasks now
};
#endif

//deprecated, define these in program instead.