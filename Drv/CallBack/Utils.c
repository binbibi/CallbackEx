#include <ntddk.h>
#include "Utils.h"
#include "Global.h"
#include "Native.h"


KEVENT kEvent;

BOOLEAN GetDriverInformation(
	_In_ wchar_t* DriverName,
	_In_ PDRIVER_OBJECT pDriver,
	_Out_ UINT64* ModuleBase,
	_Out_ UINT64* ModuleSize
)
{
	UINT64 pLdrDataEntry = (UINT64)pDriver->DriverSection;
	UINT64 pHead = pLdrDataEntry;
	do
	{
		if (MmIsAddressValid((PVOID)(pLdrDataEntry + 0x58)))
		{
			PUNICODE_STRING pusName = (PUNICODE_STRING)(pLdrDataEntry + 0x58);
			kprintf("%wZ\n", pusName);

			if (MmIsAddressValid(pusName->Buffer))
			{
				if (!memcmp(pusName->Buffer, DriverName, sizeof(DriverName)))
				{
					if (MmIsAddressValid((PVOID)(pLdrDataEntry + 0x30)))
						*ModuleBase = *(ULONGLONG*)(pLdrDataEntry + 0x30);

					if (MmIsAddressValid((PVOID)(pLdrDataEntry + 0x40)))
						*ModuleSize = *(ULONG*)(pLdrDataEntry + 0x40);

					return TRUE;

				}
			}
		}

		pLdrDataEntry = *(UINT64*)pLdrDataEntry;
	} while (pHead != pLdrDataEntry);
	return FALSE;
}


UINT64 FindPattern(
	_In_ UINT64 Base,
	_In_ UINT64 Size,
	_In_ char* Pattern,
	_In_ size_t PatternSize
)
{
	__try
	{
		for (UINT64 i = Base; i < (Base + Size); ++i)
		{
			for (int j = 0; j < PatternSize; ++j)
			{
				if ((UCHAR)Pattern[j] == *(UCHAR*)(i + j))
				{					
					if (j == (PatternSize - 1))
						return i;
				}
				else
					break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		kprintf("%s: Exception!\n", __FUNCTION__);
		return 0;
	}
	return 0;
}


KIWI_OS_INDEX getWindowsIndex()

{
	switch (g_DynData.dwOsBuildNumber)

	{

	case 2600:

		return KiwiOsIndex_XP;

		break;

	case 3790:

		return KiwiOsIndex_2K3;

		break;

	case 6000:

	case 6001:

	case 6002:

		return KiwiOsIndex_VISTA;

		break;

	case 7600:

	case 7601:

		return KiwiOsIndex_7;

		break;

	case 8102:

	case 8250:

	case 9200:

		return KiwiOsIndex_8;

	case 9431:

	case 9600:

		return KiwiOsIndex_BLUE;

		break;

	case 10240:

		return KiwiOsIndex_10_1507;

		break;

	case 10586:

		return KiwiOsIndex_10_1511;

		break;

	case 14393:

		return KiwiOsIndex_10_1607;

		break;

	case 15063:

		return KiwiOsIndex_10_1707;

		break;

	case 16299:
		return KiwiOsIndex_10_1707;
		break;

	default:

		return KiwiOsIndex_UNK;

	}

}


BOOLEAN InitDynamicData()
{
	RTL_OSVERSIONINFOW VersionInfo = { 0 };
	BOOLEAN bStatus = TRUE;

	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	NTSTATUS status = RtlGetVersion(&VersionInfo);
	
	if (NT_SUCCESS(status))
	{	
		g_DynData.dwOsBuildNumber = VersionInfo.dwBuildNumber;
		
		if (VersionInfo.dwBuildNumber >= 9200)
		{
			g_DynData.CallbackListOffset = 0xC8;
			g_DynData.TypeLockOffest = 0xB8;
		}
			

		else if (VersionInfo.dwBuildNumber >= 7600)
		{
			g_DynData.CallbackListOffset = 0xC0;
			g_DynData.TypeLockOffest = 0xB0;
		}

		else
			bStatus = FALSE;
	}

	KiwiOsIndex = getWindowsIndex();

	return bStatus;
}


VOID KSleep(ULONG ulMilliseconds)
{
	LARGE_INTEGER my_interval;
	my_interval.QuadPart = DELAY_ONE_MILLISECOND;
	my_interval.QuadPart *= ulMilliseconds;
	KeDelayExecutionThread(KernelMode, 0, &my_interval);
}


// 实现驱动路径的欺骗
VOID FakeSelf(__in PDRIVER_OBJECT pSelfDriver)
{
	// 保存这个 
	WCHAR szDriverName[256];
	WCHAR szCheatDrvName[] = L"AUSAAUDIO";
	PVOID pDrvName = NULL;
	PKLDR_DATA_TABLE_ENTRY pSelfModule = NULL;

	pSelfModule = (PKLDR_DATA_TABLE_ENTRY)pSelfDriver->DriverSection;
	if (pSelfDriver->DriverName.Buffer)
	{
		wcsncpy(szDriverName, pSelfDriver->DriverName.Buffer, pSelfDriver->DriverName.Length);
		ExFreePool(pSelfDriver->DriverName.Buffer);

		pDrvName = ExAllocatePool(PagedPool, (wcslen(szCheatDrvName) + 1) * sizeof(wchar_t));
		wcsncpy(pDrvName, szCheatDrvName, wcslen(szCheatDrvName));
		pSelfDriver->DriverName.Buffer = pDrvName;
		pSelfDriver->DriverName.MaximumLength = pSelfDriver->DriverName.Length = wcslen(szCheatDrvName) + 1;
	}

	for (PLIST_ENTRY pListEntry = pSelfModule->InLoadOrderLinks.Flink->Flink; pListEntry != &pSelfModule->InLoadOrderLinks; pListEntry = pListEntry->Flink)
	{
		PKLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		kprintf("%s Driver Path is %wZ \n", __FUNCTION__, &pEntry->FullDllName);
	}

	// From WRK
	// DataTableEntry->FullDllName.Buffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
	// PrefixedImageName.Length + sizeof(UNICODE_NULL),'TDmM');
	// 
	// 可以释放这个内存
	if (pSelfModule->FullDllName.Buffer)
	{
		ExFreePool(pSelfModule->FullDllName.Buffer);
		pDrvName = ExAllocatePool(PagedPool, pSelfModule->FullDllName.Length);
		memset(pDrvName, 0, pSelfModule->FullDllName.Length);
		pSelfModule->FullDllName.Buffer = pDrvName;
		pSelfModule->FullDllName.MaximumLength = pSelfModule->FullDllName.Length;
	}

	// From WRK
	// DataTableEntrySize = sizeof (KLDR_DATA_TABLE_ENTRY) + DebugInfoSize + BaseName.Length + sizeof(UNICODE_NULL);
	// RtlCopyMemory (DataTableEntry->BaseDllName.Buffer, BaseName.Buffer, BaseName.Length);
	// DataTableEntry->BaseDllName.Buffer[BaseName.Length / sizeof(WCHAR)] = UNICODE_NULL;
	// 	
	// 不能直接释放这个buffer,原因在上面
	if (pSelfModule->BaseDllName.Buffer)
	{
		// ExFreePool(pSelfModule->BaseDllName.Buffer);
		// pDrvName = ExAllocatePool(PagedPool, pSelfModule->BaseDllName.Length);
		memset(pSelfModule->BaseDllName.Buffer, 0, pSelfModule->BaseDllName.Length);
		// pSelfModule->BaseDllName.Buffer = pDrvName;
		// pSelfModule->BaseDllName.MaximumLength = pSelfModule->BaseDllName.Length;
	}
}


VOID MyThreadFunc(IN PVOID context)
{	// 延时处理 10s
	KSleep(10 * 1000);

	FakeSelf((PDRIVER_OBJECT)context);
	
	KeSetEvent(&kEvent, 0, FALSE);
	PsTerminateSystemThread(STATUS_SUCCESS);
}


VOID MyCreateThread(PVOID context)
{
	HANDLE   hThread;
	NTSTATUS  status;

	KeInitializeEvent(&kEvent, SynchronizationEvent, TRUE);
	status = PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, MyThreadFunc, context);
	if (!NT_SUCCESS(status))
	{
		return;
	}

	ZwClose(hThread);
	KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, 0);
}


