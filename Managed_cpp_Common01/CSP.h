#pragma once
#include <functional>
namespace LF
{
	namespace csp
	{
		template<typename local_, local_ initializer>
		struct instantiated_stack_function {
			local_ storage = initializer;// something for function to access in a local scope
			bool active = true;

			std::function<void(local_&)> image;
			void operator () (std::function<void(local_&)> execution_)
			{
				image = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}//no adding to array, it's goona overwrite
			void execute()
			{
				image(storage);
			}
			void deactivate()
			{
				active = false;
			}
			~instantiated_stack_function()
			{
				if (active != false)
				{
					//maybe fence - BUG?
					execute();
				}
			}
		};

		template<const size_t tsp_threads, const size_t tsp_array_size>
		struct L1_thread_version_instantiated_stack_function {
			//local_ storage; - for these type you can use external thread_local
			bool active = true;

			std::array<std::function<void()>, tsp_array_size> image = { nullptr };

			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			constexpr size_t qt() { return (tsp_array_size - 1) / tsp_threads; }//compile time constant, if tsp_threads >= tsp_array_size qt == 0
			constexpr size_t mdl() { return (tsp_array_size - 1) % tsp_threads; }//compile time constant, if you lie about the array size to be larger than the functions you are putting in it's alright, if tsp_threads > tsp_array_size run every function in a single threads

			//constexpr size_t threads() {if (mdl != 0) { return tsp_threads + 1; } return tsp_threads;}//if multiple blocks can fit in specified tsp_threads, if not a new thread is created for redundent

			std::array < std::future<void>, tsp_threads + 1> tp;//threads pool. +1 for in case the task can't spread completely..
			//csp is using stsd::async for executing while threads aren't created for each lambda

			//if I can figure out constexpr I can make array size not always neccesary to +1 when mdl == 0. - optimization

			constexpr void operator () (std::function<void()> execution_, size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0]();
				}
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos]();
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			constexpr void execute_all()
			{
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt() * index_limit/*it's optimized right because unique value*/; i < (qt() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				if (has_multiple_blocks())
				{
					if (tsp_threads < tsp_array_size)
					{
						for (size_t ia = 0; ia < tsp_threads; ia++)//make N threads, where N would have mdl blocks inside
						{
							tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
						}
						if (mdl() != 0 && mdl() != (tsp_array_size - 1))//or &&tsp_threads!=1
						{
							tp[tsp_threads] = std::async(std::launch::async, [&]() {
								for (size_t i = ((qt() * tsp_threads) + 1); i < tsp_array_size; i++)
								{
									execute_index(i);
								}
								});//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads == tsp_array_size)//qt == 0
					{
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < tsp_array_size; i++)
						{
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads > tsp_array_size)//threads
					{
						for (size_t i = 1; i < tsp_array_size; i++)
						{
							tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < (tsp_threads + 1); i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L1_thread_version_instantiated_stack_function()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
			//notice the programming pattern how things get abstracted more the closer to end of struct
		};

		template<typename local_, const size_t tsp_threads, const size_t tsp_array_size>
		struct L2_thread_version_instantiated_stack_function {
			local_ storage;// something for function to access in a local scope
			bool active = true;

			std::array<std::function<void(local_&)>, tsp_array_size> image = { nullptr };

			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			constexpr size_t qt() { (tsp_array_size - 1) / tsp_threads; }//compile time constant
			constexpr size_t mdl() { return (tsp_array_size - 1) % tsp_threads; }//compile time constant

			//constexpr size_t threads() {if (mdl != 0) { return tsp_threads + 1; } return tsp_threads;}//if multiple blocks can fit in specified tsp_threads, if not a new thread is created for redundent

			std::array < std::future<void>, tsp_threads + 1> tp;//threads pool. +1 for in case the task can't spread completely..
			//if I can figure out constexpr I can make array size not always neccesary to +1 when mdl == 0. - optimization

			constexpr void operator () (std::function<void(local_&)> execution_, size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0](storage);
				}//in case people declared two slots and put it in a latter one
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			constexpr void execute_all()
			{
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt() * index_limit/*it's optimized right because unique value*/; i < (qt() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				if (has_multiple_blocks())
				{
					if (tsp_threads < tsp_array_size)
					{
						for (size_t ia = 0; ia < tsp_threads; ia++)//make N threads, where N would have mdl blocks inside
						{
							tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
						}
						if (mdl() != 0 && mdl() != (tsp_array_size - 1))//or &&tsp_threads!=1
						{
							tp[tsp_threads] = std::async(std::launch::async, [&]() {
								for (size_t i = ((qt() * tsp_threads) + 1); i < tsp_array_size; i++)
								{
									execute_index(i);
								}
								});//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads == tsp_array_size)//qt == 0
					{
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < tsp_array_size; i++)
						{
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads > tsp_array_size)//threads
					{
						for (size_t i = 1; i < tsp_array_size; i++)
						{
							tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < (tsp_threads + 1); i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L2_thread_version_instantiated_stack_function()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
		};

		template<const size_t tsp_array_size>
		struct L1_thread_version_instantiated_stack_function_nosplit {
			//local_ storage; - for these type you can use external thread_local
			bool active = true;

			constexpr bool has_multiple_blocks() { return tsp_array_size > 1; }
			std::array<std::function<void()>, tsp_array_size> image = { nullptr };
			std::array < std::future<void>, tsp_array_size> tp;
			//accept everything to image and for tp
			//csp is using stsd::async for executing while threads aren't created for each lambda

			constexpr void operator () (std::function<void()> execution_, size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0]();
				}
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos]();
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			void execute_all()
			{
				for (size_t i = 1; i < tsp_array_size; i++)
				{
					tp[i] = std::async(std::launch::async, image[i]);
				}
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < tsp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L1_thread_version_instantiated_stack_function_nosplit()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
			//notice the programming pattern how things get abstracted more the closer to end of struct
		};

		template<typename local_, const size_t tsp_array_size>
		struct L2_thread_version_instantiated_stack_function_nosplit {
			local_ storage;// something for function to access in a local scope
			bool active = true;

			std::array<std::function<void(local_&)>, tsp_array_size> image = { nullptr };
			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			std::array < std::future<void>, tsp_array_size> tp;

			constexpr void operator () (std::function<void(local_&)> execution_, size_t index)//make sure you are passing local_ as reference
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0](storage);
				}//in case people declared two slots and put it in a latter one
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			void execute_all()
			{
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < tsp_array_size; i++)
					{
						tp[i] = std::async(std::launch::async, image[i], storage);
					}
				}
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < tsp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L2_thread_version_instantiated_stack_function_nosplit()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
		};

		//lite version using function pointer instead of std::function for lesser ram
		template<const size_t tsp_threads, const size_t tsp_array_size/*maybe implement typename array_return_types but since these functions aren't usually expected to return anything not using it now*/>
		/// <summary>
		/// size_t is
		/// Alias of one of the fundamental unsigned integer types.
		/// depends on it
		/// </summary>
		struct L1_thread_version_instantiated_stack_function_lite_array {
			//local_ storage; - for these type you can use external thread_local
			bool active = true;

			std::array<void(*)(), tsp_array_size> image = { nullptr };
			//initiate all function ptr to nullptr so you have a state to check if valid
			//although the C++ language guarantees you that any object for which you do not provide an explicit initializer will be default initialized (C++11 §8.5/11).
			//Be aware that there are types for which default initialization has no effect and leaves the object's value indeterminate: any non-class, non-array type (§8.5/6). Consequently, a default-initialized array of objects with such types will have indeterminate value, e.g.:
			//I think they are default to nullptr here, but just besafe I am going to add it : = { nullptr }
			//https://stackoverflow.com/questions/18295302/default-initialization-of-stdarray

			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			constexpr size_t qt() { return (tsp_array_size - 1) / tsp_threads; }//compile time constant, if tsp_threads >= tsp_array_size qt == 0
			constexpr size_t mdl() { return (tsp_array_size - 1) % tsp_threads; }//compile time constant, if you lie about the array size to be larger than the functions you are putting in it's alright, if tsp_threads > tsp_array_size run every function in a single threads

			//constexpr size_t threads() {if (mdl != 0) { return tsp_threads + 1; } return tsp_threads;}//if multiple blocks can fit in specified tsp_threads, if not a new thread is created for redundent

			std::array < std::future<void>, tsp_threads + 1> tp;//threads pool. +1 for in case the task can't spread completely..
			//if I can figure out constexpr I can make array size not always neccesary to +1 when mdl == 0. - optimization

			constexpr void operator () (void(*execution_)(), size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0]();
				}
				///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
				///https://www.cplusplus.com/reference/vector/vector/operator[]/
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				//a pointer itself can be treated as bool too
				//https://stackoverflow.com/questions/17772103/can-i-use-if-pointer-instead-of-if-pointer-null
				if (image[pos])
				{
					image[pos]();
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			constexpr void execute_all()
			{
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt() * index_limit/*it's optimized right because unique value*/; i < (qt() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				if (has_multiple_blocks())
				{
					if (tsp_threads < tsp_array_size)
					{
						for (size_t ia = 0; ia < tsp_threads; ia++)//make N threads, where N would have mdl blocks inside
						{
							tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
						}
						if (mdl() != 0 && mdl() != (tsp_array_size - 1))//or &&tsp_threads!=1///this if block avoid uneccesary operations?
						{
							tp[tsp_threads] = std::async(std::launch::async, [&]() {
								for (size_t i = ((qt() * tsp_threads) + 1); i < tsp_array_size; i++)
								{
									execute_index(i);
								}
								});//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads == tsp_array_size)//qt == 0
					{
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < tsp_array_size; i++)
						{
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads > tsp_array_size)//threads
					{
						for (size_t i = 1; i < tsp_array_size; i++)
						{
							tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < (tsp_threads + 1); i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L1_thread_version_instantiated_stack_function_lite_array()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
			//notice the programming pattern how things get abstracted more the closer to end of struct
		};

		template<typename local_, const size_t tsp_threads, const size_t tsp_array_size>
		struct L2_thread_version_instantiated_stack_function_lite_array {
			local_ storage;// something for function to access in a local scope
			bool active = true;

			std::array<void(*)(local_&), tsp_array_size> image = { nullptr };;

			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			constexpr size_t qt() { return (tsp_array_size - 1) / tsp_threads; }//compile time constant
			constexpr size_t mdl() { return (tsp_array_size - 1) % tsp_threads; }//compile time constant

			//constexpr size_t threads() {if (mdl != 0) { return tsp_threads + 1; } return tsp_threads;}//if multiple blocks can fit in specified tsp_threads, if not a new thread is created for redundent

			std::array < std::future<void>, tsp_threads + 1> tp;//threads pool. +1 for in case the task can't spread completely..
			//= { nullptr };
			//because by default I can already cheack if future valid

			//if I can figure out constexpr I can make array size not always neccesary to +1 when mdl == 0. - optimization

			constexpr void operator () (void(*execution_)(local_&), size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0](storage);
				}
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			constexpr void execute_all()
			{
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt() * index_limit/*it's optimized right because unique value*/; i < (qt() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				if (has_multiple_blocks())
				{
					if (tsp_threads < tsp_array_size)
					{
						for (size_t ia = 0; ia < tsp_threads; ia++)//make N threads, where N would have mdl blocks inside
						{
							tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
						}
						if (mdl() != 0 && mdl() != (tsp_array_size - 1))//or &&tsp_threads!=1
						{
							tp[tsp_threads] = std::async(std::launch::async, [&]() {
								for (size_t i = ((qt() * tsp_threads) + 1); i < tsp_array_size; i++)
								{
									execute_index(i);
								}
								});//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads == tsp_array_size)//qt == 0
					{
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < tsp_array_size; i++)
						{
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads > tsp_array_size)//threads
					{
						for (size_t i = 1; i < tsp_array_size; i++)
						{
							tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < (tsp_threads + 1); i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L2_thread_version_instantiated_stack_function_lite_array()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
		};

		template<const size_t tsp_array_size>
		struct L1_thread_version_instantiated_stack_function_lite_array_nosplit {
			//local_ storage; - for these type you can use external thread_local
			bool active = true;

			constexpr bool has_multiple_blocks() { return tsp_array_size > 1; }
			std::array<void(*)(), tsp_array_size> image = { nullptr };
			std::array < std::future<void>, tsp_array_size> tp;
			//accept everything to image and for tp
			//csp is using stsd::async for executing while threads aren't created for each lambda

			constexpr void operator () (std::function<void()> execution_, size_t index)
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0]();
				}
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos]();
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			void execute_all()
			{
				for (size_t i = 1; i < tsp_array_size; i++)
				{
					tp[i] = std::async(std::launch::async, image[i]);
				}
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < tsp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L1_thread_version_instantiated_stack_function_lite_array_nosplit()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
			//notice the programming pattern how things get abstracted more the closer to end of struct
		};

		template<typename local_, const size_t tsp_array_size>
		struct L2_thread_version_instantiated_stack_function_lite_array_nosplit {
			local_ storage;// something for function to access in a local scope
			bool active = true;

			std::array<void(*)(local_&), tsp_array_size> image = { nullptr };
			constexpr bool has_multiple_blocks() { if (tsp_array_size > 1) { return true; } else { return false; } };
			std::array < std::future<void>, tsp_array_size> tp;

			constexpr void operator () (std::function<void(local_&)> execution_, size_t index)//make sure you are passing local_ as reference
			{
				image[index] = execution_;//take snapshot of the execution definition
				//execution_(storage);
			}
			void execute_M()
			{
				if (image[0] != nullptr)
				{
					image[0](storage);
				}//in case people declared two slots and put it in a latter one
			}//The main thread runing after async settle
		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
		public:
			constexpr void deactivate()
			{
				active = false;
			}
			void execute_all()
			{
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < tsp_array_size; i++)
					{
						tp[i] = std::async(std::launch::async, image[i], storage);
					}
				}
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					for (size_t i = 0; i < tsp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M();//execute Main thread
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			~L2_thread_version_instantiated_stack_function_lite_array_nosplit()
			{
				if (active != false)
				{
					execute_all_now();
				}
			}
		};


#define isf instantiated_stack_function
		//alias in case you wanna access struct
#define af(_T,_I,_A,_X) instantiated_stack_function<_T,_I> _A; _A ([&](_T& instx_lc)_X)
//instx_lc = instancex_local, potential elimination of std::function for performance? but the gain depends on implementation, and now you can use all variable available through lambda capture, if not it's gonna be strict.
#define af_nit(_T,_I,_A) instantiated_stack_function<_T,_I> _A
#define af_nit_overwrite(_T,_A,_X) _A ([](_T& instx_lc)_X)
#define af_nit_add(_T,_A,_X) _A ([](_T& instx_lc)_X)
#define af_exc(_A) _A.execute();_A.deactivate()
//execute all instances and finish all threads upon destruction
#define af_exc_dec(_T,_I,_A,_X) instantiated_stack_function<_T> _A; _A ([&](_T& instx_lc)_X);_A.execute();_A.deactivate()
//finish all threads upon declaration
#define lsp_ {
//linear_stack_phrase abstraction
#define lsp_end }
//linear_stack_phrase abstraction ending


//#define L1_tvisf L1_thread_version_instantiated_stack_function
//maybe implement L1 to only be using function pointer for lambda with no captures, or specialize all the classes -optimization
#define L1_taf(_P,_A,_t,_c,_X) L1_thread_version_instantiated_stack_function<_t,_c> _A; _A ([&]()_X,_P)
#define L1_taf_nit(_A,_t,_c) L1_thread_version_instantiated_stack_function<_t,_c> _A
#define L1_taf_ManTrigger(_A) _A.execute_all(); _A.finish_all()

#define L1_taf_napkin(_P,_A,_t,_c,_X) L1_thread_version_instantiated_stack_function_lite_array<_t,_c> _A; _A ([]()_X,_P)
#define L1_taf_napkin_nit(_A,_t,_c) L1_thread_version_instantiated_stack_function_lite_array<_t,_c> _A
#define L1_taf_napkin_ManTrigger(_A) _A.execute_all(); _A.finish_all()

#define L1_taf_nosplit(_P,_A,_c,_X) L1_thread_version_instantiated_stack_function_nosplit<_c> _A; _A ([]()_X,_P)
#define L1_taf_nosplit_nit(_A,_c) L1_thread_version_instantiated_stack_function_nosplit<_c> _A
#define L1_taf_nosplit_ManTrigger(_A) _A.execute_all(); _A.finish_all()

#define L1_taf_nosplit_napkin(_P,_A,_c,_X) L1_thread_version_instantiated_stack_function_lite_array_nosplit<_c> _A; _A ([]()_X,_P)
#define L1_taf_nosplit_napkin_nit(_A,_c) L1_thread_version_instantiated_stack_function_lite_array_nosplit<_c> _A
#define L1_taf_nosplit_napkin_ManTrigger(_A) _A.execute_all(); _A.finish_all()

//instx_lc = instancex_local, it's important to be able to access variables through capturing right
//taf = threaded_annonymous_function
//_T - emmited / _A - names / _X inline codes / _t define a maximum number of threads to use / _c size for async functions array / _P position to put function in array which is static and known and better checked before compile time for out of bound / _I - initial value for _T - deprecated
#define L1_taf_fold_var(_P,_A,_X) _A ([&]()_X,_P)//not a lambda but calling backend to pushback the function
#define L1_taf_add(_P,_A,_X)  _A ([&]()_X,_P)
#define L1_taf_napkin_fold_var(_P,_A,_X) _A ([]()_X,_P)//not a lambda but calling backend to pushback the function
#define L1_taf_napkin_add(_P,_A,_X)  _A ([]()_X,_P)
#define L1_taf_nosplit_fold_var(_P,_A,_X) _A ([&]()_X,_P)//not a lambda but calling backend to pushback the function
#define L1_taf_nosplit_add(_P,_A,_X)  _A ([&]()_X,_P)
#define L1_taf_nosplit_napkin_fold_var(_P,_A,_X) _A ([]()_X,_P)//not a lambda but calling backend to pushback the function
#define L1_taf_nosplit_napkin_add(_P,_A,_X)  _A ([]()_X,_P)

#define L1_taf_exc(_A) _A.execute_all()
#define L1_taf_napkin_exc(_A) _A.execute_all()
#define L1_taf_nosplit_exc(_A) _A.execute_all()
#define L1_taf_nosplit_napkin_exc(_A) _A.execute_all()
//L1 is a simpler struct that holds functions to execute but doesnt have locals
//execute all instances and finish all threads upon destruction

#define L1_taf_exc_dec(_P,_A,_t,_c,_X) L1_thread_version_instantiated_stack_function<_t,_c> _A; _A ([&]()_X,_P);_A.deactivate(); _A.execute_all_now()
#define L1_taf_napkin_exc_dec(_P,_A,_t,_c,_X) L1_thread_version_instantiated_stack_function_lite_array<_t,_c> _A; _A ([]()_X,_P);_A.deactivate(); _A.execute_all_now()
#define L1_taf_nosplit_exc_dec(_P,_A,_c,_X) L1_thread_version_instantiated_stack_function_nosplit<_c> _A; _A ([&]()_X,_P);_A.deactivate(); _A.execute_all_now()
#define L1_taf_nosplit_napkin_exc_dec(_P,_A,_c,_X) L1_thread_version_instantiated_stack_function_lite_array_nosplit<_c> _A; _A ([]()_X,_P);_A.deactivate(); _A.execute_all_now()

//finish all threads at declaration
#define L1_taf_core_duo_Inx(_A,_X1,_X2) L1_thread_version_instantiated_stack_function<2,2> _A; _A (_X1,0); _A (_X2,1)//Generate 2 threads, with one main thread that will conclude after the other ends, as in inheritancy
#define L1_taf_napkin_core_duo_INX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_lite_array<2,2> _A; _A (_X1,0); _A (_X2,1)//Generate 2 threads, with one main thread that will conclude after the other ends, as in inheritancy
#define L1_taf_core_duo_MX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function<2,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2) ///Generating 3 threads, leaving the main thread empty, and executing other two in parellel
#define L1_taf_napkin_core_duo_MX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_lite_array<2,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2) //;std::cout<<"zpYsPSom1L8Q3 This comment should not be included in output, blame compiler?"<<std::endl; ///Generating 3 threads, leaving the main thread empty, and executing other two in parellel
#define L1_taf_nosplit_core_duo_Inx(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_nosplit<2> _A; _A (_X1,0); _A (_X2,1)
#define L1_taf_nosplit_napkin_core_duo_INX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_lite_array_nosplit<2> _A; _A (_X1,0); _A (_X2,1)
#define L1_taf_nosplit_core_duo_MX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_nosplit<3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2)
#define L1_taf_nosplit_napkin_core_duo_MX(_A,_X1,_X2) L1_thread_version_instantiated_stack_function_lite_array_nosplit<3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2) 
//2 threads, 2 lines, customizable inline function futuristic design!

#define L1_tsp_ {
#define L1tsp_end }
//Napkin no capture

//#define L2_tvisf L2_thread_version_instantiated_stack_function
//#define L2_taf(_P,_T,_I,_A,_t,_c,_X) L2_thread_version_instantiated_stack_function<_T,_I,_t,_c> _A; _A ([&](_T& instx_lc)_X,_P)
#define L2_taf(_P,_T,_A,_t,_c,_X) L2_thread_version_instantiated_stack_function<_T,_t,_c> _A; _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_nit(_T,_A,_t,_c) L2_thread_version_instantiated_stack_function<_T,_t,_c> _A
#define L2_taf_ManTrigger(_A) _A.execute_all(); _A.finish_all()
#define L2_taf_napkin(_P,_T,_A,_t,_c,_X) L2_thread_version_instantiated_stack_function_lite_array<_T,_t,_c> _A; _A ([](_T& instx_lc)_X,_P)
#define L2_taf_napkin_nit(_T,_A,_t,_c) L2_thread_version_instantiated_stack_function_lite_array<_T,_t,_c> _A
#define L2_taf_napkin_ManTrigger(_A) _A.execute_all(); _A.finish_all()
#define L2_taf_nosplit(_P,_T,_A,_c,_X) L2_thread_version_instantiated_stack_function_nosplit<_T,_c> _A; _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_nosplit_nit(_A,_c) L2_thread_version_instantiated_stack_function_nosplit<_T,_c> _A
#define L2_taf_nosplit_ManTrigger(_A) _A.execute_all(); _A.finish_all()
#define L2_taf_nosplit_napkin(_P,_A,_c,_X) L2_thread_version_instantiated_stack_function_lite_array_nosplit<_T,_c> _A; _A ([](_T& instx_lc)_X,_P)
#define L2_taf_nosplit_napkin_nit(_A,_c) L2_thread_version_instantiated_stack_function_lite_array_nosplit<_T,_c> _A
#define L2_taf_nosplit_napkin_ManTrigger(_A) _A.execute_all(); _A.finish_all()
//instx_lc = instancex_local, it's important to be able to access variables through capturing right
//taf = threaded_annonymous_function
//_T - local data type / _A - names / _X inline codes / _t define a maximum number of threads to use / _c size for async functions array / _P position to put function in array which is static and known and better checked before compile time for out of bound / _I - initial value for _T - deprecated
#define L2_taf_fold_var(_P,_T,_A,_X) _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_add(_P,_T,_A,_X)  _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_napkin_fold_var(_P,_T,_A,_X) _A ([](_T& instx_lc)_X,_P)
#define L2_taf_napkin_add(_P,_T,_A,_X)  _A ([](_T& instx_lc)_X,_P)
#define L2_taf_exc(_A) _A.execute_all()
#define L2_taf_nosplit_fold_var(_P,_T,_A,_X) _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_nosplit_add(_P,_T,_A,_X)  _A ([&](_T& instx_lc)_X,_P)
#define L2_taf_nosplit_napkin_fold_var(_P,_T,_A,_X) _A ([](_T& instx_lc)_X,_P)
#define L2_taf_nosplit_napkin_add(_P,_T,_A,_X)  _A ([](_T& instx_lc)_X,_P)
//execute all instances and finish all threads upon destruction
#define L2_taf_exc_dec(_P,_T,_A,_t,_c,_X) L2_thread_version_instantiated_stack_function<_T,_t,_c> _A; _A ([&](_T& instx_lc)_X,_P);_A.deactivate(); _A.execute_all_now()
#define L2_taf_napkin_exc_dec(_P,_T,_A,_t,_c,_X) L2_thread_version_instantiated_stack_function_lite_array<_T,_t,_c> _A; _A ([](_T& instx_lc)_X,_P);_A.deactivate(); _A.execute_all_now()
#define L2_taf_nosplit_exc_dec(_P,_T,_A,_c,_X) L2_thread_version_instantiated_stack_function_nosplit<_T,_c> _A; _A ([]()_X,_P);_A.deactivate(); _A.execute_all_now()
#define L2_taf_nosplit_napkin_exc_dec(_P,_T,_A,_c,_X) L2_thread_version_instantiated_stack_function_lite_array_nosplit<_T,_c> _A; _A ([]()_X,_P);_A.deactivate(); _A.execute_all_now()
//finish all threads at declaration
#define L2_taf_core_duo_INX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function<_T,2,2> _A; _A (_X1,0); _A (_X2,1)
#define L2_taf_napkin_core_duo_INX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_lite_array<_T,2,2> _A; _A (_X1,0); _A (_X2,1)
#define L2_taf_core_duo_MX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function<_T,2,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2)
#define L2_taf_napkin_core_duo_MX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_lite_array<_T,2,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2)
#define L2_taf_nosplit_core_duo_Inx(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_nosplit<_T,2> _A; _A (_X1,0); _A (_X2,1)
#define L2_taf_nosplit_napkin_core_duo_INX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_lite_array_nosplit<_T,2> _A; _A (_X1,0); _A (_X2,1)
#define L2_taf_nosplit_core_duo_MX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_nosplit<_T,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2)
#define L2_taf_nosplit_napkin_core_duo_MX(_T,_A,_X1,_X2) L2_thread_version_instantiated_stack_function_lite_array_nosplit<_T,3> _A; _A ([](){},0); _A (_X1,1); _A (_X2,2) 
//2 threads, 2 lines, customizable inline function futuristic design!
#define L2_tsp_ {
#define L2tsp_end }
//And you can even change the internal resource like "storage" before L2tsp_end for all the functions which is going to execute them right at that point

//by the way type casting in template programming is kinda dangerous - get to it later perhaps?
	}
	//ALL Tasks are splited into the number of threads specified, while index 0 will always be the main thread, unless in some cases provided through macros hidden away and no longer running things in a conclusion manner
	//taf_napkin is using function pointer, while taf lets you use lambda capture and stuffs, and L2 let you besides using lambda capture able to access local variable in the virtual machine which was provided by you
	}