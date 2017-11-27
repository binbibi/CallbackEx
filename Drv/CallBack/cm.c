#include <ntddk.h>
#include "cm.h"
#include "Global.h"
#include "Utils.h"



extern KIWI_OS_INDEX KiwiOsIndex;
extern DYNDATA g_DynData;



NTSTATUS FindCallbackListHead(ULONG_PTR* ppCallbackListHead)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG_PTR i = 0;
	LONG OffsetAddr64 = 0;
	ULONG_PTR OffsetAddr = 0;
	ULONG_PTR NotifyRoutine = 0;
	UNICODE_STRING unCallback;
	ULONG_PTR lpCmUnRegisterCallback, CallbackListHead;

	RtlInitUnicodeString(&unCallback, L"CmUnRegisterCallback");

	lpCmUnRegisterCallback = (ULONG_PTR)MmGetSystemRoutineAddress(&unCallback);
	if (!lpCmUnRegisterCallback)
		return status;

	if (g_DynData.dwOsBuildNumber < 7600)
		return status;


	//  xor     r8d, r8d
	//  lea     rdx, [rsp + 20h]
	//  lea     rcx, [nt!CallbackListHead]
	char Pattern[3] = { 0x45,0x33,0xc0 };
	CallbackListHead = FindPattern(lpCmUnRegisterCallback, 0x1FF, Pattern, sizeof(Pattern));
	if (!CallbackListHead)
		return status;

	char Pattern1[3] = { 0x48,0x8d,0x0d };
	CallbackListHead = FindPattern(CallbackListHead, 0x10, Pattern1, sizeof(Pattern1));
	if (!CallbackListHead)
		return status;
	
	//
	CallbackListHead = (ULONG_PTR)((PUCHAR)CallbackListHead + *((PLONG)((PUCHAR)CallbackListHead + 3)) + 7);
	if (CallbackListHead && MmIsAddressValid(CallbackListHead))
	{
		*ppCallbackListHead = CallbackListHead;
		status = STATUS_SUCCESS;
	}

	return status;
}


//枚举RegisterCallback
NTSTATUS EnumCmRegisterCallback()
{
	//定义变量
	NTSTATUS   status = STATUS_UNSUCCESSFUL; 
	PLIST_ENTRY			notifyList = NULL;
	PCM_CALLBACK_ENTRY	notify = NULL;
	ULONG_PTR    pCallbackListHead = NULL;

	status = FindCallbackListHead(&pCallbackListHead);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	
	notifyList = ((PLIST_ENTRY)pCallbackListHead)->Flink;

	do
	{
		notify = (PCM_CALLBACK_ENTRY)notifyList;
		if (MmIsAddressValid(notify) == FALSE)
			break;
		if (notify->Cookie.QuadPart <= 0)
			break;
		if (notify->Function <= MmSystemRangeStart)
			break;

		if (MmIsAddressValid((PVOID)(notify->Function)))
		{
			kprintf("%s Function is %p, cookie is %lld, attidute is %wZ \n", __FUNCTION__, (PVOID)(notify->Function), (PVOID)(notify->Cookie.QuadPart),
				&notify->Attidute);
		}

		notifyList = notifyList->Flink;
	} while (notifyList != ((LIST_ENTRY*)(pCallbackListHead)));

	return STATUS_SUCCESS;
}





