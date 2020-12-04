#pragma once
namespace Common_functions
{
	
	namespace Common01
	{
		//Put stuffs that doesn't depend on rest on top, other wise.

		///This the place you are stacking useful functions
		
		void pop_optional_at_end_str(std::string& input, char& proper)
		{
			if (input.back() == proper)
			{
				input.pop_back();
			}
		}
		void pop_optional_at_end_str(std::wstring& input,wchar_t& proper)
		{
			if (input.back() == proper)
			{
				input.pop_back();
			}
		}
		/// optional_path_way_after_ensured_sucess
		/// used for calling codes that might throw that aren't your design
		std::optional<const char*> opwaes(std::function<void()> func)
		{
			try
			{
				func();
				return std::nullopt;
			}
			catch (std::exception e) { return (e.what()); }
		}
		bool Create_folder(std::ostringstream paths)
		{
			return std::filesystem::create_directories(paths.str());
		}
		bool Create_folder(std::string paths)
		{
			return std::filesystem::create_directories(paths);
		}
		bool Create_folder(std::wstring paths)
		{
			return std::filesystem::create_directories(paths);
		}
		static void Imagine_str_lp(std::string* input_A, std::wstring* input_B, /*std::function<void(std::any, std::any, bool*) > function_*//*Let's use normal function pointer for now and if needed use std::function*/void(*function_)(std::any,std::any,bool*), std::any output)
		{
			/// <summary>
			/// input is being accessed through reference!!!
			/// </summary>
			/// <param name="input_A"></param>
			/// <param name="input_B"></param>
			/// <param name="input_C"></param>
			/// <param name="function_"></param>
			/// <param name="output"></param>
			if ((input_A)!=nullptr && !(*input_A).empty())
			{
				std::istringstream iss(*input_A);
				std::string tmp; bool signal = false;
				while (std::getline(iss, tmp))
				{
					function_(&tmp,output,&signal);
					if (signal == true)
					{
						break;
					}
				}
			}
			if ((input_B) != nullptr && !(*input_B).empty())
			{
				std::wistringstream iss(*input_B);
				std::wstring tmp; bool signal = false;
				while (std::getline(iss, tmp))
				{
					function_(&tmp, output,&signal);
					if (signal == true)
					{
						break;
					}
				}
			}
		}
		static std::shared_ptr<std::fstream> fstream_create(std::string filename)
		{
			auto out = std::make_shared<std::fstream>(filename, std::fstream::in | std::fstream::out | std::fstream::app);
			//if file exist, append
			//if file doesnt exist, create
			//even without | std::fstream::app it shall append
			return out;
		}
		static std::shared_ptr<std::wfstream> wfstream_create(std::string filename)
		{
			auto out = std::make_shared<std::wfstream>(filename, std::wfstream::in | std::wfstream::out | std::wfstream::app);
			//if file exist, append
			//if file doesnt exist, create
			//even without | std::fstream::app it shall append
			return out;
		}
		static std::shared_ptr<std::wofstream> wofstream_create(std::string filename)
		{
			auto out = std::make_shared<std::wofstream>(filename);
			//if file exist, append
			//if file doesnt exist, create
			//even without | std::fstream::app it shall append
			return out;
		}
		static std::wstring Get_str_between_wstr(std::wstring& input, std::wstring first, std::wstring next)
		{
			/// <summary>
			/// Input is being acessed through reference !!!
			/// </summary>
			/// <param name="input"></param>
			/// <param name="first"></param>
			/// <param name="next"></param>
			/// <returns></returns>
			std::wstring output;
			//fwcout(output);
			auto pos_1 = input.find(first);//using the auto keyword because you are not enforcing types and letting compiler decides what the function is returning.
			if (pos_1 == std::wstring::npos)
			{
				return output;
			}
			auto pos_2 = input.find(next, pos_1 + 1);
			if (pos_2 == std::wstring::npos)
			{
				return output;
			}
			//fcout("pos_1 :" << pos_1);
			//fcout("pos_2 :" << pos_2);

			auto trim_begin_pos_draft_variable = (pos_1 + first.length());
			//fcout("trim_begin_pos_draft_variable :" << trim_begin_pos_draft_variable);

			auto lengthy_draft = pos_2 - trim_begin_pos_draft_variable;
			//fcout("lengthy_draft :" << lengthy_draft);

			if (trim_begin_pos_draft_variable >= 0 && lengthy_draft >= 0)
			{
				output = input.substr(trim_begin_pos_draft_variable, lengthy_draft);
			}
			return output;
		}
		static std::vector<std::wstring> Get_str_lists_between_wstr(std::wstring input, std::wstring first, std::wstring next)
		{
			std::vector<std::wstring> output;
			//fwcout(input);
			//fwcout(output);
			auto offset_t = input.npos; offset_t = 0;

			J0XWuFotUBkMQaisdj_start:
			auto pos_1 = input.find(first, offset_t);//using the auto keyword because you are not enforcing types and letting compiler decides what the function is returning.
			if (pos_1 == input.npos)
			{
				//fcout("First term error");
				return output;
			}
			auto pos_2 = input.find(next, pos_1 + 1);
			if (pos_2 == input.npos)
			{
				//fcout("Second term error");
				return output;
			}
			//fcout("pos_1 :" << pos_1);
			//fcout("pos_2 :" << pos_2);

			auto trim_begin_pos_draft_variable = (pos_1 + first.length());
			//fcout("trim_begin_pos_draft_variable :" << trim_begin_pos_draft_variable);

			auto lengthy_draft = pos_2 - trim_begin_pos_draft_variable;
			//fcout("lengthy_draft :" << lengthy_draft);

			if (trim_begin_pos_draft_variable >= 0 && lengthy_draft >= 0)
			{
				//auto str = input.substr(trim_begin_pos_draft_variable, lengthy_draft);
				//fwcout(str);
				output.emplace_back(input.substr(trim_begin_pos_draft_variable, lengthy_draft));
				offset_t = pos_2+1;//start from where the second term was found, essentially skipping this terms ok
				goto J0XWuFotUBkMQaisdj_start;
			}
			return output;
		}
	}
}