#pragma once

#include <windows.h>
#include <stdlib.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) == STATUS_SUCCESS)
#define OBJ_CASE_INSENSITIVE	0x00000040L
#define KeyWriteTimeInformation 0
#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );        \
    (p)->RootDirectory = r;                           \
    (p)->Attributes = a;                              \
    (p)->ObjectName = n;                              \
    (p)->SecurityDescriptor = s;                      \
    (p)->SecurityQualityOfService = NULL;             \
    }

typedef enum _KEY_INFORMATION_CLASS {
	KeyBasicInformation,
	KeyNodeInformation,
	KeyFullInformation,
	KeyNameInformation,
	KeyCachedInformation,
	KeyFlagsInformation,
	KeyVirtualizationInformation,
	KeyHandleTagsInformation,
	MaxKeyInfoClass
} KEY_INFORMATION_CLASS;

typedef struct _KEY_BASIC_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG TitleIndex;
	ULONG NameLength;
	WCHAR Name[4096];
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef struct _STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PCHAR Buffer;
} STRING;
typedef STRING ANSI_STRING;
typedef STRING *PANSI_STRING;

typedef struct _KEY_WRITE_TIME_INFORMATION {
	LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION;
typedef KEY_WRITE_TIME_INFORMATION *PKEY_WRITE_TIME_INFORMATION;


typedef NTSTATUS(STDAPICALLTYPE RTLINITANSISTRING)
(
	IN OUT PANSI_STRING DestinationString,
	IN LPCSTR SourceString
);
typedef RTLINITANSISTRING FAR * LPRTLINITANSISTRING;

typedef NTSTATUS(STDAPICALLTYPE RTLANSISTRINGTOUNICODESTRING)
(
	IN OUT PUNICODE_STRING	DestinationString,
	IN PANSI_STRING			SourceString,
	IN BOOLEAN				AllocateDestinationString
);
typedef RTLANSISTRINGTOUNICODESTRING FAR * LPRTLANSISTRINGTOUNICODESTRING;

typedef NTSTATUS(STDAPICALLTYPE NTOPENKEY)
(
	IN HANDLE				KeyHandle,
	IN ULONG				DesiredAccess,
	IN POBJECT_ATTRIBUTES	ObjectAttributes
);
typedef NTOPENKEY FAR * LPNTOPENKEY;

typedef NTSTATUS(STDAPICALLTYPE NTCLOSE)
(
	IN HANDLE KeyHandle
);
typedef NTCLOSE FAR * LPNTCLOSE;

typedef NTSTATUS(STDAPICALLTYPE NTFLUSHKEY)
(
	IN HANDLE KeyHandle
);
typedef NTFLUSHKEY FAR * LPNTFLUSHKEY;

typedef NTSTATUS(STDAPICALLTYPE NTSETINFORMATIONKEY)
(
	IN HANDLE KeyHandle,
	IN DWORD number,
	IN PKEY_WRITE_TIME_INFORMATION info,
	IN DWORD size
);
typedef NTSETINFORMATIONKEY FAR * LPNTSETINFORMATIONKEY;

typedef NTSTATUS(STDAPICALLTYPE NTENUMERATEKEY)
(
	IN HANDLE KeyHandle,
	IN ULONG index,
	IN KEY_INFORMATION_CLASS information_type,
	IN void * key_information,
	IN ULONG size,
	OUT ULONG * result_size
);
typedef NTENUMERATEKEY FAR * LPNTENUMERATEKEY;

LPRTLINITANSISTRING				RtlInitAnsiString;
LPRTLANSISTRINGTOUNICODESTRING	RtlAnsiStringToUnicodeString;
LPNTOPENKEY						NtOpenKey;
LPNTCLOSE						NtClose;
LPNTFLUSHKEY					NtFlushKey;
LPNTSETINFORMATIONKEY			NtSetInformationKey;
LPNTENUMERATEKEY				NtEnumerateKey;



NTSTATUS NtStatus = STATUS_SUCCESS;
