#include <ntddk.h>
#include "Utils.h"
#include "Global.h"


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