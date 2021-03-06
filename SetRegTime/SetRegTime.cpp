#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "SetRegTime.h"

int resolve_functions() {

	HINSTANCE hinstStub = GetModuleHandleA("ntdll.dll");
	if (!hinstStub)
	{
		printf("Could not GetModuleHandle of NTDLL.DLL");
		return 1;
	}

	RtlInitAnsiString = (LPRTLINITANSISTRING)GetProcAddress(hinstStub, "RtlInitAnsiString");
	if (!RtlInitAnsiString) {
		printf("Could not find RtlInitAnsiString entry point in NTDLL.DLL");
		return 1;
	}
	RtlAnsiStringToUnicodeString = (LPRTLANSISTRINGTOUNICODESTRING)GetProcAddress(hinstStub, "RtlAnsiStringToUnicodeString");
	if (!RtlAnsiStringToUnicodeString) {
		printf("Could not find RtlAnsiStringToUnicodeString entry point in NTDLL.DLL");
		return 1;
	}
	NtOpenKey = (LPNTOPENKEY)GetProcAddress(hinstStub, "NtOpenKey");
	if (!NtOpenKey)
	{
		printf("Could not find NtOpenKey entry point in NTDLL.DLL");
		return 1;
	}
	NtClose = (LPNTCLOSE)GetProcAddress(hinstStub, "NtClose");
	if (!NtClose) {
		printf("Could not find NtClose entry point in NTDLL.DLL");
		return 1;
	}
	NtFlushKey = (LPNTCLOSE)GetProcAddress(hinstStub, "NtFlushKey");
	if (!NtClose) {
		printf("Could not find NtFlushKey entry point in NTDLL.DLL");
		return 1;
	}
	NtSetInformationKey = (LPNTSETINFORMATIONKEY)GetProcAddress(hinstStub, "NtSetInformationKey");
	if (!NtSetInformationKey)
	{
		printf("Could not find NtSetInformationKey entry point in NTDLL.DLL");
		return 1;
	}
	NtEnumerateKey = (LPNTENUMERATEKEY)GetProcAddress(hinstStub, "NtEnumerateKey");
	if (!NtEnumerateKey)
	{
		printf("Could not find NtEnumerateKey entry point in NTDLL.DLL");
		return 1;
	}

	return 0;
}

HANDLE open_key(const char * keyname)
{
	HANDLE key = NULL;
	HANDLE hMachineReg = 0x00000000;
	UNICODE_STRING unicode_keyname;
	ANSI_STRING ansi_keyname;
	OBJECT_ATTRIBUTES ObjectAttributes;

	//Initialize the unicode key name
	memset(&ansi_keyname, 0, sizeof(ansi_keyname));
	RtlInitAnsiString(&ansi_keyname, keyname);
	memset(&unicode_keyname, 0, sizeof(unicode_keyname));
	RtlAnsiStringToUnicodeString(&unicode_keyname, &ansi_keyname, TRUE);

	//Setup the object attributes
	InitializeObjectAttributes(&ObjectAttributes, &unicode_keyname, OBJ_CASE_INSENSITIVE, hMachineReg, NULL);

	//Open the key
	NtStatus = NtOpenKey(&key, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(NtStatus)) {
		printf("[!]NtOpenKey Error trying to open %s: 0x%lx\n", keyname, NtStatus);
		return INVALID_HANDLE_VALUE;
	}
	return key;
}

int convert_time(const char * timestamp, LARGE_INTEGER * out_timestamp)
{
	SYSTEMTIME system_time;
	FILETIME file_time;

	//Expected format: YYYY:MM:DD:HH:UU:SS:III (UU = minutes, III = milliseconds)
	if (strlen(timestamp) != 23) {
		printf("Bad timestamp: %s\n", timestamp);
		return 1;
	}

	memset(&system_time, 0, sizeof(system_time));
	system_time.wYear         = atoi(timestamp);
	system_time.wMonth        = atoi(timestamp + 5);
	system_time.wDay          = atoi(timestamp + 8);
	system_time.wHour         = atoi(timestamp + 11);
	system_time.wMinute       = atoi(timestamp + 14);
	system_time.wSecond       = atoi(timestamp + 17);
	system_time.wMilliseconds = atoi(timestamp + 20);

	if (!SystemTimeToFileTime(&system_time, &file_time)) {
		printf("SystemTimeToFileTime failed for %s\n", timestamp);
		return 1;
	}

	memset(out_timestamp, 0, sizeof(LARGE_INTEGER));
	out_timestamp->HighPart = file_time.dwHighDateTime;
	out_timestamp->LowPart = file_time.dwLowDateTime;
	return 0;
}

int set_timestamp(HANDLE key, LARGE_INTEGER timestamp)
{
	KEY_WRITE_TIME_INFORMATION info;

	//Copy in the timestamp info
	memset(&info, 0, sizeof(info));
	info.LastWriteTime = timestamp;

	//Call NtSetInformationKey to set the timestamp
	NtStatus = NtSetInformationKey(key, KeyWriteTimeInformation, &info, sizeof(info));
	if (!NT_SUCCESS(NtStatus))
		return 1;
	return 0;
}

int flush_and_close_key(HANDLE key)
{
	NtStatus = NtFlushKey(key);
	if (!NT_SUCCESS(NtStatus))
		return 1;
	
	NtStatus = NtClose(key);
	if (!NT_SUCCESS(NtStatus))
		return 1;
	return 0;
}

int change_key_timestamp(const char * key_name, LARGE_INTEGER timestamp, int recursive, int ignore_errors) {
	HANDLE key;
	ULONG index = 0, length;
	KEY_BASIC_INFORMATION key_information;
	char child_key_name[4096];

	key = open_key(key_name);
	if (key == INVALID_HANDLE_VALUE)
		return 1;

	if (recursive) {
		while (1) {
			memset(&key_information, 0, sizeof(key_information));
			NtStatus = NtEnumerateKey(key, index, KeyBasicInformation, &key_information, sizeof(key_information), &length);
			if (!NT_SUCCESS(NtStatus))
				break;
			memset(child_key_name, 0, sizeof(child_key_name));
			snprintf(child_key_name, sizeof(child_key_name), "%s\\%ws", key_name, key_information.Name);
			if (change_key_timestamp(child_key_name, timestamp, recursive, ignore_errors) && !ignore_errors)
				return 1;
			index++;
		}
	}

	set_timestamp(key, timestamp);
	if (flush_and_close_key(key)) {
		printf("Failed to flush and close key %s\n", key_name);
		return 1;
	}
	return 0;
}

int main(int argc, char ** argv)
{
	LARGE_INTEGER timestamp;
	int recursive = 0, ignore_errors = 0;

	if (argc < 3) {
		printf("Usage: SetRegTime keyname timestamp [-r [-i]]\n"
			"  keyname = The key to change the timestamp.\n"
			"    Example: \\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\n"
			"  timestamp = The new last write time to set. Format: YYYY:MM:DD:HH:UU:SS:III (UU = minutes, III = milliseconds)\n"
			"    Example: 2020:05:01:19:04:59:142\n"
			"  -r = Specify that SetRegTime should recursively change the last write time of any subkeys as well\n"
			"  -i = Ignore errors and continue when recursively changing timestamps and a subkey fails\n"
		);
		return 1;	
	}
	
	if (resolve_functions())
		return 1;

	convert_time(argv[2], &timestamp);
	if (argc > 3 && !strcmp(argv[3], "-r")) {
		recursive = 1;
		if (argc > 4 && !strcmp(argv[4], "-i"))
			ignore_errors = 1;
	}
	return change_key_timestamp(argv[1], timestamp, recursive, ignore_errors);
}
