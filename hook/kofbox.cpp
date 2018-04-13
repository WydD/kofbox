#include <cmath>
#include <cstdio>
#include "stdafx.h"
#include "hook_util.h"

static const unsigned int PHYBOX_COLOR = 0xFFA500;
static const unsigned int PLAYER1_HURTBOX_COLOR = 0x00FF00;
static const unsigned int PLAYER2_HURTBOX_COLOR = 0x0000FF;
static const unsigned int PROJECTILE_HURTBOX_COLOR = 0xFFFFFF;
static const unsigned int HITBOX_COLOR = 0xFF0000;
static const unsigned int PROXIMITYBOX_COLOR = 0xAAAAAA;
static const unsigned int AUTOGUARD_BORDER_COLOR = 0x00FFFF;
static const unsigned int COUNTER_BORDER_COLOR = 0xFF0000;
static const unsigned int FILL_MASK = 0x88000000;
static const unsigned int BORDER_MASK = 0xF8000000;

static const unsigned int pCameraControlOffset = 0x3A0D0;
/**
 * Here lies the injected functions
 */

void (*DrawKeyHistory)(void *obj, int playerID, void *history, bool bFrameDraw) = nullptr;

void (*EasyDraw)(void *obj, void *graphics) = nullptr;

void (*AttackHitCheck)(void *actionSystem) = nullptr;

void *(*GetRealHitRect)(void *obj, void *result) = nullptr;

void (*GraphicsFillRect)(void *graphics, int x, int y, int width, int height, unsigned int color, char bDraw) = nullptr;

void (*OnKeyInput)(void *obj, int key) = nullptr;

void (*AgTrace)(const char *text, ...) = nullptr;

void *HitRectCollectionInstance;

/**
 * Simple struct declarations
 */

struct Camera {
    float x;
    float y;
    float z;
    float height;
    float scale;
};

struct BoxPos {
    float x;
    float y;
    float w;
    float h;
};

/**
 * State variables
 */

Camera camera = {0.0, 0.0, 0.0, 1, 1.0};

bool displayHitboxes = true;

/**
 * IMPL PART
 */

void drawBox(void *g, int screenWidth, int screenHeight, BoxPos *box, unsigned int color, unsigned int borderColor) {
    float scaleRatio = (screenHeight / camera.height) * camera.scale * 5 / 6;
    auto x = (int) round((box->x - camera.x) * scaleRatio + screenWidth / 2.0);
    auto y = (int) round(screenHeight / 2.0 - (box->y - camera.y) * scaleRatio);
    auto w = (int) round(box->w * scaleRatio);
    auto h = (int) round(box->h * scaleRatio);
    GraphicsFillRect(g, x, y, w, h, color, 1);
    GraphicsFillRect(g, x, y, 2, h, borderColor, 1);
    GraphicsFillRect(g, x, y, w, 2, borderColor, 1);
    GraphicsFillRect(g, x + w - 2, y, 2, h, borderColor, 1);
    GraphicsFillRect(g, x, y + h - 2, w, 2, borderColor, 1);
}

void drawGroups(void *graphics, int screenWidth, int screenHeight, int from, int to, void *hitRectListBase, int player) {
    BoxPos realHitRect{};
    unsigned int color;
    unsigned int borderColor;
    for (int group = from; group < to; ++group) {
        void *rectList = readPointer(hitRectListBase, player * 120 + group * 8);
        void *entry = readPointer(rectList, 8);
        void *lastEntry = readPointer(rectList, 16);
        while (entry < lastEntry) {
            GetRealHitRect(entry, &realHitRect);
            if (group == 0) {
                color = PHYBOX_COLOR;
                borderColor = color;
            } else if (group >= 1 && group < 9) {
                color = player == 0 ? PLAYER1_HURTBOX_COLOR : PLAYER2_HURTBOX_COLOR;
                void* parent = readPointer(entry, 0x18);
                void* owner = readPointer(entry, 0x20);
                if (parent != owner) {
                    color = PROJECTILE_HURTBOX_COLOR;
                    borderColor = color;
                } else if (group == 5) {
                    // Counter
                    borderColor = COUNTER_BORDER_COLOR;
                } else if (group == 4) {
                    // Auto Guard
                    borderColor = AUTOGUARD_BORDER_COLOR;
                } else {
                    borderColor = color;
                }
            } else {
                if (readInt(entry, 0x30) < 5) {
                    color = HITBOX_COLOR;
                } else {
                    color = PROXIMITYBOX_COLOR;
                }
                borderColor = color;
            }
            drawBox(graphics, screenWidth, screenHeight, &realHitRect, color | FILL_MASK, borderColor | BORDER_MASK);
            entry = ptrOffset(entry, 0x1F8);
        }
    }
}

void drawHitboxes(void *graphics) {
    // Draw hitboxes here
    int screenWidth = readInt(graphics, 0x34);
    int screenHeight = readInt(graphics, 0x38);
    void *hitRectListBase = readPointer(HitRectCollectionInstance, 0);
    // first draw hurtboxes
    drawGroups(graphics, screenWidth, screenHeight, 1, 9, hitRectListBase, 0);
    drawGroups(graphics, screenWidth, screenHeight, 1, 9, hitRectListBase, 1);
    // then phyboxes
    drawGroups(graphics, screenWidth, screenHeight, 0, 1, hitRectListBase, 0);
    drawGroups(graphics, screenWidth, screenHeight, 0, 1, hitRectListBase, 1);
    // then hitboxes
    drawGroups(graphics, screenWidth, screenHeight, 9, 15, hitRectListBase, 0);
    drawGroups(graphics, screenWidth, screenHeight, 9, 15, hitRectListBase, 1);
}

/**
 * Hooked functions
 */

void HookedOnKeyInput(void *obj, int key) {
    if (key == 0x74) {
        displayHitboxes = !displayHitboxes;
    }
    return OnKeyInput(obj, key);
}

void HookedEasyDraw(void *obj, void *graphics) {
    EasyDraw(obj, graphics);
    if (displayHitboxes) {
        drawHitboxes(graphics);
    }
}

void HookedDrawKeyHistory(void *obj, int playerID, void *history, bool bFrameDraw) {
    return DrawKeyHistory(obj, playerID, history, true);
}

void HookedAttackHitCheck(void *actionSystem) {
    AttackHitCheck(actionSystem);
    // gather camera data
    camera.x = readFloat(actionSystem, pCameraControlOffset + 0x1FC);
    camera.y = readFloat(actionSystem, pCameraControlOffset + 0x1FC + 4);
    camera.z = readFloat(actionSystem, pCameraControlOffset + 0x1FC + 8);
    camera.height = readFloat(actionSystem, pCameraControlOffset + 0x60) * 2;
    camera.scale = readFloat(actionSystem, pCameraControlOffset + 0x5C) / camera.z;
}


// LOG That exploits the KOF debug.log
void koflog(const char* text, ...) {
    va_list args;
    va_start(args, text);
    if (AgTrace) {
        AgTrace(text, args);
    } else {
        printf(text, args);
    }
    va_end(args);
    return;
}

/**
 * BOOTSTRAP PART
 */
// EasyHook DLL injection entry point
extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO *inRemoteInfo);
BOOL hasBeenInjected = FALSE;
void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO *inRemoteInfo) {
    if (hasBeenInjected) {
        return;
    }
    hasBeenInjected = TRUE;
    // Dark magic!
    BYTE agTracePattern[] = {
            0x48, 0x89, 0x4C, 0x24, 0x08, 0x48, 0x89, 0x54, 0x24, 0x10, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x4C, 0x89, 0x4C,
            0x24, 0x20, 0x48, 0x83, 0xEC, 0x38, 0x48, 0xC7, 0x44, 0x24, 0x20, 0xFE, 0xFF, 0xFF, 0xFF
    };
    BYTE drawKeyHistoryPattern[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83,
            0xEC, 0x30, 0x48, 0x8B, 0x41, 0x08, 0x48, 0x8B, 0xD9, 0x48, 0x83, 0xC1, 0x08, 0x41, 0x0F, 0xB6, 0xF9, 0x49,
            0x8B, 0xE8, 0x8B, 0xF2, 0xFF, 0x10, 0x84, 0xC0, 0x75, 0x3D, 0x38, 0x43
    };
    BYTE getRealHitRectPattern[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x0F, 0x10, 0x41, 0x04, 0x48, 0x8B, 0xF9, 0x48,
            0x8B, 0xDA, 0x48, 0x8B, 0x49, 0x20, 0x0F, 0x11, 0x02, 0x83, 0xB9, 0x8C, 0x01, 0x00
    };
    BYTE easyDrawPattern[] = {
            0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B, 0xF9,
            0x48, 0x8B, 0xEA, 0x48, 0x8B, 0xCA
    };
    BYTE fillRectPattern[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83,
            0xEC, 0x30, 0x48, 0x8B, 0xE9, 0x41, 0x8B, 0xD9, 0x48, 0x8B, 0x49, 0x08, 0x41, 0x8B
    };
    BYTE playerAttackCheckPattern[] = {
            0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x48, 0x48,
            0xC7, 0x44, 0x24, 0x30, 0xFE, 0xFF, 0xFF, 0xFF, 0x33, 0xC9, 0x33, 0xC0, 0x33, 0xFF, 0x48, 0x89, 0xBC, 0x24,
            0x98, 0x00, 0x00, 0x00, 0x48, 0x8D, 0, 0, 0, 0, 0, 0x90, 0xFF, 0xC1, 0x89, 0x8C, 0x24, 0x90, 0x00, 0x00
    };
    BYTE attackHitCheckPattern[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x18, 0x56, 0x57, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x20,
            0x48, 0x8B, 0, 0, 0, 0, 0x00, 0x48, 0x8B
    };
    BYTE onKeyInputPattern[] = {
            0x40, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0xC7, 0x44, 0x24, 0x20, 0xFE, 0xFF, 0xFF, 0xFF, 0x48, 0x89, 0x5C,
            0x24, 0x78, 0x48, 0x89, 0xB4, 0x24, 0x88, 0x00, 0x00, 0x00, 0x48, 0x63, 0xDA, 0x48
    };

    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(TEXT("kofxiv.exe")), &moduleInfo, sizeof(moduleInfo));

    AgTrace = (void (*)(const char *, ...)) findPointer(agTracePattern, sizeof(agTracePattern), &moduleInfo, -1, -1);
    if (!AgTrace) {
        return;
    }
    DrawKeyHistory = (void (*)(void *, int, void *, bool)) findPointer(drawKeyHistoryPattern, sizeof(drawKeyHistoryPattern), &moduleInfo, -1, -1);
    GetRealHitRect = (void *(*)(void *, void *)) findPointer(getRealHitRectPattern, sizeof(getRealHitRectPattern), &moduleInfo, -1, -1);
    EasyDraw = (void (*)(void *, void *)) findPointer(easyDrawPattern, sizeof(easyDrawPattern), &moduleInfo, -1, -1);
    GraphicsFillRect = (void (*)(void *, int, int, int, int, unsigned int, char)) findPointer(fillRectPattern, sizeof(fillRectPattern),&moduleInfo, -1, -1);
    // moar dark magic
    const BYTE *playerAttachCheck = findPointer(playerAttackCheckPattern, sizeof(playerAttackCheckPattern), &moduleInfo, 42, 47);
    AttackHitCheck = (void (*)(void *)) findPointer(attackHitCheckPattern, sizeof(attackHitCheckPattern), &moduleInfo, 20, 24);
    OnKeyInput = (void (*)(void *, int)) findPointer(onKeyInputPattern, sizeof(onKeyInputPattern), &moduleInfo, -1, -1);

    if (!DrawKeyHistory || !GetRealHitRect || !EasyDraw || !GraphicsFillRect || !playerAttachCheck || !AttackHitCheck ||
        !OnKeyInput) {
        koflog("Something didnt work, stopping injection");
        // make sure everything is installed
        return;
    }
    HitRectCollectionInstance = (void *) (playerAttachCheck + 90 + *(unsigned int *) (playerAttachCheck + 85));

    if (!installHook(DrawKeyHistory, (void *) HookedDrawKeyHistory)) {
        //AgTrace("Impossible to install hook for DrawKeyHistory");
        return;
    };
    if (!installHook(EasyDraw, (void *) HookedEasyDraw)) {
        koflog("Impossible to install hook for EasyDraw");
        return;
    };
    if (!installHook(AttackHitCheck, (void *) HookedAttackHitCheck)) {
        koflog("Impossible to install hook for AttackHitCheck");
        return;
    };
    if (!installHook(OnKeyInput, (void *) HookedOnKeyInput)) {
        koflog("Impossible to install hook for OnKeyInput");
        return;
    };
    koflog("Ready to go!");
}