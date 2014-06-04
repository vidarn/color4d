/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for creating a styling tag for Hair

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"

#include "thairsdkstyling.h"

//////////////////////////////////////////////////////////////////////////

class HairStylingTag : public TagData
{
	INSTANCEOF(HairStylingTag,TagData)

public:

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);

	virtual Bool Message(GeListNode *node, LONG type, void *data);

	static NodeData *Alloc(void) { return gNew HairStylingTag; }

	//////////////////////////////////////////////////////////////////////////
	
	HairPluginObjectData m_FnTable;
};

//////////////////////////////////////////////////////////////////////////

Bool _StyleFn(BaseDocument *doc, BaseList2D *op, HairObject *hair, HairGuides *guides, Vector *padr, LONG cnt, LONG scnt)
{
	LONG i,l;
	Matrix axis(DC);
	BaseContainer *bc=op->GetDataInstance();

	SplineData *dsplinex=(SplineData*)bc->GetCustomDataType(HAIR_STYLING_SPLINE_X,CUSTOMDATATYPE_SPLINE);
	SplineData *dspliney=(SplineData*)bc->GetCustomDataType(HAIR_STYLING_SPLINE_Y,CUSTOMDATATYPE_SPLINE);
	Real displace=bc->GetReal(HAIR_STYLING_DISPLACE);

	for (i=0;i<cnt;i++)
	{
		guides->GetRootAxis(i,axis,TRUE,TRUE,TRUE,TRUE);	// NOTE: during styling the points are in local space and their initial state

		for (l=1;l<scnt;l++)
		{
			Real dx=0.0,dy=0.0;

			if (dsplinex) dx=dsplinex->GetPoint(Real(l)/Real(scnt-1)).y;
			if (dspliney) dy=dspliney->GetPoint(Real(l)/Real(scnt-1)).y;

			dx*=displace; dy*=displace;

			padr[i*scnt+l]+=dx*axis.v1+dy*axis.v2;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

Bool HairStylingTag::Init(GeListNode *node)
{
	BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();
	
	bc->SetReal(HAIR_STYLING_DISPLACE,10.0);

	GeData d(CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);

	SplineData* p = (SplineData*)d.GetCustomDataType(CUSTOMDATATYPE_SPLINE);
	if (p)
	{
		p->MakeLinearSplineBezier(2);
	}
	bc->SetData(HAIR_STYLING_SPLINE_X, d);
	bc->SetData(HAIR_STYLING_SPLINE_Y, d);

	m_FnTable.calc_style=_StyleFn;

	return TRUE;
}

void HairStylingTag::Free(GeListNode *node)
{
}

Bool HairStylingTag::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData *mdata=(HairPluginMessageData*)data;
		mdata->data=&m_FnTable;
		return TRUE;
	}
	
	return SUPER::Message(node,type,data);
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_STYLING_EXAMPLE 1018980

Bool RegisterStylingTag()
{
	return RegisterTagPlugin(ID_HAIR_STYLING_EXAMPLE,GeLoadString(IDS_HAIR_STYLING_EXAMPLE),TAG_MULTIPLE|TAG_VISIBLE,HairStylingTag::Alloc,"Thairsdkstyling",AutoBitmap("hairstyling.tif"),0);
}
