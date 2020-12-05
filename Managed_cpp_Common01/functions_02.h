#pragma once
namespace Common_functions
{
	namespace Common01
	{
		void folder_name_sanitize(std::string& input)
		{

			//sanitize string formatting - don't end with space or \n
			static constexpr std::array<char, 2> stuffs = { '\n',' ' };
			for (auto i : stuffs)
			{
				Common_functions::Common01::pop_optional_at_end_str(input, i);
			}

		}
		void folder_name_sanitize(std::wstring& input)
		{

			//sanitize string formatting - don't end with space or \n
			static constexpr std::array<wchar_t, 2> stuffs = { '\n',' ' };
			for (auto i : stuffs)
			{
				Common_functions::Common01::pop_optional_at_end_str(input, i);
			}

		}
		std::wstring HttpGetString(std::wstring input)
		{
			input = Common_functions::Common01::sanitize_url(input);
			uri URL;
			try
			{
				URL = web::uri::encode_uri(input);
			}
			catch (std::exception e)
			{
				std::cerr << "url validation failed :Wbg1TsaygtQWB" << std::endl;
				throw("url validation failed :Wbg1TsaygtQWB");//not catching it here and the function will fail entirely if nobody is catching it later
			}
			http_client client(URL);
			auto res = client.request(methods::GET);
			return (res.get().extract_string().get());
		}
		std::wstring HttpGetString(std::string input)
		{
			auto url_str = utility::conversions::utf8_to_utf16(Common_functions::Common01::sanitize_url(input));
			//std::wcout << url_str << std::endl; std::cin.ignore();
			uri URL;
			try
			{
				URL = web::uri::encode_uri(url_str);
			}
			catch (std::exception e)
			{
				std::cerr << "url validation failed :Wbg1TsaygtQWB" << std::endl;
				throw("url validation failed :Wbg1TsaygtQWB");//not catching it here and the function will fail entirely if nobody is catching it later
			}
			http_client client(URL);
			auto res = client.request(methods::GET);
			return (res.get().extract_string().get());
		}
		std::wstring HttpGetString(web::uri URI)
			/// Utility::string_t is something that's used specifically for this method and the library it's using :cpp_restSDK
		{
#ifdef Using_httplib_implementation
			httplib::Client cli(url.c_str());
			auto res = cli.Get(path_str.c_str());
			return res->body;
#endif
#ifdef Using_cpp_restSDK
			//I think we don't need to validate url since I can assume it was validated usually

			// Create http_client to send the request.
			http_client client(URI);
			//uri_builder builder(path_str);//This u macro might cause conflicts
			//fwcout(client.base_uri().to_string());
			auto res = client.request(methods::GET);

			//converting wstring to string
			//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
			//https://stackoverflow.com/questions/402283/stdwstring-vs-stdstring


			//setup converter
			//using convert_type = std::codecvt_utf8<wchar_t>;
			//std::wstring_convert<convert_type, wchar_t> converter;

			//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
			return (res.get().extract_string().get());
#endif
		}
		std::wstring HttpGetString(utility::string_t url, utility::string_t path_str)
			/// Utility::string_t is something that's used specifically for this method and the library it's using :cpp_restSDK
		{
#ifdef Using_httplib_implementation
			httplib::Client cli(url.c_str());
			auto res = cli.Get(path_str.c_str());
			return res->body;
#endif
#ifdef Using_cpp_restSDK

			auto url_str = Common_functions::Common01::sanitize_url(url+path_str);
			uri URL;
			http_client client(URL);
			auto res = client.request(methods::GET);
			return (res.get().extract_string().get());
#endif
		}
		std::optional<int> HttpGetStreambuf_toFile(utility::string_t url, utility::string_t path_str, utility::string_t file_name)
		{
			auto url_str = Common_functions::Common01::sanitize_url(url+path_str);
			uri URL;
			/*
			if (web::uri::validate(URL.to_string()))
			{
				//this also performs validation check, I jus needed it to be explicit before, because this might throw and I kinda wanna eliminate at least one situation with useful debug message and yes at cost of repeated
				URL = web::uri::encode_uri(url_str);
				
			}
			else
			{
				return 101;
			}
			*/
			try
			{
				URL = web::uri::encode_uri(url_str);
			}
			catch (std::exception e)
			{ return 101; }
			{
				http_client client(URL);
				auto fhandle = Concurrency::streams::fstream::open_ostream(file_name);
				//uri_builder builder(path_str);//This u macro might cause conflicts
				auto res = client.request(methods::GET);
				res.get().body().read_to_end(fhandle.get().streambuf()).wait();
				return std::nullopt;
			}
		}
		void HttpGetString(utility::string_t url, utility::string_t path_str, std::wstring* output)
			/// Utility::string_t is something that's used specifically for this method and the library it's using :cpp_restSDK
		{
#ifdef Using_httplib_implementation
			httplib::Client cli(url.c_str());
			auto res = cli.Get(path_str.c_str());
			(*output) = res->body;
#endif
#ifdef Using_cpp_restSDK
			auto url_str = Common_functions::Common01::sanitize_url(url + path_str);
			uri URL;
			http_client client(URL);
			auto res = client.request(methods::GET);
			(*output) = (res.get().extract_string().get());
#endif
		}
		static void String_pop_lastn(std::string* A, std::wstring* B, utility::string_t* C)
		{
			if (A != nullptr)
			{
				//http://www.cplusplus.com/reference/functional/reference_wrapper/get/
				if (!(*A).empty())
				{
					if ((*A).back() == '\n')//will throw if std::wstring::back() called on empty string
					{
						(*A).pop_back();
					}
				}
			}
			if (B != nullptr)
			{
				if (!(*B).empty())
				{
					if ((*B).back() == '\n')
					{
						(*B).pop_back();
					}
				}

			}
			if (C != nullptr)
			{
				if (!(*C).empty())
				{
					if ((*C).back() == '\n')
					{
						(*C).pop_back();
					}
				}
			}
		}
		/// <summary>
		/// param overloads
		/// https://stackoverflow.com/questions/3784114/how-to-pass-optional-arguments-to-a-method-in-c
		/// So you don't have to copy the body serveral times -sigh wasn't working
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
	}
}