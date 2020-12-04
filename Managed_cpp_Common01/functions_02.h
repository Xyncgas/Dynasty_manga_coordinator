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
			uri URL; URL = web::uri::encode_uri(input);
			if (web::uri::validate(URL.to_string()))
			{
				http_client client(URL);
				auto res = client.request(methods::GET);
				return (res.get().extract_string().get());
			}
			else {
				throw("url validation failed :Wbg1TsaygtQWB");
			}
		}
		std::wstring HttpGetString(std::string input)
		{
			auto url_str = utility::conversions::utf8_to_utf16(input);
			uri URL; URL = web::uri::encode_uri(url_str);
			if (web::uri::validate(URL.to_string()))
			{
				http_client client(URL);
				auto res = client.request(methods::GET);
				return (res.get().extract_string().get());
			}
			else {
				throw("url validation failed :Wbg1TsaygtQWB");
			}
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
			if (web::uri::validate(URI.to_string()))
			{
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
			}
			else {
				throw("url validation failed :Wbg1TsaygtQWB");
			}
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

			// Create http_client to send the request.
			if (web::uri::validate(url+path_str))
			{
				return L"";
			}
			http_client client(url);
			uri_builder builder(path_str);//This u macro might cause conflicts
			auto res = client.request(methods::GET, builder.to_string());

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
		std::optional<int> HttpGetStreambuf_toFile(utility::string_t url, utility::string_t path_str, utility::string_t file_name)
		{
			if (web::uri::validate(url + path_str))
			{
				http_client client(url);
				auto fhandle = Concurrency::streams::fstream::open_ostream(file_name);
				uri_builder builder(path_str);//This u macro might cause conflicts
				auto res = client.request(methods::GET, builder.to_string());
				res.get().body().read_to_end(fhandle.get().streambuf()).wait();
				return std::nullopt;
			}
			else
			{
				return 101;
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

			// Create http_client to send the request.
			http_client client(url);
			uri_builder builder(path_str);//This u macro might cause conflicts
			auto res = client.request(methods::GET, builder.to_string());

			//converting wstring to string
			//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
			//https://stackoverflow.com/questions/402283/stdwstring-vs-stdstring


			//setup converter
			//using convert_type = std::codecvt_utf8<wchar_t>;
			//std::wstring_convert<convert_type, wchar_t> converter;

			//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
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