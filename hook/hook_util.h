//
// Created by WydD on 11/06/2017.
//

#ifndef KOFBOX_HOOK_UTIL_H
#define KOFBOX_HOOK_UTIL_H

#include <easyhook.h>
#include <Psapi.h>

const BYTE* findPointer(const BYTE *drawKeyHistoryPattern, int length, const MODULEINFO *moduleInfo, int ignoreFrom, int ignoreTo);

bool installHook(void *toHook, void *hookFunction);

inline void *ptrOffset(void *ptr, int offset) {
    return (void *) (((BYTE *) ptr) + offset);
}

inline float readFloat(void *ptr, int offset) {
    return *(float *) ptrOffset(ptr, offset);
}

inline int readInt(void *ptr, int offset) {
    return *(int *) ptrOffset(ptr, offset);
}

inline void *readPointer(void *ptr, int offset) {
    return *(void **) ptrOffset(ptr, offset);
}

#endif //KOFBOX_HOOK_UTIL_H
