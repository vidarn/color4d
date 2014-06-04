/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// "look at editor camera" expression example

#include "c4d.h"
#include "c4d_symbols.h"
#include "tlookatcameraexp.h"

#include "lib_description.h"
#include "customgui_priority.h"


class LookAtCamera : public TagData
{
	public:
		virtual Bool Init(GeListNode *node);

		virtual EXECUTIONRESULT Execute(BaseTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, LONG priority, EXECUTIONFLAGS flags);
		virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);

		static NodeData *Alloc(void) { return gNew LookAtCamera; }
};

Bool LookAtCamera::Init(GeListNode *node)
{
	BaseTag				*tag  = (BaseTag*)node;
	BaseContainer *data = tag->GetDataInstance();

	data->SetBool(LOOKATCAMERAEXP_PITCH,TRUE);
	GeData d;
	if (node->GetParameter(DescLevel(EXPRESSION_PRIORITY),d,DESCFLAGS_GET_0))
	{
		PriorityData *pd = (PriorityData*)d.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);
		if (pd) pd->SetPriorityValue(PRIORITYVALUE_CAMERADEPENDENT,GeData(TRUE));
		node->SetParameter(DescLevel(EXPRESSION_PRIORITY),d,DESCFLAGS_SET_0);
	}

	return TRUE;
}


Bool LookAtCamera::GetDDescription(GeListNode *node, Description *description,DESCFLAGS_DESC &flags)
{
	if (!description->LoadDescription(1001165)) return FALSE;
	const DescID *singleid = description->GetSingleDescID();

	DescID cid = DescLevel(6001, DTYPE_GROUP, 0);
	if (!singleid || cid.IsPartOf(*singleid,NULL)) // important to check for speedup c4d!
	{
		BaseContainer maingroup = GetCustomDataTypeDefault(DTYPE_GROUP);
		maingroup.SetString(DESC_NAME, "Main Group");
		if (!description->SetParameter(cid, maingroup, DescLevel(0))) return TRUE;
	}
	
	cid = DescLevel(6002, DTYPE_GROUP, 0);
	if (!singleid || cid.IsPartOf(*singleid,NULL)) // important to check for speedup c4d!
	{
		BaseContainer subgroup = GetCustomDataTypeDefault(DTYPE_GROUP);
		subgroup.SetString(DESC_NAME, "Sub Group");
		if	(!description->SetParameter(cid, subgroup, DescLevel(6001))) return TRUE;
	}
	
	cid = DescLevel(6003, DTYPE_BOOL, 0);
	if (!singleid || cid.IsPartOf(*singleid,NULL)) // important to check for speedup c4d!
	{
		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
		locked.SetString(DESC_NAME, "SPECIAL Locked");
		locked.SetBool(DESC_DEFAULT, TRUE);
		if (!description->SetParameter(cid, locked, DescLevel(6002))) return TRUE;
	}

	cid = DescLevel(6004, DTYPE_BOOL, 0);
	if (!singleid || cid.IsPartOf(*singleid,NULL)) // important to check for speedup c4d!
	{
		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
		locked = GetCustomDataTypeDefault(DTYPE_LONG);
		locked.SetString(DESC_NAME, "SPECIAL Long");
		locked.SetBool(DESC_DEFAULT, TRUE);
		if (!description->SetParameter(cid, locked, DescLevel(6002))) return TRUE;
	}

	flags |= DESCFLAGS_DESC_LOADED;

	return TRUE;
}


EXECUTIONRESULT LookAtCamera::Execute(BaseTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, LONG priority, EXECUTIONFLAGS flags)
{
	BaseDraw   *bd = doc->GetRenderBaseDraw(); if (!bd) return EXECUTIONRESULT_OK;
	BaseObject *cp = bd->GetSceneCamera(doc); 
	if (!cp) cp=bd->GetEditorCamera();
	if (!cp) return EXECUTIONRESULT_OK;

	Vector local = cp->GetMg().off * (!(op->GetUpMg() * op->GetFrozenMln())) - op->GetRelPos();
	Vector hpb   = VectorToHPB(local);

	if (!tag->GetData().GetBool(LOOKATCAMERAEXP_PITCH))
		hpb.y = op->GetRelRot().y;

	hpb.z = op->GetRelRot().z;

	op->SetRelRot(hpb);

	return EXECUTIONRESULT_OK;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_LOOKATCAMERATAG	1001165

Bool RegisterLookAtCamera(void)
{
	return RegisterTagPlugin(ID_LOOKATCAMERATAG,GeLoadString(IDS_LOOKATCAMERA),TAG_EXPRESSION|TAG_VISIBLE,LookAtCamera::Alloc,"Tlookatcameraexp",AutoBitmap("lookatcamera.tif"),0);
}
