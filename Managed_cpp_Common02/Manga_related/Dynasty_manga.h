#pragma once
using namespace LF::csp_dynamic;
using namespace LF::csp;
namespace Models
{
	namespace Common02
	{
		struct Dynasty_manga_
		{
			//struct meta_io
			//{
			struct Config_files_
			{
				std::fstream Series_subscription;
				std::ifstream Param_lists;
				std::string Update_resides_in_this_folder;
				std::string Content_folder;
				Config_files_(std::string&& Series_url_locate = "", std::string&& Program_param_locate = "", std::string&& Update_folder_locate = "")/*&& is for rvalue apparently and you can use std::move()*/
				{
					if (!Series_url_locate.empty() && !Program_param_locate.empty())
					{
						Series_subscription.open(Series_url_locate, std::fstream::in | std::fstream::out | std::fstream::app);
						Param_lists.open(Program_param_locate);
					}
					Update_resides_in_this_folder = Update_folder_locate;
					//opening in and out and append to end for writing new stuffs
				}
			};
			//};
			//for steam elements always require a pointer for intances, so it was initialized or maybe without and you can check if.
			//although, please try not to do inheritance
			//it's a headach if you are going more than one level to initialize these variables in meta_
			struct meta_
			{
				L1_thread_version_dynamic_container* L1_dynamic_async_inx;
				//meta_io* io;
				Config_files_* Config_files;
			};//for something like a steam that runs through all the control paths
		public:
			inline static std::wstring Koshiki_url_utf16 = U("https://dynasty-scans.com");
			inline static std::string Koshiki_url_utf8 = "https://dynasty-scans.com";
			inline static nlohmann::json Json_container_for_series_meta;//series are looped through synchronously
			inline static nlohmann::json Json_container_for_locally_cached_series_meta;
			inline static nlohmann::json Json_container_for_program_config;
			//if something is static, it has to have a cpp file for the declaration? other wise unresolved externals?
			//If using older versions of the C++ standard, you must add the definitions to match your declarations
			//https://stackoverflow.com/questions/195207/unresolved-external-symbol-on-static-class-members
			//can be bypassed
			//by using the inline keyword (c++ 17)

			//it's fine I feel like to have non-static functions even though they can be static, because although these things carries the information/awareness for their enviornment the performance impact is minimal because the function body is there it wouldn't be copied many times.

			std::wstring Series_processor(std::wstring& input)
			{
				auto action = [](std::any tmp, std::any collector, bool* stop_running_me)
				{
					/// <summary>
					/// params will be given as pointers, cast them to pointers please
					/// </summary>
					/// <param name="input"></param>
					/// <returns></returns>
					auto str = std::any_cast<std::wstring*>(tmp);
					auto res = Common_functions::Common01::Get_str_between_wstr(*str, L"<a href=\"", L"\" class=\"name\">");
					if (!res.empty())
					{
						auto pog = (std::any_cast<std::wstring*>(collector));
						*pog += res;
						*pog += '\n';
					}
				};
				std::wstring collector;
				Common_functions::Common01::Imagine_str_lp(nullptr, &input, action, &collector);
				Common_functions::Common01::String_pop_lastn(nullptr, &collector, nullptr);

				return collector;

			}

			std::wstring Chapters_processor(std::wstring& input)
			{
				//using reference input for orchestrator type pipelining
				auto action = [](std::any tmp, std::any collector, bool* stop_running_me)
				{

					auto str = std::any_cast<std::wstring*>(tmp);
					auto pos = (*str).find(L"var pages = [{\"image\":\"");
					//if (pos != (*str).npos)//might be slow to dereference but easier to read
					if (pos != std::wstring::npos)
					{
						//fcout("found");
						auto a = Common_functions::Common01::Get_str_lists_between_wstr(*str, L"\"image\":\"", L"\",\"name\":\"");
						auto pog = (std::any_cast<std::wstring*>(collector));
						for (size_t i = 0; i < a.size(); i++)
						{
							(*pog) += Koshiki_url_utf16;
							(*pog) += a[i];
							(*pog) += '\n';
						}
						*stop_running_me = true;
					}
				};
				std::wstring collector;
				Common_functions::Common01::Imagine_str_lp(nullptr, &input, action, &collector);
				Common_functions::Common01::String_pop_lastn(nullptr, &collector, nullptr);
				return collector;
			}


			static void probe_chapters(std::string Series_name, std::string perma_link, meta_* steam = nullptr)
			{
#ifndef disable_switch_for_debugging_purpose
				fcout("\n\n********************\nProbe_chapters"<<std::endl);
				//fcout(chapters_url);
				//fcout("probe_chapters running on thread :"<<std::this_thread::get_id());

				//construct the pieces to form a string url
				std::ostringstream string_builder;
				string_builder << Koshiki_url_utf8 << "/chapters/" << perma_link << ".json";

				//make network request through HttpGetString, parse returned message to json
				utility::string_t network_response; nlohmann::json Json_container_for_chapters_meta;
				try
				{
					network_response = Common_functions::Common01::HttpGetString(string_builder.str()); Json_container_for_chapters_meta = nlohmann::json::parse(network_response);
				}
				catch(std::exception e)
				{
					std::cerr<<"Failure to process network request at:" << string_builder.str();
					return;
				}

				if (!Json_container_for_chapters_meta["pages"].is_null()&&!Json_container_for_chapters_meta["permalink"].is_null())
				{
					std::string Chapter_name = Json_container_for_chapters_meta["permalink"];//please don't put this inline because if I do it this way the json library would output raw datas where as if I use it in some sort of implicit casting liike using it in ostringstream output_filename it would preserve the double quote for json value which is bad for filenames
					std::ostringstream chapter_folder_location;
					chapter_folder_location << steam->Config_files->Content_folder << "/" << Series_name << "/" << Chapter_name;
					fcout("Creating folder :" << chapter_folder_location.str());
					Common_functions::Common01::Create_folder(std::move(chapter_folder_location));
					//fcout("Finished creating folder");
					L1_thread_version_dynamic_container inx;
					inx.Reserve_expected(Json_container_for_chapters_meta["pages"].size(), 9);
					for (auto element : Json_container_for_chapters_meta["pages"])
					{
						inx([&, element]()
							{
								if (element.contains("name")/*not using this as filename though, it's needed for api validation, still I would get the file name directly on url*/ && element.contains("url"))
								{
									if (!element["url"].is_null())
									{

										//create string builder and temporary string, for storing target output file_path and getting the filename based on url given 
										std::ostringstream output_filename; std::string dynasty_system_paths = element["url"]; std::string web_filename_str = dynasty_system_paths.substr(dynasty_system_paths.find_last_of("/") + 1);
										output_filename << steam->Config_files->Content_folder << "/" << Series_name << "/" << Chapter_name << "/" << web_filename_str;
										//make network request and download file using this function originally put to gether utilizing casablanca(cpprestsdk)'s capabilities
										//fwcout("A :" << Koshiki_url_utf16);
										//fcout("B :" << dynasty_system_paths);
										//fwcout("C :" << utility::conversions::utf8_to_utf16(output_filename.str()));
										try
										{
											fcout("output target :" << output_filename.str());
											fcout("At :" << dynasty_system_paths);
											fcout('\n');
											auto res = Common_functions::Common01::HttpGetStreambuf_toFile(Koshiki_url_utf16, utility::conversions::utf8_to_utf16(dynasty_system_paths), utility::conversions::utf8_to_utf16(output_filename.str()));
											//it would throw when directory doesnt exist, it can create file though. see Concurrency::streams::fstream::open_ostream
											if (res.has_value())
											{
												if (res.value() == 101)
												{
													std::wcerr << "Detected interal error :request failed because invalid url at :"<<Koshiki_url_utf16<< utility::conversions::utf8_to_utf16(dynasty_system_paths) << std::endl;
													abort();
												}
											}
										}//try blocks for normal route so it doesn't use it usually, if you have to check for errors, use if statement at the top of control apths.
										catch (std::exception e)
										{
											std::cerr << "Caught exception when downloading file from Dynasty-manga :" << Koshiki_url_utf8 << dynasty_system_paths << ". Code(#12938)";
											throw;
											return;
										}
									}
								}
							});
					}
					inx.Rotate();
					inx.execute_all();
				}
				else
				{
					//std::exception e("Chapter_probing_exception :api mismatch panick");
					//throw e;
					std::cerr << "Eh :parse sucess but mismatch network response message json structure :url_str :" << string_builder.str() << std::endl;//if string literal doesn't have L macro (for wstring) you get run time error on release x64 builds even you are not trigerring it (or perhaps I am mistakened)
					std::cerr << Json_container_for_chapters_meta.dump() << std::endl;
					throw("Chapter_probing_exception :parse sucess but mismatch network response message json structure (std::cerr output available) :api mismatch panick");
					//these statements even though not usually triggering and thus free it's still occupying stack memory.
				}
#endif
			}
			static void probe_series(std::string series_url, meta_* steam=nullptr)//Better use ptr for steam because you are copying it for the lambda
			{
				fcout("Probing_series :"<<series_url);
#if 1
				auto url_str = utility::conversions::utf8_to_utf16(series_url);
				uri URL; URL = web::uri::encode_uri(url_str);
				auto network_response =  Common_functions::Common01::HttpGetString(URL); Json_container_for_series_meta = nlohmann::json::parse(network_response);
				{
					//safety check, sometimes if statement is importnat, don't just flip the statement for testing
					try
					{
						if (!Json_container_for_series_meta["taggings"].is_null() && !Json_container_for_series_meta["name"].is_null())
						{
							//for (int i = 0; i < 10; i++)
							//{
								//ct_start
							//std::string tmp; turn out we don't need this right n, let's declare variables before where they are first needed even when considering object life time, because it can get too long the function body and other things
							std::string Series_name = Json_container_for_series_meta["name"];
							Common_functions::Common01::folder_name_sanitize(Series_name);//if you don't do this, things can either doesnt work, or when components are working together something isn't supposed to work worked and other things relying on the information that it worked or it's valid ending up doesnt work

							std::ostringstream Content_folder_out;

							Content_folder_out << steam->Config_files->Content_folder << "/" << Series_name;
							fcout("Initiating at the directory paths :" << Content_folder_out.str());

							Common_functions::Common01::Create_folder(std::move(Content_folder_out));//create directory to put the series in

							//now let's open the index file which caches the response from this serie's json api
							std::ostringstream index_location;
							index_location << steam->Config_files->Update_resides_in_this_folder << "/" << Series_name << ".json";
							fcout("Index paths :" << index_location.str());

							//try to read something too
							std::wstringstream cache_content;
							{
								utility::ifstream_t fhandle(index_location.str());
								if (fhandle.is_open())
									cache_content << fhandle.rdbuf(); fhandle.close();
							}

							//now if not empty, compare these two versions from the network and in the local storage to eachother, see if there's an unmatch, and if there is one, then move forward doing something and eventually refreshing it if suceeds
							//fwcout(U("index_File_content :")<<cache_content.str());
							if (cache_content.str() == network_response)
								return;

							//now that you notice the differnce, decide which chapters you are going to fetch.
							//let's do concurrency on chapters of the current series working on, let's parse the json of the cached version in local file
							//because you don't wanna redownload old things too (if you just call probe_cahpter it's gonna do its job and download the chapter

							if (!cache_content.str().empty())
								Json_container_for_locally_cached_series_meta = nlohmann::json::parse(cache_content.str());


							auto lambda = [&](L1_thread_version_dynamic_container* dynamic_async_module) {
								//fcout("Element dump A :" << Json_container_for_locally_cached_series_meta["taggings"]);
								for (auto&& element : Json_container_for_series_meta["taggings"])
								{
									if (!Json_container_for_locally_cached_series_meta["taggings"].contains(element))
									{
										//fcout("Element dump B :" << element);
										//fcout("Evaluated :" << Json_container_for_locally_cached_series_meta["taggings"].contains(element));
										if (!element["permalink"].is_null())//always check when you are producer
										{


											(*dynamic_async_module)([&Series_name, steam, element]() {if (element.contains("title") && element.contains("permalink"))
											{

												probe_chapters(Series_name, element["permalink"], steam);

											}});//Pushing tasks

										}
									}
								}
							};//Maybe in the future do a Generic model like, include a function pool header inside these files that are included by omiko, and instead of using lambdas just dump the function in there and it's like becomes more focused about concepts in one hand and focusing implementations in another like a seperation.
							//Control paths
							if (steam)//using existing resource, other wise fall from steam, and use self contained method instead leading to prosparity
							{
								if ((*steam).L1_dynamic_async_inx)
								{
									//fcout("Taking path A in probe s");
									auto A = (*steam).L1_dynamic_async_inx;
									(*A).Reserve_expected(Json_container_for_series_meta["taggings"].size(), 2);
									(*A)([&]() {
										utility::ofstream_t output_overwrite(index_location.str());
										if (output_overwrite.is_open())
										{
											fcout("Overwrite indexer after finished updating:" << index_location.str());
											output_overwrite << network_response;
										}
										});//conclusions,please notice this lambda is put infront of lambda, therefore it's executed last because that's the way container designed
									lambda(A);
									(*A).Rotate(true);
									(*A).execute_all_now();
								}
							}
							else
							{
								L1_thread_version_dynamic_container A;//equivilent to making a std::vector and pushing async in the for loops
								A.Reserve_expected(Json_container_for_series_meta["taggings"].size(), 2);
								A([&]() {
									utility::ofstream_t output_overwrite(index_location.str());
									if (output_overwrite.is_open())
									{
										fcout("Overwrite indexer after finished updating:" << index_location.str());
										output_overwrite << network_response;
									}
									});//conclusions,please notice this lambda is put infront of lambda, therefore it's executed last because that's the way container designed
								lambda(&A);
								A.Rotate(true);
								A.execute_all_now();
							}

							//ct_end
						//} //-debug test
						}
						else
						{
							throw ("API error while fetching Series meta at remote location :");
						}
					}
					catch (std::exception e)
					{
						std::cerr << e.what() << series_url;
						//abort();
					}
				}
#endif
			}
			static void Loop_through_series_subscription_file(meta_* steam)
			{
				if (steam->Config_files->Series_subscription.is_open())
				{
					std::string tmp;
					while (std::getline((*steam).Config_files->Series_subscription, tmp))
					{
						probe_series((tmp + ".json"), steam);
						std::cout << std::endl;
					}
				}
			}
			static void Fetch_series_sbuscription(meta_* steam)
			{
				auto Config_files = std::ref(steam->Config_files);
				if (
					!Config_files.get()->Update_resides_in_this_folder.empty() &&
					!Config_files.get()->Content_folder.empty()
					)
				{
					auto&& Update_index_target = Config_files.get()->Update_resides_in_this_folder;
					Common_functions::Common01::Create_folder(Update_index_target);

					auto&& Content_output_target = Config_files.get()->Content_folder;
					Common_functions::Common01::Create_folder(Content_output_target);
					Loop_through_series_subscription_file(steam);
				}
			}
			static void Execute_config(meta_* steam)
			{
				auto Param_lists = std::ref(steam->Config_files->Param_lists);
				if (Param_lists.get().is_open())
				{
#if 1
					Param_lists.get() >> Json_container_for_program_config;
					if (!Json_container_for_program_config["Dynasty_content_folder"].is_null())
					{
						steam->Config_files->Content_folder = Json_container_for_program_config["Dynasty_content_folder"];
					}
#endif
				}
			}
#ifdef Deprecated_boDroXLlt1Ov2
		private:
			void INIT_cpp_compermises()
			{
				///Unlike c#, you can't just assign values to string variable in class
				///So you have to do it here
				///

				//Koshiki_url_utf16 = "https://dynasty-scans.com";
				///Never mind
				///Found a way to bypass
			}
#endif
		public:
			Dynasty_manga_(meta_* steam)
			{
				//Because these methods are using something like a steam they can basically be static and the whole thing is like a module

				Execute_config(steam);
				Fetch_series_sbuscription(steam);//Create output and index folder, with information from config file, and proceed further if success

#ifdef Deprecated_boDroXLlt1Ov2
				INIT_cpp_compermises();
#endif
			}//acting as orchestrator, so it's resonable that steam would punch through the routine
			//Apparently constructor can be private in cpp, but it needs to be public here, some how, I don't know why is, that because used in Core_
		public:
			struct Dev_env
			{
			public:
				void understanding_web_uri(std::string series_url)
				{
					auto url_str = utility::conversions::utf8_to_utf16(series_url);
					uri URL; URL = URL.encode_uri(U("https://dynasty-scans.com/chapters/citrus_ch01.json"));
					fwcout(URL.fragment());
					fwcout(URL.scheme());
					fwcout(URL.decode(URL.to_string()));
					fwcout(URL.host());
					fwcout(URL.validate(URL.to_string()));
					//fwcout(Common_functions::Common01::HttpGetString(URL.host(), URL.path()));
				}
			private:
				std::wstring inx001(utility::string_t& path_str)
				{
					if (!Koshiki_url_utf16.empty())
					{
						//std any
						//https://en.cppreference.com/w/cpp/utility/any
						//https://www.youtube.com/watch?v=7nPrUBNGRAk
						//auto res = Common_functions::Common01::HttpGetString(Koshiki_url_utf16, std::any_cast<utility::string_t>(path_str));
						//std::wcout << res << std::endl;
						return Common_functions::Common01::HttpGetString(Koshiki_url_utf16, path_str);
					}
					return (L"");//return empty wchar_t
				}
				static std::wstring Series_processor(std::wstring& input)
				{
					auto action = [](std::any tmp, std::any collector, bool* stop_running_me)
					{
						/// <summary>
						/// params will be given as pointers, cast them to pointers please
						/// </summary>
						/// <param name="input"></param>
						/// <returns></returns>
						auto str = std::any_cast<std::wstring*>(tmp);
						auto res = Common_functions::Common01::Get_str_between_wstr(*str, L"<a href=\"", L"\" class=\"name\">");
						if (!res.empty())
						{
							auto pog = (std::any_cast<std::wstring*>(collector));
							*pog += res;
							*pog += '\n';
						}
					};
#ifdef Deprecated_gHeMfhI1R1YIR
					std::wistringstream iss(input);
					std::wstring tmp; std::wstring collector; //int_fast64_t counter=0;
					while (std::getline(iss, tmp))
					{
						//counter++;
						//fcout("processed target at line :" << counter);


					}
#endif
					std::wstring collector;
					Common_functions::Common01::Imagine_str_lp(nullptr, &input, action, &collector);
					Common_functions::Common01::String_pop_lastn(nullptr, &collector, nullptr);

					return collector;

				}
				static std::wstring Chapters_processor(std::wstring& input)
				{
					//using reference input for orchestrator type pipelining
#ifdef Deprecated_gHeMfhI1R1YIR
					std::wistringstream iss(input);
					std::wstring tmp; std::wstring collector; int_fast64_t counter = 0;
					while (std::getline(iss, tmp))
					{
						counter++;
						auto pos = tmp.find(L"var pages = [{\"image\":\"");
						if (pos != tmp.npos)
						{
							//fcout("found");
							auto a = Common_functions::Common01::Get_str_lists_between_wstr(tmp, L"\"image\":\"", L"\",\"name\":\"");
							for (int_fast32_t i = 0; i < a.size(); i++)
							{
								collector += Koshiki_url_utf16;
								collector += a[i];
								collector += '\n';
							}
							break;
						}
					}
#endif
					auto action = [](std::any tmp, std::any collector, bool* stop_running_me)
					{

						auto str = std::any_cast<std::wstring*>(tmp);
						auto pos = (*str).find(L"var pages = [{\"image\":\"");
						//if (pos != (*str).npos)//might be slow to dereference but easier to read
						if (pos != std::wstring::npos)
						{
							//fcout("found");
							auto a = Common_functions::Common01::Get_str_lists_between_wstr(*str, L"\"image\":\"", L"\",\"name\":\"");
							auto pog = (std::any_cast<std::wstring*>(collector));
							for (size_t i = 0; i < a.size(); i++)
							{
								(*pog) += Koshiki_url_utf16;
								(*pog) += a[i];
								(*pog) += '\n';
							}
							*stop_running_me = true;
						}
					};
					std::wstring collector;
					Common_functions::Common01::Imagine_str_lp(nullptr, &input, action, &collector);
					Common_functions::Common01::String_pop_lastn(nullptr, &collector, nullptr);
					return collector;
				}
				static void inxend(std::wstring input)
				{
					auto file_handle = Common_functions::Common01::wofstream_create("Feedback_manga.txt");//the benifit of using new here would be you can get the executable's directory for creating output file even though you are in development
					//https://stackoverflow.com/questions/4053918/how-to-portably-write-stdwstring-to-file#18226387

					(*file_handle) << input;
					(*file_handle).close();
				}
			public:
				void Series_Orchestrator(utility::string_t path_str)
				{
					auto Str = inx001(path_str);

					/*
					LF::L1_taf_core_duo_MX(XXX,
						[](){

						auto Making_files = Common_functions::Common01::wofstream_create("Feedback_manga.txt");//Doesn't depend, but it's an effort to create a new file in the system while other things run, and before creating it and using again(which can race but operating system can handle it? and it's just dev codes.).
						},
						[&Str](){
							auto collected_terms = inx002(Str);
							inxend(collected_terms);

						});
					*///works
					auto collected_terms = Series_processor(Str);
					inxend(collected_terms);
				}
				void Chapters_Orchestrator(utility::string_t path_str)
				{
					auto Str = inx001(path_str);
					auto collected_terms = Chapters_processor(Str);
					inxend(collected_terms);
				}
			};
		};
	}
}