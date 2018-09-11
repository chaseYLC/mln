#include "stdafx.h"
#include "exceptionCrashDump.h"

#include <process.h>
#include <string>
#include <WinSock2.h>

#include "xmlParsingSupport.h"

#include "logger.h"


#pragma comment(lib,"imagehlp.lib")
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

namespace MLN
{

#ifndef _AddressOfReturnAddress

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

	EXTERNC void * _AddressOfReturnAddress(void);
	EXTERNC void * _ReturnAddress(void);

#endif 

	typedef void(*DUMP_FUNC)(EXCEPTION_POINTERS* pExcPtrs);
	static DUMP_FUNC g_dump_func = nullptr;

	static char g_filename[MAX_PATH] = { 0, };
	static char g_dump_path[MAX_PATH] = { 0, };

	void generate_filename(char out[MAX_PATH], const char* header, const char* ext)
	{
		char hostname[50] = { 0, };
		char target_dir[MAX_PATH] = { 0, };

		memset(hostname, 0, sizeof(hostname));
		gethostname(hostname, sizeof(hostname));

		SYSTEMTIME st;
		GetLocalTime(&st);

		_snprintf_s(target_dir, MAX_PATH, _TRUNCATE
			, "%s\\%04d%02d%02d"
			, g_dump_path, st.wYear, st.wMonth, st.wDay);

		// recursive creating directories
		try {
			std::string path_base(target_dir);
			std::string::size_type pos = 0;
			do
			{
				pos = path_base.find_first_of("\\/", pos + 1);
				CreateDirectory(path_base.substr(0, pos).c_str(), NULL);
			} while (pos != std::string::npos);
		}
		catch (...)
		{
			strncpy_s(g_dump_path, MAX_PATH, ".", 1);
		}

		_snprintf_s(out, MAX_PATH, _TRUNCATE, "%s\\%s_%s_%s_%04d%02d%02d_%02d%02d%02d.%s"
			, target_dir
			, header
			, g_filename
			, hostname
			, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond
			, ext
		);
	}

	void CCrashHandler::Init(const char* root_path, const char* header, const bool isFullDump)
	{
		memset(g_filename, 0, sizeof(g_filename));
		memset(g_dump_path, 0, sizeof(g_dump_path));

		// root_path
		if (root_path && 0 < strlen(root_path))
		{
			strncpy_s(g_dump_path, sizeof(g_dump_path), root_path, _TRUNCATE);
		}
		else
		{
			char pathModule[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH];
			memset(pathModule, 0, sizeof(pathModule));
			memset(drive, 0, sizeof(drive));
			memset(dir, 0, sizeof(dir));

			::GetModuleFileName(NULL, pathModule, MAX_PATH);
			_splitpath_s(pathModule, drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);

			_snprintf_s(g_dump_path, MAX_PATH, _TRUNCATE
				, "%s\\%s"
				, drive, dir);
		}

		for (size_t i = strlen(g_dump_path) - 1; i >= 0; --i)
		{
			if ('\\' == g_dump_path[i] || '/' == g_dump_path[i])
				g_dump_path[i] = 0;
			else
				break;
		}

		if (0 >= strlen(g_dump_path))
		{
			g_dump_path[0] = '.';
		}

		// header
		if (header && 0 < strlen(header))
		{
			strncpy_s(g_filename, sizeof(g_filename), header, _TRUNCATE);
		}
		else
		{
			// getting filename
			char path_buffer[MAX_PATH];
			memset(path_buffer, 0, sizeof(path_buffer));
			memset(g_filename, 0, sizeof(g_filename));

			::GetModuleFileName(NULL, path_buffer, MAX_PATH);
			_splitpath_s(path_buffer, nullptr, 0, nullptr, 0, g_filename, sizeof(g_filename), nullptr, 0);
		}

		// is full dump??
		if (isFullDump)
			g_dump_func = CCrashHandler::CreateFullDump;
		else
			g_dump_func = CCrashHandler::CreateMiniDump;

		SetProcessExceptionHandlers();
		SetThreadExceptionHandlers();
	}

	void CCrashHandler::Init(const char* env_file)
	{
		assert(env_file && "env_file cannot be null. in CCrashHandler::Init()");

		if (false == InitFromXML(env_file))
		{
			CCrashHandler::Init(nullptr, nullptr, true);
		}
	}

	bool CCrashHandler::InitFromXML(const char* env_file)
	{
		XML_PARSING_BEGIN(env_file);

		const auto &info_pt = XML_TREEOBJ_REF.get_child("config.dump_info.info");

		std::string strFilename, strPath;
		bool isFullDump = true;

		strFilename = info_pt.get("file", "");
		strPath = info_pt.get("dir", "");
		isFullDump = info_pt.get("dir", true);

		CCrashHandler::Init(strPath.c_str(), strFilename.c_str(), isFullDump);

		XML_PARSING_END;

		return true;
	}

	void CCrashHandler::SetProcessExceptionHandlers()
	{
		SetUnhandledExceptionFilter(SehHandler);
		_set_purecall_handler(PureCallHandler);
		//	_set_new_handler(NewHandler);
		_set_invalid_parameter_handler(InvalidParameterHandler);
		_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
		signal(SIGABRT, SigabrtHandler);
		signal(SIGINT, SigintHandler);
		signal(SIGTERM, SigtermHandler);
	}

	void CCrashHandler::SetThreadExceptionHandlers()
	{

		set_terminate(TerminateHandler);
		set_unexpected(UnexpectedHandler);
		typedef void(*sigh)(int);
		signal(SIGFPE, (sigh)SigfpeHandler);
		signal(SIGILL, SigillHandler);
		signal(SIGSEGV, SigsegvHandler);
	}


	void CCrashHandler::GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** ppExceptionPointers)
	{
		// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

		EXCEPTION_RECORD ExceptionRecord;
		CONTEXT ContextRecord;
		memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_
		__asm {
			mov dword ptr[ContextRecord.Eax], eax
			mov dword ptr[ContextRecord.Ecx], ecx
			mov dword ptr[ContextRecord.Edx], edx
			mov dword ptr[ContextRecord.Ebx], ebx
			mov dword ptr[ContextRecord.Esi], esi
			mov dword ptr[ContextRecord.Edi], edi
			mov word ptr[ContextRecord.SegSs], ss
			mov word ptr[ContextRecord.SegCs], cs
			mov word ptr[ContextRecord.SegDs], ds
			mov word ptr[ContextRecord.SegEs], es
			mov word ptr[ContextRecord.SegFs], fs
			mov word ptr[ContextRecord.SegGs], gs
			pushfd
			pop[ContextRecord.EFlags]
		}

		ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
		ContextRecord.Eip = (ULONG)_ReturnAddress();
		ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
		ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);


#elif defined (_IA64_) || defined (_AMD64_)

		/* Need to fill up the Context in IA64 and AMD64. */
		RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

		ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

		ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

		ExceptionRecord.ExceptionCode = dwExceptionCode;
		ExceptionRecord.ExceptionAddress = _ReturnAddress();

		///

		EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
		memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
		CONTEXT* pContextRecord = new CONTEXT;
		memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

		*ppExceptionPointers = new EXCEPTION_POINTERS;
		(*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
		(*ppExceptionPointers)->ContextRecord = pContextRecord;
	}

	// This method creates minidump of the process
	void CCrashHandler::CreateMiniDump(EXCEPTION_POINTERS* pExcPtrs)
	{
		HMODULE hDbgHelp = NULL;
		HANDLE hFile = NULL;
		MINIDUMP_EXCEPTION_INFORMATION mei;
		MINIDUMP_CALLBACK_INFORMATION mci;

		// Load dbghelp.dll
		hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
		if (hDbgHelp == NULL)
		{
			// Error - couldn't load dbghelp.dll
			return;
		}

		char filename[MAX_PATH] = { 0, };
		generate_filename(filename, "MiniDump", "dmp");

		// Create the minidump file
		hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		// Write minidump to the file
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pExcPtrs;
		mei.ClientPointers = FALSE;
		mci.CallbackRoutine = NULL;
		mci.CallbackParam = NULL;

		typedef BOOL(WINAPI *LPMINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
			MINIDUMP_TYPE DumpType,
			CONST		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
			CONST		PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
			CONST		PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

		LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		if (!pfnMiniDumpWriteDump)
		{
			// Bad MiniDumpWriteDump function
			return;
		}

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessId = GetCurrentProcessId();

		BOOL bWriteDump = pfnMiniDumpWriteDump(hProcess, dwProcessId, hFile, MiniDumpNormal, &mei, NULL, &mci);

		if (!bWriteDump)
		{
			// Error writing dump.
			return;
		}

		// Close file
		CloseHandle(hFile);

		// Unload dbghelp.dll
		//    FreeLibrary(hDbgHelp);
	}

	void CCrashHandler::CreateFullDump(EXCEPTION_POINTERS* pExcPtrs)
	{
		HMODULE hDbgHelp = NULL;
		HANDLE hFile = NULL;
		MINIDUMP_EXCEPTION_INFORMATION mei;
		MINIDUMP_CALLBACK_INFORMATION mci;

		// Load dbghelp.dll
		hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
		if (hDbgHelp == NULL)
		{
			// Error - couldn't load dbghelp.dll
			return;
		}


		char filename[MAX_PATH] = { 0, };
		generate_filename(filename, "FullDump", "dmp");

		// Create the minidump file
		hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		// Write minidump to the file
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pExcPtrs;
		mei.ClientPointers = FALSE;
		mci.CallbackRoutine = NULL;
		mci.CallbackParam = NULL;

		typedef BOOL(WINAPI *LPMINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
			MINIDUMP_TYPE DumpType,
			CONST		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
			CONST		PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
			CONST		PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

		LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		if (!pfnMiniDumpWriteDump)
		{
			// Bad MiniDumpWriteDump function
			return;
		}

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessId = GetCurrentProcessId();

		const DWORD Flags = MiniDumpWithFullMemory |
			MiniDumpWithFullMemoryInfo |
			MiniDumpWithHandleData |
			MiniDumpWithUnloadedModules |
			MiniDumpWithThreadInfo;

		BOOL bWriteDump = pfnMiniDumpWriteDump(hProcess, dwProcessId, hFile, (MINIDUMP_TYPE)Flags, &mei, NULL, &mci);

		if (!bWriteDump)
		{
			// Error writing dump.
			return;
		}

		// Close file
		CloseHandle(hFile);

		// Unload dbghelp.dll
		//    FreeLibrary(hDbgHelp);
	}


	void CCrashHandler::MyStackWalk(EXCEPTION_POINTERS* pExcPtrs)
	{

		SYSTEMTIME st;
		GetLocalTime(&st);

		SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);

		if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) return;



		HANDLE hFile = NULL;
		unsigned long  numOfByteWritten = 0;

		char filename[MAX_PATH] = { 0, };
		generate_filename(filename, "StackDump", "txt");

		hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		char buff[1024] = { 0, };

		_snprintf_s(buff, 1024, _TRUNCATE, "Date:%d-%d-%d_%d:%d:%d\r\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "Process:%x\r\n", GetCurrentProcessId());
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "Thread:%x\r\n", GetCurrentThreadId());
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "Exception Type:0x%08x\r\n", pExcPtrs->ExceptionRecord->ExceptionCode);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);


		HANDLE		hProcess = GetCurrentProcess();
		HANDLE		hThread = GetCurrentThread();
		CONTEXT&	context = *pExcPtrs->ContextRecord;

		STACKFRAME	stackFrame = { 0, };

#if defined(_M_X64) || defined(_M_AMD64)
		stackFrame.AddrPC.Offset = context.Rip;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = context.Rsp;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = context.Rbp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;

		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "rax: %#p\trbx: %#p\r\n", context.Rax, context.Rbx);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "rcx: %#p\trdx: %#p\r\n", context.Rcx, context.Rdx);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "rsi: %#p\trdi: %#p\r\n", context.Rsi, context.Rdi);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "rbp: %#p\trsp: %#p\r\n", context.Rbp, context.Rsp);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "r8: %#p\tr9: %#p\r\n", context.R8, context.R9);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "r10: %#p\tr11: %#p\r\n", context.R10, context.R11);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "r12: %#p\tr13: %#p\r\n", context.R12, context.R13);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "r14: %#p\tr15: %#p\r\n", context.R14, context.R15);
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);


		for (int walkIdx = 0; walkIdx < 512; ++walkIdx) {
			if (0 == stackFrame.AddrPC.Offset) break;
			if (!StackWalk(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &stackFrame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL)) break;

			PDWORD64			dw64Displacement = 0;
			PDWORD				dwDisplacement = 0;
			char				chSymbol[256] = { 0, };

			PIMAGEHLP_SYMBOL	pSymbol = (PIMAGEHLP_SYMBOL)chSymbol;
			pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			pSymbol->MaxNameLength = sizeof(chSymbol) - sizeof(PIMAGEHLP_SYMBOL) + 1;

			if (SymGetSymFromAddr(hProcess, stackFrame.AddrPC.Offset, dw64Displacement, pSymbol))
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "%#p - %s() + %xh\r\n", stackFrame.AddrPC.Offset, pSymbol->Name, stackFrame.AddrPC.Offset - pSymbol->Address);
				WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
			}
			else
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "%#p - [Unknown Symbol:Error %u]\r\n", stackFrame.AddrPC.Offset, GetLastError());
				WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
			}

			IMAGEHLP_MODULE	module = { sizeof(IMAGEHLP_MODULE), 0 };
			if (SymGetModuleInfo(hProcess, stackFrame.AddrPC.Offset, &module) != FALSE)
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tImageName:%s\r\n", module.ImageName);
				_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tLoadedImageName:%s\r\n", module.LoadedImageName);
			}

			IMAGEHLP_LINE line = { sizeof(IMAGEHLP_LINE), 0 };
			for (int lineIdx = 0; lineIdx < 512; ++lineIdx)
			{
				if (SymGetLineFromAddr(hProcess, stackFrame.AddrPC.Offset - lineIdx, dwDisplacement, &line) != FALSE)
				{
					_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tFileName:%s\r\n", line.FileName);
					WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
					_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tLineNumber:%u\r\n", line.LineNumber);
					WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
					break;
				}
			}
		}

		BYTE* stack = (BYTE*)context.Rsp;
		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "stack %p - %p\r\n", stack, stack + ((sizeof(size_t) * 4) * 16));
		WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		for (int i = 0; i < sizeof(size_t) * 4; ++i) {
			_snprintf_s(buff, 1024, _TRUNCATE, "%p : ", stack + i * 16);
			WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
			for (int j = 0; j < 16; ++j) {
				_snprintf_s(buff, 1024, _TRUNCATE, "%02X ", stack[i * 16 + j]);
				WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
			}
			_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
			WriteFile(hFile, buff, static_cast<DWORD>(strlen(buff)), &numOfByteWritten, NULL);
		}
#else
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = context.Ebp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;

		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "eax: 0x%08x\tebx: 0x%08x\r\n", context.Eax, context.Ebx);
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "ecx: 0x%08x\tedx: 0x%08x\r\n", context.Ecx, context.Edx);
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "esi: 0x%08x\tedi: 0x%08x\r\n", context.Esi, context.Edi);
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "ebp: 0x%08x\tesp: 0x%08x\r\n", context.Ebp, context.Esp);
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);


		for (int walkIdx = 0; walkIdx < 512; ++walkIdx) {
			if (0 == stackFrame.AddrPC.Offset) break;
			if (!StackWalk(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &stackFrame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL)) break;

			PDWORD				dwDisplacement = 0;
			char				chSymbol[256] = { 0, };

			PIMAGEHLP_SYMBOL	pSymbol = (PIMAGEHLP_SYMBOL)chSymbol;
			pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			pSymbol->MaxNameLength = sizeof(chSymbol) - sizeof(PIMAGEHLP_SYMBOL) + 1;

			if (SymGetSymFromAddr(hProcess, stackFrame.AddrPC.Offset, dwDisplacement, pSymbol))
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "0x%08x - %s() + %xh\r\n", stackFrame.AddrPC.Offset, pSymbol->Name, stackFrame.AddrPC.Offset - pSymbol->Address);
				WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
			}
			else
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "0x%08x - [Unknown Symbol:Error %d]\r\n", stackFrame.AddrPC.Offset, GetLastError());
				WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
			}

			IMAGEHLP_MODULE	module = { sizeof(IMAGEHLP_MODULE), 0 };
			if (SymGetModuleInfo(hProcess, stackFrame.AddrPC.Offset, &module) != FALSE)
			{
				_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tImageName:%s\r\n", module.ImageName);
				_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tLoadedImageName:%s\r\n", module.LoadedImageName);
			}

			IMAGEHLP_LINE line = { sizeof(IMAGEHLP_LINE), 0 };
			for (int lineIdx = 0; lineIdx < 512; ++lineIdx)
			{
				if (SymGetLineFromAddr(hProcess, stackFrame.AddrPC.Offset - lineIdx, dwDisplacement, &line) != FALSE)
				{
					_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tFileName:%s\r\n", line.FileName);
					WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
					_snprintf_s(buff, 1024, _TRUNCATE, "\t\t\t\tLineNumber:%d\r\n", line.LineNumber);
					WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
					break;
				}
			}
		}

		BYTE* stack = (BYTE*)context.Esp;
		_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		_snprintf_s(buff, 1024, _TRUNCATE, "stack %08x - %08x\r\n", stack, stack + ((sizeof(size_t) * 4) * 16));
		WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		for (int i = 0; i < sizeof(size_t) * 4; ++i) {
			_snprintf_s(buff, 1024, _TRUNCATE, "%08X : ", stack + i * 16);
			WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
			for (int j = 0; j < 16; ++j) {
				_snprintf_s(buff, 1024, _TRUNCATE, "%02X ", stack[i * 16 + j]);
				WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
			}
			_snprintf_s(buff, 1024, _TRUNCATE, "\r\n");
			WriteFile(hFile, buff, strlen(buff), &numOfByteWritten, NULL);
		}
#endif
		SymCleanup(hProcess);

		CloseHandle(hFile);
	}

	// Structured exception handler
	LONG WINAPI CCrashHandler::SehHandler(PEXCEPTION_POINTERS pExceptionPtrs)
	{
		// Write minidump file

		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);


		TerminateProcess(GetCurrentProcess(), 1);

		// Unreacheable code  
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// CRT terminate() call handler
	void __cdecl CCrashHandler::TerminateHandler()
	{
		// Abnormal program termination (terminate() function was called)

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);
	}

	// CRT unexpected() call handler
	void __cdecl CCrashHandler::UnexpectedHandler()
	{
		// Unexpected error (unexpected() function was called)


		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);


		TerminateProcess(GetCurrentProcess(), 1);
	}

	// CRT Pure virtual method call handler
	void __cdecl CCrashHandler::PureCallHandler()
	{
		// Pure virtual function call

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);


		TerminateProcess(GetCurrentProcess(), 1);

	}


	// CRT invalid parameter handler
	void __cdecl CCrashHandler::InvalidParameterHandler(
		const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
	{
		pReserved;

		// Invalid parameter exception

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		// Write exception note
		do
		{
			char filename[MAX_PATH] = { 0, };
			generate_filename(filename, "InvalidParamDump", "txt");

			HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				unsigned long  numOfByteWritten = 0;
				wchar_t buff[1024] = { 0, };

				_snwprintf_s(buff, 1024, _TRUNCATE, L"Exception : %s\r\n", expression);
				WriteFile(hFile, buff, static_cast<DWORD>(wcslen(buff) * sizeof(wchar_t)), &numOfByteWritten, NULL);
				_snwprintf_s(buff, 1024, _TRUNCATE, L"Function : %s\r\n", function);
				WriteFile(hFile, buff, static_cast<DWORD>(wcslen(buff) * sizeof(wchar_t)), &numOfByteWritten, NULL);
				_snwprintf_s(buff, 1024, _TRUNCATE, L"File : %s\r\n", file);
				WriteFile(hFile, buff, static_cast<DWORD>(wcslen(buff) * sizeof(wchar_t)), &numOfByteWritten, NULL);
				_snwprintf_s(buff, 1024, _TRUNCATE, L"Line : %u\r\n", line);
				WriteFile(hFile, buff, static_cast<DWORD>(wcslen(buff) * sizeof(wchar_t)), &numOfByteWritten, NULL);

				CloseHandle(hFile);
			}
		} while (0);

		TerminateProcess(GetCurrentProcess(), 1);

	}

	// CRT new operator fault handler
	int __cdecl CCrashHandler::NewHandler(size_t)
	{
		// 'new' operator memory allocation exception

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);


		TerminateProcess(GetCurrentProcess(), 1);

		// Unreacheable code
		return 0;
	}

	// CRT SIGABRT signal handler
	void CCrashHandler::SigabrtHandler(int)
	{
		// Caught SIGABRT C++ signal

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);


		TerminateProcess(GetCurrentProcess(), 1);

	}

	// CRT SIGFPE signal handler
	void CCrashHandler::SigfpeHandler(int /*code*/, int subcode)
	{
		// Floating point exception (SIGFPE)

		EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);
	}

	// CRT sigill signal handler
	void CCrashHandler::SigillHandler(int)
	{
		// Illegal instruction (SIGILL)

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);

	}

	// CRT sigint signal handler
	void CCrashHandler::SigintHandler(int)
	{
		// Interruption (SIGINT)

		// Retrieve exception information
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);

	}

	// CRT SIGSEGV signal handler
	void CCrashHandler::SigsegvHandler(int)
	{
		// Invalid storage access (SIGSEGV)

		PEXCEPTION_POINTERS pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

		// Write minidump file
		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);
	}

	// CRT SIGTERM signal handler
	void CCrashHandler::SigtermHandler(int)
	{
		EXCEPTION_POINTERS* pExceptionPtrs = NULL;
		GetExceptionPointers(0, &pExceptionPtrs);

		g_dump_func(pExceptionPtrs);
		MyStackWalk(pExceptionPtrs);

		TerminateProcess(GetCurrentProcess(), 1);

	}

};//namespace MLN