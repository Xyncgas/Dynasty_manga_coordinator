#pragma once

//A macro for switching from push_back for vector to emplace_back
#ifdef _DEBUG
#define v_back push_back
#endif
#ifdef NDEBUG
#define v_back emplace_back
#endif