#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <winternl.h>

typedef NTSTATUS(NTAPI* NtRaiseHardErrorFunc)(
	NTSTATUS ErrorStatus,
	ULONG NumberOfParameters,
	ULONG UnicodeStringParameterMask,
	PULONG_PTR Parameters,
	ULONG ValidResponseOptions,
	PULONG Response
	);

extern "C" NTSYSAPI NTSTATUS NTAPI NtRaiseHardError(
	NTSTATUS ErrorStatus,
	ULONG NumberOfParameters,
	ULONG UnicodeStringParameterMask,
	PULONG_PTR Parameters,
	ULONG ValidResponseOptions,
	PULONG Response
);

void InitializeUnicodeString(PUNICODE_STRING unicodeString, PCWSTR buffer) {
	unicodeString->Length = wcslen(buffer) * sizeof(wchar_t);
	unicodeString->MaximumLength = unicodeString->Length + sizeof(wchar_t);
	unicodeString->Buffer = const_cast<PWSTR>(buffer);
}

int main(int argc, char* argv[]) {
	// Check if an error code parameter is provided
	if (argc != 2) {
		std::cout << "RaiseHError - Invoke errors from NTSTATUS.H. \n" << std::endl;
		std::cout << "Made by dragogos using ChatGPT. \n" << std::endl;
		std::cout << "Usage: RaiseHError <hex error code> \n" << std::endl;
		std::cout << "Does not blue screen the computer. Use this to preview error codes. \n" << std::endl;
		return 1;
	}

	// Parse the error code parameter
	ULONG errorCode = 0;
	try {
		errorCode = std::strtoul(argv[1], nullptr, 16);
	}
	catch (std::exception&) {
		std::cout << "Invalid error code parameter" << std::endl;
		return 1;
	}

	// Load the NtRaiseHardError function dynamically
	HMODULE ntdllModule = GetModuleHandle(L"ntdll.dll");
	if (!ntdllModule) {
		std::cout << "Failed to load ntdll.dll" << std::endl;
		return 1;
	}

	NtRaiseHardErrorFunc raiseHardError = reinterpret_cast<NtRaiseHardErrorFunc>(
		GetProcAddress(ntdllModule, "NtRaiseHardError"));
	if (!raiseHardError) {
		std::cout << "Failed to find NtRaiseHardError function" << std::endl;
		return 1;
	}

	// Prepare the parameters for NtRaiseHardError
	UNICODE_STRING errorString;
	InitializeUnicodeString(&errorString, L"Error message");

	ULONG_PTR parameters[4] = { 0 };
	ULONG response = 0;

	// Call NtRaiseHardError with the specified error code
	ULONG ValidResponseOptions = 1; // OptionOkOnly
	NTSTATUS status = raiseHardError(
		static_cast<NTSTATUS>(errorCode),
		1,
		0,
		parameters,
		ValidResponseOptions,
		&response
	);

	if (NT_SUCCESS(status)) {
		std::cout << "Successfully raised error with code: " << std::hex << errorCode << std::endl;
	}
	else {
		std::cout << "Failed to raise error. Status code: " << std::hex << status << std::endl;
	}

	return 0;
}