using namespace std;
using namespace LF::csp;
using namespace Models;
//These processor archetectures are builit in faith that they are supposed to yeild a result with good user experience, usually means controlling the latency, for whatever you are doing here.
#define M_ P0
struct M_ {
	inline static int control_paths = -1;
	M_()
	{	
		std::string user_input;
		fcout("Here are options to choose :"
			<< std::endl << "1) Automatic series parsing (print the output!)"
			<< std::endl << "2) Download a chapter"
		);
		std::cout << "Input :";
		std::getline(std::cin, user_input);
		
		if (user_input == "1")
		{
			
		}
		else if (user_input == "2")
		{

		}
	}
};//c++ struct is basically public class in c#
#undef M_

#define M_ P1
struct M_ {
	inline static std::array<std::string, 3>Config_files = { "dynasty_manga_series","configuration.json","Update" };//moved in the next line, don't touch it anymore
	inline static std::array<bool, 1>behavior = { false };
	//behavior[0] set to true in main function when argc>1, which is after all these processors are run(constructed).
	M_()
	{
#if 1
		fcout("Core enviornment production dynasty-manga signal :" << Models::Common02::Dynasty_manga_::Koshiki_url_utf8);
		{
			Models::Common02::Dynasty_manga_::meta_ Dynasty_manga_meta;
			Dynasty_manga_meta.L1_dynamic_async_inx = &(Core.c._Initialized_Moduels_.global_dynamic_csp[0]);
			using namespace Models::Common02; using std::move;

			//Because you can not access struct through variable names in c++, using namespace instead, and eventhough with usings the first line let's be explicit

			Dynasty_manga_::Config_files_ Dynasty_manga_meta_io_Config_files(std::move(Config_files[0]), move(Config_files[1]), move(Config_files[2]));
			Dynasty_manga_meta.Config_files = &Dynasty_manga_meta_io_Config_files;


			Dynasty_manga_ Dynasty_manga(&Dynasty_manga_meta);//Pass metas, it was designed to work without, as an example, it would cause compiler warning for not using the prototype, the param is specialized to use a pointer as parameter, and it acts like a steam that will go through its scopes
		}

#endif
	}
};//c++ struct is basically public class in c#
#undef M_

#define M_ Px_end
//Px_end Enabled execution of M_x in constructor --This feature not on currently, refers back to 1d2cd205aa30940323e3e7db8c907a9c0e7772fe on github.
struct M_ {
	~M_()
	{
		fcout("A, gwart.");
		if (P1::behavior[0] == true)
		{
			std::cin.ignore();
		}
	}
};
#undef M_

//Approaches :make all struct static is one appropach, everything is floating in global.
//Another way is to wrap them in one single struct, similar to above.
//Currentl using el_nests for creating their instances so technically these structs are isolated here and even though it's possible to access eachother it's only through the api/capability ROOT_ is serving.