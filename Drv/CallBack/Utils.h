#pragma once

#ifndef UTILS_H
#define UTILS_H


#define DELAY_ONE_MICROSECOND 	(-10)
#define DELAY_ONE_MILLISECOND	(DELAY_ONE_MICROSECOND*1000)
/*
	Traversing the DriverObjects linked list for the specified driver.
*/
BOOLEAN GetDriverInformation(
	_In_ wchar_t* DriverName,
	_In_ PDRIVER_OBJECT pDriver,
	_Out_ UINT64* ModuleBase,
	_Out_ UINT64* ModuleSize
);

UINT64 FindPattern(
	_In_ UINT64 Base,
	_In_ UINT64 Size,
	_In_ char* Pattern,
	_In_ size_t PatternSize
);

BOOLEAN InitDynamicData();

VOID MyCreateThread(PVOID lp);

VOID KSleep(ULONG ulMilliseconds);

#endif // !UTILS_H
