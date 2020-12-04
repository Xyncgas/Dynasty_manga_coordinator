#pragma once
//namespace are based on project :currently is common01 for functions.h
#include "generate_random_alphaSquence.h"
#include "json.h"
#include "functions_01.h"
#include "Libcurl_custom.h"

#if 0
#include "httplib.h"
#define Using_httplib_implementation
#endif

#if 1
#include "cpprestsdk.h"
#define Using_cpp_restSDK
#endif

#include "functions_02.h"
///Generally we are not changing these orders since the files are built and written with this dependency in mind