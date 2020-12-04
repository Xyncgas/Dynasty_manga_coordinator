#pragma once
// we are still using conclusions on L1_taf_series, although L1_dynamics is set for this option during rotate.
//when using these dynamic containers please use execute_all_now in place, and these structs must be accessed synchronously, it is practicals.

namespace LF
{
	namespace csp_dynamic
	{
		enum Execution_modes { reset = 0, replay };
		//for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		//Currently there is a copy for the enum in each of the dynamic container, usually yo would use this, but if sometimes you are implementing specific ones for the struts then perhaps yea.
		//Also I feel like it feels good to declare these strcuts and know what you are using when you are using them, that is when you are uses their intances as variables it gotta contain some type informations, so I can put metas in there and access them while coding
#define Struct_tag L1_thread_version_dynamic_container
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			//size_t tsp_array_size = 0;//rt bound

			std::vector<std::function<void()>> image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			size_t tsp_threads_ = 1;
			void set_default_number_for_threads(size_t t) { tsp_threads_ = t; }
		private:
			size_t qt_ = 0;
			void qt_set() { qt_ = (image.size() - 1) / tsp_threads_; }
			size_t mdl_ = 0;
			void mdl_set() { mdl_ = (image.size() - 1) % tsp_threads_; }
		public:
			size_t tsp_threads_get() { return tsp_threads_; }
			size_t qt_get() { return qt_; }
			size_t mdl_get() { return mdl_; }
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


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
			void safety_check()
			{
				assert(!(tsp_threads_get() == 0));
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (std::function<void()> execution_)
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0]();
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0]));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)//enums
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				//std::cout << "Available images :" << image.size()<<std::endl;
	//std::cout << "qt :" << qt_get()<<std::endl;
	//std::cout << "mdl :" << mdl_get() << std::endl;
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt_get() * index_limit/*it's optimized right because unique value*/; i < (qt_get() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				execute_M_merged();
				if (has_multiple_blocks())
				{
					if (tsp_threads_get() < image.size())
					{
						//std::cout << "inside has_multiple_blocks :1" << std::endl;
						for (size_t ia = 0; ia < tsp_threads_get(); ia++)//make N threads, where N would have mdl blocks inside
						{
							//std::cout << "A executing :" << ia << std::endl;
							//tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, threaded_Lambda, ia));
						}
						if (
							mdl_get() != 0
							&&
							//mdl_get() != (image.size() - 1)
							//tsp_threads_get()!=1
							mdl_get() != (image.size() - 1)//if tsp_threads_get()==1, mdl_get()==(image.size()-1)
							)
						{
							//tp[tsp_threads_get()] = std::async(std::launch::async, [&]() {
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&]() {
								for (size_t i = ((qt_get() * tsp_threads_get()) + 1); i < image.size(); i++)
								{
									execute_index(i);
								}
								}));//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads_get() == image.size())//qt == 0
					{
						//std::cout << "inside has_multiple_blocks :2" << std::endl;
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "B executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							//tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads_get() > image.size())//threads
					{
						//std::cout << "inside has_multiple_blocks :3" << std::endl;
						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "C executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					auto tp_array_size = tp.size();
					//std::cout << "Finishing tp :" << tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						//std::cout << "Finishing at :" << i << std::endl;
						if (tp[i].valid())
						{
							//std::cout << "Valid_check passed :" << i << std::endl;
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
							//std::cout << "get() called :" << i << std::endl;
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			void Reserve_vanilla(size_t k, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(k);
				tp.reserve(thread_count + 1);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k, size_t thread_count = 0)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
					if (tp.capacity() < thread_count)//extra check
					{
						tp.reserve(thread_count + 1);
					}
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + thread_count);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					qt_set();
					mdl_set();
					std::get<1>(engaging) = true;
				}
			}//set concurrency through this public method because these three variables are bonded together and is calculated while program is running.
			~L1_thread_version_dynamic_container()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L1_dynamic_async_container_T_

#define Struct_tag L1_thread_version_dynamic_container_lite
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			//size_t tsp_array_size = 0;//rt bound

			std::vector<void(*)()>image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			size_t tsp_threads_ = 1;
			void set_default_number_for_threads(size_t t) { tsp_threads_ = t; }
		private:
			size_t qt_ = 0;
			void qt_set() { qt_ = (image.size() - 1) / tsp_threads_; }
			size_t mdl_ = 0;
			void mdl_set() { mdl_ = (image.size() - 1) % tsp_threads_; }
		public:
			size_t tsp_threads_get() { return tsp_threads_; }
			size_t qt_get() { return qt_; }
			size_t mdl_get() { return mdl_; }
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready
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
			void safety_check()
			{
				assert(!(tsp_threads_get() == 0));
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (void(*execution_)())
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0]();
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0]));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				//std::cout << "Available images :" << image.size()<<std::endl;
				//std::cout << "qt :" << qt_get()<<std::endl;
				//std::cout << "mdl :" << mdl_get() << std::endl;
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt_get() * index_limit/*it's optimized right because unique value*/; i < (qt_get() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				execute_M_merged();
				if (has_multiple_blocks())
				{
					if (tsp_threads_get() < image.size())
					{
						//std::cout << "inside has_multiple_blocks :1" << std::endl;
						for (size_t ia = 0; ia < tsp_threads_get(); ia++)//make N threads, where N would have mdl blocks inside
						{
							//std::cout << "A executing :" << ia << std::endl;
							//tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, threaded_Lambda, ia));
						}
						if (
							mdl_get() != 0
							&&
							//mdl_get() != (image.size() - 1)
							//tsp_threads_get()!=1
							mdl_get() != (image.size() - 1)//if tsp_threads_get()==1, mdl_get()==(image.size()-1)
							)
						{
							//tp[tsp_threads_get()] = std::async(std::launch::async, [&]() {
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&]() {
								for (size_t i = ((qt_get() * tsp_threads_get()) + 1); i < image.size(); i++)
								{
									execute_index(i);
								}
								}));//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads_get() == image.size())//qt == 0
					{
						//std::cout << "inside has_multiple_blocks :2" << std::endl;
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "B executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							//tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads_get() > image.size())//threads
					{
						//std::cout << "inside has_multiple_blocks :3" << std::endl;
						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "C executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					auto tp_array_size = tp.size();
					//std::cout << "Finishing tp :" << tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						//std::cout << "Finishing at :" << i << std::endl;
						if (tp[i].valid())
						{
							//std::cout << "Valid_check passed :" << i << std::endl;
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
							//std::cout << "get() called :" << i << std::endl;
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			void Reserve_vanilla(size_t k, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(k);
				tp.reserve(thread_count + 1);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k, size_t thread_count = 0)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
					if (tp.capacity() < thread_count)//extra check
					{
						tp.reserve(thread_count + 1);
					}
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + thread_count);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					qt_set();
					mdl_set();
					std::get<1>(engaging) = true;
				}
			}//set concurrency through this public method because these three variables are bonded together and is calculated while program is running.
			~L1_thread_version_dynamic_container_lite()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L1_dynamic_async_lite_container_T_

#define Struct_tag L2_thread_version_dynamic_container
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			std::any storage;

			//size_t tsp_array_size = 0;//rt bound

			std::vector<std::function<void(std::any)>> image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			size_t tsp_threads_ = 1;
			void set_default_number_for_threads(size_t t) { tsp_threads_ = t; }
		private:
			size_t qt_ = 0;
			void qt_set() { qt_ = (image.size() - 1) / tsp_threads_; }
			size_t mdl_ = 0;
			void mdl_set() { mdl_ = (image.size() - 1) % tsp_threads_; }
		public:
			size_t tsp_threads_get() { return tsp_threads_; }
			size_t qt_get() { return qt_; }
			size_t mdl_get() { return mdl_; }
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is 
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				//a pointer itself can be treated as bool too
				//https://stackoverflow.com/questions/17772103/can-i-use-if-pointer-instead-of-if-pointer-null
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
			void safety_check()
			{
				assert(!(tsp_threads_get() == 0));
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (std::function<void(std::any)> execution_)
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0](storage);
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0], storage));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				//std::cout << "Available images :" << image.size()<<std::endl;
				//std::cout << "qt :" << qt_get()<<std::endl;
				//std::cout << "mdl :" << mdl_get() << std::endl;
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt_get() * index_limit/*it's optimized right because unique value*/; i < (qt_get() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				execute_M_merged();
				if (has_multiple_blocks())
				{
					if (tsp_threads_get() < image.size())
					{
						//std::cout << "inside has_multiple_blocks :1" << std::endl;
						for (size_t ia = 0; ia < tsp_threads_get(); ia++)//make N threads, where N would have mdl blocks inside
						{
							//std::cout << "A executing :" << ia << std::endl;
							//tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, threaded_Lambda, ia));
						}
						if (
							mdl_get() != 0
							&&
							//mdl_get() != (image.size() - 1)
							//tsp_threads_get()!=1
							mdl_get() != (image.size() - 1)//if tsp_threads_get()==1, mdl_get()==(image.size()-1)
							)
						{
							//tp[tsp_threads_get()] = std::async(std::launch::async, [&]() {
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&]() {
								for (size_t i = ((qt_get() * tsp_threads_get()) + 1); i < image.size(); i++)
								{
									execute_index(i);
								}
								}));//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads_get() == image.size())//qt == 0
					{
						//std::cout << "inside has_multiple_blocks :2" << std::endl;
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "B executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							//tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads_get() > image.size())//threads
					{
						//std::cout << "inside has_multiple_blocks :3" << std::endl;
						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "C executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					auto tp_array_size = tp.size();
					//std::cout << "Finishing tp :" << tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						//std::cout << "Finishing at :" << i << std::endl;
						if (tp[i].valid())
						{
							//std::cout << "Valid_check passed :" << i << std::endl;
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
							//std::cout << "get() called :" << i << std::endl;
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			void Reserve_vanilla(size_t k, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(k);
				tp.reserve(thread_count + 1);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k, size_t thread_count = 0)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
					if (tp.capacity() < thread_count)//extra check
					{
						tp.reserve(thread_count + 1);
					}
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + thread_count);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					qt_set();
					mdl_set();
					std::get<1>(engaging) = true;
				}
			}//set concurrency through this public method because these three variables are bonded together and is calculated while program is running.
			~L2_thread_version_dynamic_container()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L2_dynamic_async_container_T_

#define Struct_tag L2_thread_version_dynamic_container_lite
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			std::any storage;

			//size_t tsp_array_size = 0;//rt bound

			std::vector<void(*)(std::any)>image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			size_t tsp_threads_ = 1;
			void set_default_number_for_threads(size_t t) { tsp_threads_ = t; }
		private:
			size_t qt_ = 0;
			void qt_set() { qt_ = (image.size() - 1) / tsp_threads_; }
			size_t mdl_ = 0;
			void mdl_set() { mdl_ = (image.size() - 1) % tsp_threads_; }
		public:
			size_t tsp_threads_get() { return tsp_threads_; }
			size_t qt_get() { return qt_; }
			size_t mdl_get() { return mdl_; }
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				//a pointer itself can be treated as bool too
				//https://stackoverflow.com/questions/17772103/can-i-use-if-pointer-instead-of-if-pointer-null
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
			void safety_check()
			{
				assert(!(tsp_threads_get() == 0));
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (void(*execution_)(std::any))
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0](storage);
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0], storage));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				//std::cout << "Available images :" << image.size()<<std::endl;
				//std::cout << "qt :" << qt_get()<<std::endl;
				//std::cout << "mdl :" << mdl_get() << std::endl;
				auto threaded_Lambda = [&](size_t index_limit) {
					for (size_t i = 1 + qt_get() * index_limit/*it's optimized right because unique value*/; i < (qt_get() * (index_limit + 1)/*next steps*/) + 1; i++)//i=1 because 0 reserved for main threads
					{
						execute_index(i);
					}
				};//will not get execute if qt==0
				execute_M_merged();
				if (has_multiple_blocks())
				{
					if (tsp_threads_get() < image.size())
					{
						//std::cout << "inside has_multiple_blocks :1" << std::endl;
						for (size_t ia = 0; ia < tsp_threads_get(); ia++)//make N threads, where N would have mdl blocks inside
						{
							//std::cout << "A executing :" << ia << std::endl;
							//tp[ia] = std::async(std::launch::async, threaded_Lambda, ia);
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, threaded_Lambda, ia));
						}
						if (
							mdl_get() != 0
							&&
							//mdl_get() != (image.size() - 1)
							//tsp_threads_get()!=1
							mdl_get() != (image.size() - 1)//if tsp_threads_get()==1, mdl_get()==(image.size()-1)
							)
						{
							//tp[tsp_threads_get()] = std::async(std::launch::async, [&]() {
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&]() {
								for (size_t i = ((qt_get() * tsp_threads_get()) + 1); i < image.size(); i++)
								{
									execute_index(i);
								}
								}));//possible bug - careful exceed boundary - tp[tsp_threads]
						}

					}//can be evaluated at compile times
					else if (tsp_threads_get() == image.size())//qt == 0
					{
						//std::cout << "inside has_multiple_blocks :2" << std::endl;
						//treat (tsp_threads == tsp_array_size) the same as (tsp_threads > tsp_array_size)
						//except threaded_Lambda wont work tho
						//but by definition every function will run in their own threads now


						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "B executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, execute_index, i); -how to use thread with a member function -let me think
							//tp[i] = std::async(std::launch::async, [&, i/*if i not using copying it is going to crash - bug fix*/]() {execute_index(i); });//executing index from image array
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
							//tp.size would be image.size + 1, so as long i<tsp_array_size it's in bound (in fact 1 extra too)
						}
					}//previously calculated information is in a state which is usless because of design, catching & addressing the situation now.
					else if (tsp_threads_get() > image.size())//threads
					{
						//std::cout << "inside has_multiple_blocks :3" << std::endl;
						for (size_t i = 1; i < image.size(); i++)
						{
							//std::cout << "C executing :" << i << std::endl;
							//tp[i] = std::async(std::launch::async, [&, i]() {execute_index(i); });
							///Notice how these lines are replaced by v_back in general
							tp.v_back(std::async(std::launch::async, [&, i]() {execute_index(i); }));
						}
					}

				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					auto tp_array_size = tp.size();
					//std::cout << "Finishing tp :" << tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
						//yea because array position match tp, don't use tp's size, but actual code block sizes - >deprecated
						//would crash if tsp_array_sizes is 1 or larger than tsp_threads
						//using the container's own sizes now
					{
						//std::cout << "Finishing at :" << i << std::endl;
						if (tp[i].valid())
						{
							//std::cout << "Valid_check passed :" << i << std::endl;
							//maybe use a lambda here for checking ready and joining, using std::future::wait, and if something not ready yet you call the next future::get and while waiting for the thread to be ready and remembering to join all waiting at the end either by threading the lambda or using if statements but still I think it would improve performance
							//or uneccesary? but maybe it helps the implementation too.
							//I mean it's the endpoint for all threads to be resolved here tho.
							tp[i].get();
							//std::cout << "get() called :" << i << std::endl;
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{

				execute_all();
				finish_all();

			}
			void Reserve_vanilla(size_t k, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(k);
				tp.reserve(thread_count + 1);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k, size_t thread_count = 0)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
					if (tp.capacity() < thread_count)//extra check
					{
						tp.reserve(thread_count + 1);
					}
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c, size_t thread_count = 0)
			{
				if (thread_count != 0)
				{
					set_default_number_for_threads(thread_count);//passing thread_count by reference
				}
				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + thread_count);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					qt_set();
					mdl_set();
					std::get<1>(engaging) = true;
				}
			}//set concurrency through this public method because these three variables are bonded together and is calculated while program is running.
			~L2_thread_version_dynamic_container_lite()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L2_dynamic_async_lite_container_T_


#define Struct_tag L1_thread_version_nosplit_dynamic_container
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			//size_t tsp_array_size = 0;//rt bound

			std::vector<std::function<void()>> image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes

		public:
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


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
			void safety_check()
			{
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (std::function<void()> execution_)
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0]();
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0]));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				execute_M_merged();
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < image.size(); i++)
					{
						tp.v_back(std::async(std::launch::async, image[i]));
					}
				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					auto tp_array_size = tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously, there can be one less task available in tp, because M_thread wasn't pushed and is executed here sometimes
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{

				execute_all();
				finish_all();

			}
			void Reserve_vanilla(size_t k)
			{
				image.reserve(k);
				tp.reserve(k);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (tp.capacity() < k)
				{
					tp.reserve(k);
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c)
			{

				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + c);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					std::get<1>(engaging) = true;
				}
			}
			~L1_thread_version_nosplit_dynamic_container()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L1_dynamic_async_nosplit_container_T_

#define Struct_tag L1_thread_version_nosplit_dynamic_container_lite
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			//size_t tsp_array_size = 0;//rt bound

			std::vector<void(*)()>image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


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
			void safety_check()
			{
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (void(*execution_)())
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0]();
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0]));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				execute_M_merged();
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < image.size(); i++)
					{
						tp.v_back(std::async(std::launch::async, image[i]));
					}
				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					//maybe fence - BUG?
					auto tp_array_size = tp.size();
					//std::cout << "Finishing tp :" << tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
					{
						//std::cout << "Finishing at :" << i << std::endl;
						if (tp[i].valid())
						{
							tp[i].get();
							//std::cout << "get() called :" << i << std::endl;
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			void Reserve_vanilla(size_t k)
			{
				image.reserve(k);
				tp.reserve(k);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (tp.capacity() < k)
				{
					tp.reserve(k);
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c)
			{

				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + c);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					std::get<1>(engaging) = true;
				}
			}
			~L1_thread_version_nosplit_dynamic_container_lite()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L1_dynamic_async_nosplit_lite_container_T_

#define Struct_tag L2_thread_version_nosplit_dynamic_container
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			std::any storage;

			//size_t tsp_array_size = 0;//rt bound

			std::vector<std::function<void(std::any)>> image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				//a pointer itself can be treated as bool too
				//https://stackoverflow.com/questions/17772103/can-i-use-if-pointer-instead-of-if-pointer-null
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
			void safety_check()
			{
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (std::function<void(std::any)> execution_)
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0](storage);
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0], storage));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				execute_M_merged();
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < image.size(); i++)
					{
						tp.v_back(std::async(std::launch::async, image[i], storage));
					}
				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					auto tp_array_size = tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{

				execute_all();
				finish_all();

			}
			void Reserve_vanilla(size_t k)
			{
				image.reserve(k);
				tp.reserve(k);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (tp.capacity() < k)
				{
					tp.reserve(k);
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c)
			{

				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + c);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					std::get<1>(engaging) = true;
				}
			}
			~L2_thread_version_nosplit_dynamic_container()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L2_dynamic_async_nosplit_container_T_

#define Struct_tag L2_thread_version_nosplit_dynamic_container_lite
		struct Struct_tag {
			//reuse interface for presisdent data structure for parelle tasks
			///Validation (whether if executed) is considered broken down as another responsibility for caller for finer grained performance
			///It is however sometimes unable to know the spots needed for threads at compile time, either you give operator pushback capability(for using data type already inside vector for tracking its size automatically), or you one way or another provide a safety check method for growing the vector
			///(invalid)So the pattern is basically, let the caller do all the validation, and provide methods that ask for useful information to optimize which you could do externally anyways.
			///(invalid) The reason I don't just use push back in the () operator is that even though it's automatic and the doubling capacity logic is viable, still if I can make capacity as much as needed it saves space, while providing errors to say coder did it wrong n there is usually enough information for you to reserve minimally
			///I am using emplace back now, current flow is to use std::vector::clear() in Reset before assembling, and either use reserve or skip reserve during filling and afterward use rotate before the executions and either automatically clearing vector or manually by calling Reset_engage
			///Notice how some the position based assign for image and tp in CSP original version is not using that informations anymore, and is replaced by v_back, it generally still works that is.

		public:
			std::any storage;

			//size_t tsp_array_size = 0;//rt bound

			std::vector<void(*)(std::any)>image;//L1 takes no function param
			///Mutable in elements

			std::vector<std::future<void>> tp;
			///Mutable in elements

			//meta field - compute everytime when tsp_threads changes
		public:
			bool has_multiple_blocks() { if (image.size() > 1) { return true; } else { return false; } };


			//Control signals
			bool active = false;//control deconstructor execution
			bool Use_conclusion_model_ = true; void conclusion_model_set(bool& input) { Use_conclusion_model_ = input; } bool conclusion_model_get() { return Use_conclusion_model_; }

		private:
			std::tuple<bool, bool> engaging = { false,false };
			//it's like airplain taking off, first bool past and then seocnd bool past and etc, 
			//first ping from the _RESERVE_, then do the things you do,
			//then _ROTATE_ and set array_size from image.size() while calling other variables' set functions and it's ready for execution, which reset by using the public function inside execute_all_now and available to caller that allows further exucutions
		public:
			enum Execution_modes { reset = 0, replay }; //for execute_all(), because if you are trying to use all the images again, you don't have to do the skip reserve/reserve to set readiness signal which by default would be reset after finish_all() is called
		public:
			void Reset_engage() { image.clear(); tp.clear(); engaging = { false,false }; }
			void Skip_reserve/*Bypass_v1*/() { std::get<0>(engaging) = true; }//usually v1 is achieved by using reserve no matter what, you can skip the process if you have confident that your program won't crash while you are adding those pointers, since there's no push_back implementations and the only way to use is to assign function pointers directly into array after reserving it.


			//bool recyle = false;//signal recycle ready


		private:
			void execute_index(size_t pos)
			{
				//std::function itself can be a bool value for checking if it has a target assinged to it
				//a pointer itself can be treated as bool too
				//https://stackoverflow.com/questions/17772103/can-i-use-if-pointer-instead-of-if-pointer-null
				if (image[pos])
				{
					image[pos](storage);
				}
			}//execute blocks stored in array, for a given position, an abstraction used for internal calls
			void safety_check()
			{
				assert(!(image.size() == 0));
				//run time aseert, if evaluated as false then abort();
			}//useed inside building
			bool engaging_readiness_check()
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == true)
				{
					return true;
				}
				else
				{
					std::cerr << "Violation of procedure :Unsatisfied running condition, in check {47C48E15-2AD2-4997-AA7C-6ED5B3FBB911}." << std::endl;
					return false;
				}
			}
		public:
			void operator () (void(*execution_)(std::any))
			{
				image.v_back(execution_);//take snapshot of the execution definition
			}//Again, letting caller set index id can ensure we aren't always pushing or checking everytime for the validity, and there is a safer version that's like see if you are overwriting tasks I am not doing it
		private:
			void execute_M_conclusion()
			{
				if (conclusion_model_get() == true)
				{
					//std::cout << "execute_M_conclusion" << std::endl;
					if (image[0] != nullptr)
					{
						image[0](storage);
					}
					///the std::vector[] operator returns a reference, therefore if the element doesnt exist or this position is out of bound, you are still able to get a reference as long you are not touching it or modifying the element it's not throwing which is nullptr
					///https://www.cplusplus.com/reference/vector/vector/operator[]/
				}
			}//The main thread runing after async settle in finish_all(), although I am making this public, thinking that it might do some magic
			void execute_M_merged()
			{
				if (conclusion_model_get() == false)
				{
					//std::cout << "execute_M_merged" << std::endl;
					if (image[0] != nullptr)
					{
						tp.v_back(std::async(std::launch::async, image[0], storage));
					}
				}
			}//running in executeall(), if available
		public:
			void deactivate()
			{
				active = false;
			}
			void execute_all(Execution_modes modes = Execution_modes::reset)
			{
				if (modes == 0)
				{
					if (engaging_readiness_check() != true)
					{
						return;//by default if unsatisfied return.
					}
				}
				else if (modes == 1)
				{
					//proceeds
				}
				execute_M_merged();
				if (has_multiple_blocks())
				{
					for (size_t i = 1; i < image.size(); i++)
					{
						tp.v_back(std::async(std::launch::async, image[i], storage));
					}
				}//check if there's other block other than main
			}//execute all, for invalid target, checked, threading here, catch on later
			void finish_all()
			{
				if (has_multiple_blocks())
				{
					auto tp_array_size = tp.size();
					for (size_t i = 0; i < tp_array_size; i++)
					{
						if (tp[i].valid())
						{
							tp[i].get();
						}
					}//get all async states involked previously
				}

				execute_M_conclusion();//execute Main thread
				Reset_engage();
			}
			void execute_all_now()
			{
				execute_all();
				finish_all();
			}
			void Reserve_vanilla(size_t k)
			{
				image.reserve(k);
				tp.reserve(k);
				std::get<0>(engaging) = true;
			}//Generally use this on initialize or you know what you are doing, type A, high confidence usage
			void Reserve_expected(size_t k)
			{
				if (image.capacity() < k)
				{
					image.reserve(k);
				}
				if (tp.capacity() < k)
				{
					tp.reserve(k);
				}
				std::get<0>(engaging) = true;
			}//Generally use this on uncertaint cases, type B, low confidance usage, usually after instance available for overwrite, targetting a capacity, if not fulfill then increase to
			void Reserve_increase(size_t c)
			{

				image.reserve(image.capacity() + c);
				tp.reserve(tp.capacity() + c);
				std::get<0>(engaging) = true;
			}//you could check capacity with c externally, and see if you need to expand capacity, or you can use this method, and by default, it doesn't change tp.size()
			void Rotate(bool Use_conclusion_model = true)/*V1/Vr/V2*/
			{
				if (std::get<0>(engaging) == true && std::get<1>(engaging) == false)
				{
					if (Use_conclusion_model != conclusion_model_get())
					{
						conclusion_model_set(Use_conclusion_model);
					}//you can optimize this for assign intensive or non intensive task, if you know there's a lot assign because the kind of tasks is unpreditable, well still maybe use a pool to sort them into similar kind, and yes know this too, if you are not in situations where you are changing the values frequenly then you can use a check to throw away assign
					safety_check();//make sure they aren't zero
					std::get<1>(engaging) = true;
				}
			}
			~L2_thread_version_nosplit_dynamic_container_lite()
			{
				if (active == true)
				{
					execute_all_now();
					//Reset_engage(); - nothing matters if struct destruct
				}
			}
		};
#undef Struct_tag
#define L2_dynamic_async_nosplit_lite_container_T_
	}
}
/// Comments about this header file :
/// There's Struct_tag which is a macro for struct names, I left the deconstructor to the original struct name, so people don't accidentally misplaced these codes
/// For dynamic_async_containers like these, execution at the end of life cycle is disabled (Through setting bool active = false; for these struct ). Because they are meant to be use through the interface elegancy (Reserve/skip reserve, fill tasks, rotate!)
/// Idealy, when using these structs you say how many threads you are to use by default
/// If you don't define it, that's fine, you can define the concurency in the reserve process
/// If you miss that chance, you can still use the module because it totally works (which is why there's an option for skip reserve), either if conciously, memory automation presents, std vector, it just means the concurrency number doesnt change and by default will be 1.
/// I didn't implement enum for execute all now because I thought the interface shall be more explicit, that's calling skip_reserve or reserving, which is the optimal way and less easier to use it wrong...