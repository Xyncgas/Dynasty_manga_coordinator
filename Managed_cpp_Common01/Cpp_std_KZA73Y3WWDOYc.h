#pragma once



/*Streams inlclude*/
//#include <sstream> included below
#include <fstream>
#include <iostream>
#include <ostream>//for clipboard function used in aplex_toolkit_(and its dummy)
/*Streams*/

/*Threading related
*/
#include <thread>
//#include <atomic>         // std::atomic
#include <future>
/*
Threading related functions*/


/*Vector - array related
*/
#include <array> //stuff like dequed
#include <vector>//using std::vector templated_clase
/*Vector - array related
*/



/*Common includes
*/
#include <sstream> //converting between int and string

#include <string>/*
//#include <string.h>

<string.h> contains old functions like strcpy, strlen for C style null-terminated strings.
<string> primarily contains the std::string, std::wstring and other classes.

It should also be noted that using string.h is deprecated within C++.
If you need the functionality contained within, you should use the header cstring.
This more or less completely bypasses the issue of "What's the difference between these two" because it's very obvious that one is from the C library. – Mike Bailey Mar 11 '12
*/
#include <float.h>

#include <stdio.h>
#include <stdint.h>

#include <errno.h>

#include <memory>
#include <functional>//for lambda functin<typedef()> stuff like this

#include <stdlib.h>//https://www.tutorialspoint.com/c_standard_library/stdlib_h.htm this is basically a c_standard library//includes stddef which defines size_t
#include <iomanip>

#include <clocale> //local

#include <random>
#include <sys/types.h> //related to system and getting pid?


#include <utility>//tuple (for returning multiple stuffs from a function)
#include "sys/stat.h"// offer [stat] type, used in a function to check if file exist (//
#include <any>// std::any


#include <variant>// std::variant

#include <optional>// std::optional
#include <filesystem> //c++ 17 filesystem

#include <exception>//exceptions and for clipboard aplex_toolkit
#include <stdexcept>//exceptions and for clipboard aplex_toolkit

#include <stdexcept>//for constexpr noexcept?
#include <complex>//complex number struct
#include <type_traits>//static_assert

#include <execution> //to make the parallel execution policies available.

#include <locale>
#include <codecvt>//For using std::wstring_convert to convert wstring to string https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string

#include <assert.h>     /* assert */

/*
Common*/



/*Time Utilities includes
*/
#include <chrono>
#include <ctime>
#include <time.h>
/*
Time Utilities includes*/


/*Mathmatics and algorithms*/
#include <cmath>
#include <algorithm>
#include <math.h>
/*mainly needed for cos and those of fancy stuffs*/




/*Private headers will start here*/
