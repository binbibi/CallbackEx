#include <fltKernel.h>
#include <Global.h>
#include <minifilter.h>
#include <string.h>



const ULONG MF_OffSetTable[KiwiOsIndex_MAX][MF_MAX] =

{

	/* CallbackOffset, CallbackPreOffset, CallbackPostOffset, CallbackVolumeNameOffset */

#ifdef _M_IX86

	/* UNK	*/{ 0 },

	/* XP	*/{ 0x007c, 0x000c, 0x0010, 0x002c },

	/* 2K3	*/{ 0x007c, 0x000c, 0x0010, 0x002c },

	/* VISTA*/{ 0x004c, 0x000c, 0x0010, 0x0030 },

	/* 7	*/{ 0x004c, 0x000c, 0x0010, 0x0030 },

	/* 8	*/{ 0x004c, 0x000c, 0x0010, 0x0030 },

	/* BLUE	*/{ 0x004c, 0x000c, 0x0010, 0x0030 },

	/* 10_1507*/{ 0x004c, 0x000c, 0x0010, 0x0040 },

	/* 10_1511*/{ 0x004c, 0x000c, 0x0010, 0x0040 },

	/* 10_1607*/{ 0x004c, 0x000c, 0x0010, 0x0040 },

	/* 10_1707*/{ 0x004c, 0x000c, 0x0010, 0x0040 },

#else

	/* UNK	*/{ 0 },

	/* XP	*/{ 0 },

	/* 2K3	*/{ 0x00e8, 0x0018, 0x0020, 0x0048 },

	/* VISTA*/{ 0x0090, 0x0018, 0x0020, 0x0050 },

	/* 7	*/{ 0x0090, 0x0018, 0x0020, 0x0050 },

	/* 8	*/{ 0x0090, 0x0018, 0x0020, 0x0050 },

	/* BLUE	*/{ 0x0090, 0x0018, 0x0020, 0x0050 },

	/* 10_1507*/{ 0x0090, 0x0018, 0x0020, 0x0060 },

	/* 10_1511*/{ 0x0090, 0x0018, 0x0020, 0x0060 },

	/* 10_1607*/{ 0x0090, 0x0018, 0x0020, 0x0060 },

	/* 10_1707*/{ 0x0090, 0x0018, 0x0020, 0x0060 },

#endif

};


const WCHAR *irpToName[] = {

	L"CREATE",

	L"CREATE_NAMED_PIPE",

	L"CLOSE",

	L"READ",

	L"WRITE",

	L"QUERY_INFORMATION",

	L"SET_INFORMATION",

	L"QUERY_EA",

	L"SET_EA",

	L"FLUSH_BUFFERS",

	L"QUERY_VOLUME_INFORMATION",

	L"SET_VOLUME_INFORMATION",

	L"DIRECTORY_CONTROL",

	L"FILE_SYSTEM_CONTROL",

	L"DEVICE_CONTROL",

	L"INTERNAL_DEVICE_CONTROL",

	L"SHUTDOWN",

	L"LOCK_CONTROL",

	L"CLEANUP",

	L"CREATE_MAILSLOT",

	L"QUERY_SECURITY",

	L"SET_SECURITY",

	L"POWER",

	L"SYSTEM_CONTROL",

	L"DEVICE_CHANGE",

	L"QUERY_QUOTA",

	L"SET_QUOTA",

	L"PNP",

};


FLT_PREOP_CALLBACK_STATUS
FLTAPI MFDPreHandler(
	_Inout_ PFLT_CALLBACK_DATA pData,
	_In_ PCFLT_RELATED_OBJECTS pFltObjects,
	_Out_ PVOID *pCompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS fltRetStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pFltObjects);
	UNREFERENCED_PARAMETER(pCompletionContext);
	ULONG ProcessId, ThreadId;

	// 增加代码;防止编译器优化；MFDPreHandler和MFDPostHandler同时空实现,会导致编译器优化成一个函数;晕
	if (pData && pData->Thread)
	{
		ULONG ProcessId = FltGetRequestorProcessId(pData);
		ULONG ThreadId = (ULONG)PsGetThreadId(pData->Thread);
	}

	return fltRetStatus;
}


FLT_POSTOP_CALLBACK_STATUS
FLTAPI MFDPostHandler(
	_Inout_ PFLT_CALLBACK_DATA pData,
	_In_ PCFLT_RELATED_OBJECTS pFltObjects,
	_In_opt_ PVOID pCompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	FLT_POSTOP_CALLBACK_STATUS fltRetStatus = FLT_POSTOP_FINISHED_PROCESSING;
	
	return fltRetStatus;
}


NTSTATUS kkll_m_minifilters_list()
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	ULONG NumberFiltersReturned, NumberInstancesReturned, sizeOfBuffer;
	PFLT_FILTER *FilterList = NULL;
	PFLT_INSTANCE *InstanceList = NULL;
	// PFLT_VOLUME Volume = NULL;
	PFILTER_FULL_INFORMATION myFilterFullInformation = NULL;
	PVOID pCallBack, preCallBack, postCallBack, AfterpreCallBack, AfterpostCallBack;
	ULONG i, j, k;
	wchar_t  FilterString[256];

	status = FltEnumerateFilters(NULL, 0, &NumberFiltersReturned);
	if (!((status == STATUS_BUFFER_TOO_SMALL) && NumberFiltersReturned))
	{
		goto __end;
	}

	sizeOfBuffer = sizeof(PFLT_FILTER) * NumberFiltersReturned;
	FilterList = (PFLT_FILTER *)ExAllocatePoolWithTag(NonPagedPool, sizeOfBuffer, POOL_TAG);
	if (!FilterList)
	{
		goto __end;
	}
	
	status = FltEnumerateFilters(FilterList, sizeOfBuffer, &NumberFiltersReturned);
	if (!NT_SUCCESS(status))
	{
		goto __end;
	}
	
	for (i = 0; (i < NumberFiltersReturned); i++)
	{
		status = FltGetFilterInformation(FilterList[i], FilterFullInformation, NULL, 0, &sizeOfBuffer);

		if (!((status == STATUS_BUFFER_TOO_SMALL) && sizeOfBuffer))
		{
			goto __clean;
		}
		
		myFilterFullInformation = (PFILTER_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, sizeOfBuffer, POOL_TAG);
		if (!myFilterFullInformation)
		{
			goto __clean;
		}

		status = FltGetFilterInformation(FilterList[i], FilterFullInformation, myFilterFullInformation, sizeOfBuffer, &sizeOfBuffer);
		if (!NT_SUCCESS(status))
		{
			goto __clean;
		}

		// RtlInitUnicodeString(&FilterString, L"QQSysMonX64");

		// status = kprintfx(outBuffer, L"[%.2u] %.*s\n", i, myFilterFullInformation->FilterNameLength / sizeof(WCHAR), myFilterFullInformation->FilterNameBuffer);
		memset(FilterString, 0x00, 256 * 2);
		RtlStringCbCopyNW(FilterString, 256 * 2, myFilterFullInformation->FilterNameBuffer, myFilterFullInformation->FilterNameLength);
		
		status = FltEnumerateInstances(NULL, FilterList[i], NULL, 0, &NumberInstancesReturned);
		if (!((status == STATUS_BUFFER_TOO_SMALL) && NumberInstancesReturned))
		{
			goto __clean;
		}
		
		InstanceList = (PFLT_INSTANCE *)ExAllocatePoolWithTag(NonPagedPool, sizeof(PFLT_INSTANCE) * NumberInstancesReturned, POOL_TAG);
		if (!InstanceList)
		{
			goto __clean;
		}
		
		status = FltEnumerateInstances(NULL, FilterList[i], InstanceList, NumberInstancesReturned, &NumberInstancesReturned);
		if (!NT_SUCCESS(status))
		{
			goto __clean;
		}

		kprintf("%s: ------Start print------- \n", __FUNCTION__);
		
		for (j = 0;   (j < NumberInstancesReturned); j++)
		{
			for (k = 0x16; k < 0x32; k++)
			{
				pCallBack = (PVOID) *(PULONG_PTR)((((ULONG_PTR)InstanceList[j]) + MF_OffSetTable[KiwiOsIndex][CallbackOffset]) + sizeof(PVOID)*k);
				if (pCallBack)
				{
					preCallBack = (PVOID) *(PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPreOffset]);
					postCallBack = (PVOID) *(PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPostOffset]);

					kprintf("%s: precallback is %p, post callback is %p \n", __FUNCTION__, preCallBack, postCallBack);

					if (_wcsicmp(FilterString, L"QQSysMonX64") == 0)
					{
						InterlockedExchangePointer((PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPostOffset]), MFDPostHandler);
						InterlockedExchangePointer((PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPreOffset]), MFDPreHandler);
						
						AfterpreCallBack = (PVOID) *(PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPreOffset]);
						AfterpostCallBack = (PVOID) *(PULONG_PTR)(((ULONG_PTR)pCallBack) + MF_OffSetTable[KiwiOsIndex][CallbackPostOffset]);
						kprintf("%s: After Anti precallback is %p, post callback is %p \n", __FUNCTION__, AfterpreCallBack, AfterpostCallBack);
					}
				}
			}
		}
		//FileRenameInformation
	    //IRP_MJ_SET_INFORMATION
		
		kprintf("%s: ------ End print ------- \n", __FUNCTION__);
		
	
	__clean:
		if (myFilterFullInformation)
		{
			ExFreePoolWithTag(myFilterFullInformation, POOL_TAG);
			myFilterFullInformation = NULL;
		}
		if (InstanceList)
		{
			ExFreePoolWithTag(InstanceList, POOL_TAG);
			InstanceList = NULL;
		}
		FltObjectDereference(FilterList[i]);		
	}

__end:
	if (FilterList)
	{
		ExFreePoolWithTag(FilterList, POOL_TAG);
	}
	FilterList = NULL;
	return status;
}