#include "c4d.h"
#include "lib_modeling.h"
#include "c4d_symbols.h"

#define ID_MODELING_REVERSE_NORMALS_SDK	450000026 

Bool ReverseNormals(Modeling* krnl, C4DAtom* op, UCHAR* reverse, LONG polycnt, LONG pgoncnt, LONG* polymap)
{
	LONG i;
	for (i = 0; i < polycnt; i++)
	{
		if (!reverse[polymap[i]])
			continue;
		reverse[polymap[i]] = 0;
		LONG lIndex = polymap[i] < pgoncnt ? polymap[i] + polycnt : i;//krnl->TranslateNgonIndex(op, polymap[i] < pgoncnt ? polymap[i] + polycnt : i);
		if (!krnl->FlipNgonNormal(op, lIndex))
		{
			GeAssert(FALSE);
			return FALSE;
		}
	}
	if (!krnl->Commit(op))
		return FALSE;
	op->Message(MSG_UPDATE);
	return TRUE;
}

Bool AddUndo(BaseDocument* doc, AtomArray* arr, UNDOTYPE type)
{
	LONG a;
	for (a = 0; a < arr->GetCount(); a++)
	{
		if (!doc->AddUndo(type, arr->GetIndex(a)))
			return FALSE;
	}
	return TRUE;
}

Bool ModelingReverseNormals(ModelingCommandData *data)
{
	if (!data->arr) return FALSE;
	data->arr->FilterObject(-1, Opolygon);
	if (data->arr->GetCount() < 1) return TRUE;

	Bool bOK = FALSE;

	LONG i, c;
	PolygonObject* pPolyObj;
	BaseSelect* pPolySel;
	AutoAlloc <Modeling> krnl;
	if (!krnl)
		return FALSE;

	if (data->doc && (data->flags & MODELINGCOMMANDFLAGS_CREATEUNDO))
		AddUndo(data->doc, data->arr, UNDOTYPE_CHANGE);

	Bool bAll = TRUE;
	UCHAR* reverse = NULL;
	LONG lNgonCount, *polymap = NULL;
	if (data->mode == MODELINGCOMMANDMODE_POLYGONSELECTION)
	{
		for (c = 0; c < data->arr->GetCount(); c++)
		{
			pPolyObj = ToPoly((BaseObject*)(data->arr->GetIndex(c)));
			if (pPolyObj->GetPolygonS()->GetCount() > 0)
			{
				bAll = FALSE;
				break;
			}
		}
	}
	for (c = 0; c < data->arr->GetCount(); c++)
	{
		pPolyObj = ToPoly((BaseObject*)(data->arr->GetIndex(c)));
		GeFree(polymap);
		GeFree(reverse);
		LONG polycnt = pPolyObj->GetPolygonCount();
		if (!pPolyObj->GetPolygonTranslationMap(lNgonCount, polymap))
			goto Error;
		reverse = GeAllocType( UCHAR, lNgonCount );
		if (!reverse)
			goto Error;
		if (bAll)
			FillMemType( UCHAR, reverse, lNgonCount, 1 );
		else
		{
			pPolySel = pPolyObj->GetPolygonS();
			for (i = 0; i < polycnt; i++)
				reverse[polymap[i]] = reverse[polymap[i]] || pPolySel->IsSelected(i);
		}
		if (!reverse)
			goto Error;
		if (!krnl->InitObject(pPolyObj))
			goto Error;
		if (!ReverseNormals(krnl, pPolyObj, reverse, polycnt, pPolyObj->GetNgonCount(), polymap))
			goto Error;
	}
	GeFree(reverse);
	GeFree(polymap);
	return TRUE;

Error:
	GeFree(reverse);
	GeFree(polymap);
	if (!bOK)
	{
		if (data->doc && (data->flags & MODELINGCOMMANDFLAGS_CREATEUNDO))
		{
		  data->doc->DoUndo(TRUE);
		}
	}
	return FALSE;
}

class ReverseNormalsCommand : public CommandData
{
public:
	LONG GetState(BaseDocument *doc)
	{
		AutoAlloc <AtomArray> arr;
		if (!doc || !arr)
			return 0;
		if (doc->GetMode()!=Mpolygons) return 0;
		doc->GetActivePolygonObjects(*arr, TRUE);
		if (arr->GetCount() == 0) return 0;
		return CMD_ENABLED;
	}

	Bool Execute(BaseDocument *doc)
	{
		AutoAlloc <AtomArray> arr;
		if (!doc || !arr)
			return 0;
		doc->GetActivePolygonObjects(*arr, TRUE);

		ModelingCommandData d;
		d.arr = arr;
		d.bc = NULL;
		d.doc = doc;
		d.flags = MODELINGCOMMANDFLAGS_CREATEUNDO;
		d.mode = MODELINGCOMMANDMODE_POLYGONSELECTION;

		return ModelingReverseNormals(&d);
	}
};

Bool RegisterReverseNormals()
{
	return RegisterCommandPlugin(ID_MODELING_REVERSE_NORMALS_SDK, GeLoadString(IDS_REVERSE_NORMALS_SDK), 0, NULL, String(), gNew ReverseNormalsCommand);
}
