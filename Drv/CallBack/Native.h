#pragma once

#ifndef NATIVE_H
#define NATIVE_H

#pragma warning( disable : 4214 )
#pragma warning( disable : 4201 )



typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

NTSTATUS NTAPI ZwQuerySystemInformation(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef struct _CALLBACK_ENTRY {
	UINT16 Version; // 0x0
	UINT16 OperationRegistrationCount; // 0x2
	UINT32 unk1; // 0x4
	PVOID RegistrationContext; // 0x8
	UNICODE_STRING Altitude; // 0x10
} CALLBACK_ENTRY, *PCALLBACK_ENTRY;

typedef struct _OBJECT_CALLBACK_ENTRY {
	LIST_ENTRY CallbackList;
	OB_OPERATION Operations;
	ULONG Active;
	/*OB_HANDLE*/ PCALLBACK_ENTRY CallbackEntry;
	POBJECT_TYPE ObjectType;
	POB_PRE_OPERATION_CALLBACK  PreOperation;
	POB_POST_OPERATION_CALLBACK PostOperation;
} OBJECT_CALLBACK_ENTRY, *POBJECT_CALLBACK_ENTRY;

#if defined (_WIN64)
#define MAX_FAST_REFS 15
#else
#define MAX_FAST_REFS 7
#endif


#if 1
typedef struct _EX_FAST_REF
{
	union
	{
		PVOID Object;
#if defined (_WIN64)
		ULONG_PTR RefCnt : 4;
#else
		ULONG_PTR RefCnt : 3;
#endif
		ULONG_PTR Value;
	};
} EX_FAST_REF, *PEX_FAST_REF;
#endif


typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
	EX_RUNDOWN_REF        RundownProtect;
	PEX_CALLBACK_FUNCTION Function;
	PVOID                 Context;
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;

#ifndef EX_PUSH_LOCK
#define EX_PUSH_LOCK ULONG_PTR
#define PEX_PUSH_LOCK PULONG_PTR
#endif // !EX_PUSH_LOCK

typedef VOID(FASTCALL* PFN_ExfReleasePushLock)(__inout PEX_PUSH_LOCK PushLock);
typedef VOID(FASTCALL* PFN_ExReleasePushLockEx)(__inout PEX_PUSH_LOCK PushLock, __in ULONG Flags);

typedef VOID(FASTCALL* PFN_ExfAcquirePushLockShared)(__inout PEX_PUSH_LOCK PushLock);
typedef VOID(FASTCALL* PFN_ExAcquirePushLockSharedEx)(__inout PEX_PUSH_LOCK PushLock, __in ULONG Flags);

typedef VOID(FASTCALL* PFN_ExfAcquirePushLockExclusive)(__inout PEX_PUSH_LOCK PushLock);
typedef VOID(FASTCALL* PFN_ExAcquirePushLockExclusiveEx)(__inout PEX_PUSH_LOCK PushLock, __in ULONG Flags);

VOID FORCEINLINE ExReleasePushLock(IN PEX_PUSH_LOCK PushLock);

VOID FORCEINLINE ExAcquirePushLockShared(IN PEX_PUSH_LOCK PushLock);

VOID FORCEINLINE ExAcquirePushLockExclusive(IN PEX_PUSH_LOCK PushLock);


typedef struct _CM_CALLBACK_CONTEXT_BLOCK {
	LARGE_INTEGER               Cookie;             // to identify a specific callback for deregistration purposes
	LIST_ENTRY                  ThreadListHead;     // Active threads inside this callback
	EX_PUSH_LOCK                ThreadListLock;     // synchronize access to the above
	PVOID                       CallerContext;
} CM_CALLBACK_CONTEXT_BLOCK, *PCM_CALLBACK_CONTEXT_BLOCK;

//
// CmCallbackBlock on Vista and later OS.
//
typedef struct _CM_CALLBACK_BLOCK {
	LIST_ENTRY Entry;
	ULONG Unknown1;
	ULONG Unknown2;
	LARGE_INTEGER Cookie;
	PVOID Context;
	PEX_CALLBACK_FUNCTION Function;
} CM_CALLBACK_BLOCK, *PCM_CALLBACK_BLOCK;

//typedef struct _KSERVICE_TABLE_DESCRIPTOR {
//    PULONG_PTR Base;
//    PULONG Count;
//    ULONG Limit;
//    PUCHAR Number;
//} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;


// begin_ntosp

//
// Push lock definitions
//
typedef struct _EX_PUSH_LOCK_V
{

	//
	// LOCK bit is set for both exclusive and shared acquires
	//
#define EX_PUSH_LOCK_LOCK_V          ((ULONG_PTR)0x0)
#define EX_PUSH_LOCK_LOCK            ((ULONG_PTR)0x1)

	//
	// Waiting bit designates that the pointer has chained waiters
	//

#define EX_PUSH_LOCK_WAITING         ((ULONG_PTR)0x2)

	//
	// Waking bit designates that we are either traversing the list
	// to wake threads or optimizing the list
	//

#define EX_PUSH_LOCK_WAKING          ((ULONG_PTR)0x4)

	//
	// Set if the lock is held shared by multiple owners and there are waiters
	//

#define EX_PUSH_LOCK_MULTIPLE_SHARED ((ULONG_PTR)0x8)

	//
	// Total shared Acquires are incremented using this
	//
#define EX_PUSH_LOCK_SHARE_INC       ((ULONG_PTR)0x10)
#define EX_PUSH_LOCK_PTR_BITS        ((ULONG_PTR)0xf)

	union
	{
		struct
		{
			ULONG_PTR Locked : 1;
			ULONG_PTR Waiting : 1;
			ULONG_PTR Waking : 1;
			ULONG_PTR MultipleShared : 1;
			ULONG_PTR Shared : sizeof(ULONG_PTR) * 8 - 4;
		};
		ULONG_PTR Value;
		PVOID Ptr;
	};
} EX_PUSH_LOCK_V, *PEX_PUSH_LOCK_V;





#endif // !NATIVE_H
