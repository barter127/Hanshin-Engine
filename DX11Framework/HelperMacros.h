#pragma once


#ifndef  COMMON_MACROS
#define COMMON_MACROS

#define DELETE_PTR(ptr) delete ptr; ptr = nullptr;

#define STRING_TO_CHARPTR(str) (char*)str.c_str()

#endif // ! COMMON_MACROS
