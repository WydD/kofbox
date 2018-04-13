//
// Created by WydD on 11/06/2017.
//
#include "hook_util.h"
#include "kofbox.h"


const BYTE *findPointer(const BYTE *drawKeyHistoryPattern, int length, const MODULEINFO *moduleInfo, int ignoreFrom, int ignoreTo) {
    const BYTE *end = (BYTE *) (moduleInfo->lpBaseOfDll) + moduleInfo->SizeOfImage;
    BYTE * result = nullptr;
    for (auto *current = (BYTE *) moduleInfo->lpBaseOfDll; current < end; ++current) {
        if (*current != drawKeyHistoryPattern[0]) {
            continue;
        }
        BOOL found = true;
        for (int i = 1; i < length; ++i) {
            if (i >= ignoreFrom && i < ignoreTo) {
                continue;
            }
            if (current[i] != drawKeyHistoryPattern[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            if (result == nullptr) {
                koflog("Found pointer!");
            } else {
                koflog("Found duplicate!!! Abort the mission!");
                return nullptr;
            }
            result = current;
        }
    }
    if (result == nullptr) {
        koflog("Couldnt find pointer!");
    }
    return result;
}

bool installHook(void *toHook, void *hookFunction) {
    // Perform hooking
    HOOK_TRACE_INFO hHook = {nullptr}; // keep track of our hook

    // Install the hook
    NTSTATUS nt = LhInstallHook(
            toHook,
            hookFunction,
            nullptr,
            &hHook);

    if (nt != 0) {
        PWCHAR err = RtlGetLastErrorString();
        koflog("RhInjectLibrary failed with error code = %d %ls",(int) nt, err);
    } else {
        koflog("Library injected successfully.");
    }

    // If the threadId in the ACL is set to 0,
    // then internally EasyHook uses GetCurrentThreadId()
    ULONG ACLEntries[1] = {0};

    // Disable the hook for the provided threadIds, enable for all others
    LhSetExclusiveACL(ACLEntries, 1, &hHook);
    return nt == 0;
}
