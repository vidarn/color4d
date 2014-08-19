#pragma once
#include "c4d.h"

class PaletteSceneHook : public SceneHookData
{
public:
    static NodeData* Alloc(void);
    virtual Bool Init(GeListNode* node);
    virtual Bool Message(GeListNode* node, Int32 type, void* data);
};

Bool Register_PaletteSceneHook(void);