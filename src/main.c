#include <Windows.h>
SIZE_T _;
#define PATCH_ATTEMPTS 	60
#define PATCH_WAIT 		2000

#define LOGIN_PORT		2107
#define REM_ADDR		(LPVOID)(0x20001000+0x3F744E)

typedef NTSTATUS(WINAPI* _RtlAdjustPrivilege)(DWORD dwPrivilege, 
	BOOLEAN bEnablePrivilege, BOOLEAN bIsThreadPriv, PBOOLEAN lpPreviousValue);

VOID Fail(LPCWSTR lpReason) {
	MessageBoxW(NULL, lpReason, L"Error", MB_ICONERROR);
	ExitProcess(-1);
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, INT nShowCmd) {
	//hide console window
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	//initialize privileges
	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	_RtlAdjustPrivilege RtlAdjustPrivilege = 
		(_RtlAdjustPrivilege) GetProcAddress(hNtdll, "RtlAdjustPrivilege");
	if (!RtlAdjustPrivilege) 
		Fail(L"Unable to get RtlAdjustPrivilege procaddr!");
	NTSTATUS dwStatus = 
		RtlAdjustPrivilege(20, TRUE, FALSE, (PBOOLEAN)&_);
	if (dwStatus)
		Fail(L"Failed to obtain SeDebugPrivilege! \
(are you running as admin?)");
	//initialize CPW structures
	LPSTARTUPINFOW lpStartupInfo = HeapAlloc(
		GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(STARTUPINFOW));
	LPPROCESS_INFORMATION lpProcInfo = HeapAlloc(GetProcessHeap(), 
		HEAP_ZERO_MEMORY, sizeof(PROCESS_INFORMATION));
	//call CPW
	BOOL bSuccess = CreateProcessW(L"system\\l2.exe", NULL, NULL, NULL, FALSE, 
		0, NULL, NULL, lpStartupInfo, lpProcInfo);
	//error handling
	if (!bSuccess)
		Fail(L"Failed to open l2.exe!");
	//perform cleanup
	HANDLE hTarget = lpProcInfo->hProcess;
	CloseHandle(lpProcInfo->hThread);
	HeapFree(GetProcessHeap(), 0, lpStartupInfo);
	HeapFree(GetProcessHeap(), 0, lpProcInfo);
	//attempt to patch
	USHORT wLoginPort = LOGIN_PORT;

	for (DWORD i = 0; i < PATCH_ATTEMPTS; i++) {
		WriteProcessMemory(hTarget, REM_ADDR, &wLoginPort, 2, &_);

		DWORD dwWaitReason = WaitForSingleObject(hTarget, PATCH_WAIT);
		if (dwWaitReason != WAIT_TIMEOUT)
			ExitProcess(0);
	}

	ExitProcess(0);
}
