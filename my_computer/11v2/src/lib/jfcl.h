#ifndef JFCL_FCL_LIB
#define JFCL_FCL_LIB

#define JFCL_DLLEXPORT
#define JFCL_APICALL __cdecl

JFCL_DLLEXPORT int JFCL_APICALL jfcl_main(int argc, const char* argv[]);

/*
///////////////////////////////////////////////////////////////////////////////
#ifndef JFCL_FCL_APICALL
#if defined(_WIN32)
/// @brief Calling convention for all API functions
#define JFCL_FCL_APICALL __cdecl
#else
#define JFCL_FCL_APICALL
#endif // defined(_WIN32)
#endif // JFCL_FCL_APICALL

#ifdef JFCL_FCL_DLL_EXPORTS

///////////////////////////////////////////////////////////////////////////////
#ifndef JFCL_FCL_DLLEXPORT
#if defined(_WIN32)
/// @brief Microsoft-specific dllexport storage-class attribute
#define JFCL_FCL_DLLEXPORT __declspec(dllexport)
#endif // defined(_WIN32)
#endif // JFCL_FCL_DLLEXPORT

///////////////////////////////////////////////////////////////////////////////
#ifndef JFCL_FCL_DLLEXPORT
#if __GNUC__ >= 4
/// @brief GCC-specific dllexport storage-class attribute
#define JFCL_FCL_DLLEXPORT __attribute__((visibility("default")))
#endif // __GNUC__ >= 4
#endif // JFCL_FCL_DLLEXPORT

#else // JFCL_FCL_DLL_EXPORTS
#define JFCL_FCL_DLLEXPORT
#endif // JFCL_FCL_DLL_EXPORTS
*/

#endif
