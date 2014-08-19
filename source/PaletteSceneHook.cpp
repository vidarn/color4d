#include "paletteSceneHook.h"
#include "main.h"
#include "palette.h"

// register the SceneHook, check out the PLUGINFLAG_SCENEHOOK_SUPPORT_DOCUMENT_DESCRIPTION flag, this is required so that we can add a description to the SceneHook
Bool Register_PaletteSceneHook(void)
{
    return RegisterSceneHookPlugin(PALETTE_SCENE_HOOK_ID,String("Palette Scene Hook"),
                                   PLUGINFLAG_SCENEHOOK_SUPPORT_DOCUMENT_DESCRIPTION, PaletteSceneHook::Alloc, EXECUTIONFLAGS::EXECUTIONFLAGS_EXPRESSION, 0);
}

NodeData* PaletteSceneHook::Alloc(void)
{
    return NewObjClear(PaletteSceneHook);
}

Bool PaletteSceneHook::Init(GeListNode* node)
{
    return true;
}

Bool PaletteSceneHook::Message(GeListNode* node, Int32 type, void* d)
{
    DocumentInfoData *data = (DocumentInfoData *)d;
    
    if (type == MSG_DOCUMENTINFO)
    {
        switch (data->type) {
            case MSG_DOCUMENTINFO_TYPE_SETACTIVE:
            case MSG_DOCUMENTINFO_TYPE_LOAD:
                Palette::InitPalettes();
                Palette::UpdateAll();
                break;
        }
    }
    return true;
}