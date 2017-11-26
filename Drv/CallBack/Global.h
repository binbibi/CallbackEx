#pragma once

#ifndef GLOBAL_H
#define GLOBAL_H
#include <ntstrsafe.h>
#include <string.h>




#define POOL_TAG	'kiwi'


extern char * PsGetProcessImageFileName(PEPROCESS monProcess);

extern NTSYSAPI NTSTATUS NTAPI ZwSetInformationProcess(__in HANDLE ProcessHandle, __in PROCESSINFOCLASS ProcessInformationClass, __in_bcount(ProcessInformationLength) PVOID ProcessInformation, __in ULONG ProcessInformationLength);

extern NTSYSAPI NTSTATUS NTAPI ZwUnloadKey(IN POBJECT_ATTRIBUTES DestinationKeyName);



typedef struct _KIWI_BUFFER {

	size_t * szBuffer;

	PWSTR * Buffer;

} KIWI_BUFFER, *PKIWI_BUFFER;


#define kprintf(FORMAT, ...)				DbgPrintEx(77, 0, FORMAT, __VA_ARGS__)

#define DeviceName							L"\\Device\\RTKT00"
#define DosDeviceName						L"\\DosDevices\\RTKT00"

#define CTL_GET_DRIVERINFO					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CTL_DISABLE_OB_CALLBACKS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x702, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CTL_RESTORE_OB_CALLBACKS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x703, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CTL_DISABLE_IMAGE_CALLBACK			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x704, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CTL_DISABLE_PROCESS_CALLBACK		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x705, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CTL_DISABLE_THREAD_CALLBACK			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x706, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _Module
{
	UINT64 Base, Size;
}Module, *PModule;

typedef struct _DRIVERNAME
{
	wchar_t Name[32];
}DRIVERNAME, *PDRIVERNAME;

typedef struct _DYNDATA
{
	UINT64 CallbackListOffset;
	UINT64 TypeLockOffest;
	ULONG dwOsBuildNumber;

}DYNDATA, *PDYNDATA;

typedef enum _KIWI_OS_INDEX {

	KiwiOsIndex_UNK = 0,

	KiwiOsIndex_XP = 1,

	KiwiOsIndex_2K3 = 2,

	KiwiOsIndex_VISTA = 3,

	KiwiOsIndex_7 = 4,

	KiwiOsIndex_8 = 5,

	KiwiOsIndex_BLUE = 6,

	KiwiOsIndex_10_1507 = 7,

	KiwiOsIndex_10_1511 = 8,

	KiwiOsIndex_10_1607 = 9,

	KiwiOsIndex_10_1707 = 10,

	KiwiOsIndex_MAX = 11,

} KIWI_OS_INDEX, *PKIWI_OS_INDEX;

extern DYNDATA g_DynData;
extern Module DriverInfo;
extern PDRIVER_OBJECT g_pDriverObject;


KIWI_OS_INDEX KiwiOsIndex;

#endif // !GLOBAL_H