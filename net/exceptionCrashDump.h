#pragma once

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <psapi.h>
#include <rtcapi.h>
#include <Shellapi.h>
#include <dbghelp.h>

namespace MLN
{
	class CCrashHandler
	{
	public:
		static void Init(const char* root_path, const char* header, const bool isFullDump);
		static void Init(const char* env_file);

	private:
		static bool InitFromXML(const char* env_file);

		// Sets exception handlers that work on per-process basis
		static void SetProcessExceptionHandlers();

		// Installs C++ exception handlers that function on per-thread basis
		static void SetThreadExceptionHandlers();

		static void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** pExceptionPointers);
		static void CreateMiniDump(EXCEPTION_POINTERS* pExcPtrs);
		static void CreateFullDump(EXCEPTION_POINTERS* pExcPtrs);
		static void MyStackWalk(EXCEPTION_POINTERS* pExcPtrs);

		/* Exception handler functions. */

		static LONG WINAPI SehHandler(PEXCEPTION_POINTERS pExceptionPtrs);
		static void __cdecl TerminateHandler();
		static void __cdecl UnexpectedHandler();
		static void __cdecl PureCallHandler();
		static void __cdecl InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
		static int __cdecl NewHandler(size_t);

		static void SigabrtHandler(int);
		static void SigfpeHandler(int /*code*/, int subcode);
		static void SigintHandler(int);
		static void SigillHandler(int);
		static void SigsegvHandler(int);
		static void SigtermHandler(int);
	};

};//namespace MLN
	
