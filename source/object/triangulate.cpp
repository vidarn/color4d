/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// generator object example (with input objects)

#include "c4d.h"
#include "c4d_symbols.h"

class TriangulateData : public ObjectData
{
	private:
		LineObject *PrepareSingleSpline(BaseObject *generator, BaseObject *op, Matrix *ml, HierarchyHelp *hh, Bool *dirty);
		void Transform(PointObject *op, const Matrix &m);

	public:
		virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);

		static NodeData *Alloc(void) { return gNew TriangulateData; }
};

LineObject *TriangulateData::PrepareSingleSpline(BaseObject *generator, BaseObject *op, Matrix *ml, HierarchyHelp *hh, Bool *dirty)
{
	LineObject *lp  = (LineObject*)GetVirtualLineObject(op,hh,op->GetMl(),FALSE,FALSE,ml,dirty);
	if (!lp || lp->GetPointCount()<1 || !lp->GetLineR()) return NULL;
	lp->Touch(); 
	generator->AddDependence(hh,lp);
	return lp;
}

void TriangulateData::Transform(PointObject *op, const Matrix &m)
{
	Vector	*padr=op->GetPointW();
	LONG		pcnt=op->GetPointCount(),i;
	
	for (i=0; i<pcnt; i++)
		padr[i]*=m;
	
	op->Message(MSG_UPDATE);
}

BaseObject *TriangulateData::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	if (!op->GetDown()) return NULL;

	LineObject		*contour=NULL;	
	PolygonObject *pp=NULL;
	Bool					dirty=FALSE;
	Matrix				ml;

	op->NewDependenceList();
	contour=PrepareSingleSpline(op,op->GetDown(),&ml,hh,&dirty);
	if (!dirty) dirty = op->CheckCache(hh);					
	if (!dirty) dirty = op->IsDirty(DIRTYFLAGS_DATA);		
	if (!dirty) dirty = !op->CompareDependenceList();
	if (!dirty) return op->GetCache(hh);

	if (!contour) return NULL;

	pp = contour->Triangulate(0.0,hh->GetThread());

	if (!pp) return NULL;

	pp->SetPhong(TRUE,FALSE,0.0);
	Transform(pp,ml);
	pp->SetName(op->GetName());
	
	if (hh->GetBuildFlags()&BUILDFLAGS_ISOPARM)
	{
		pp->SetIsoparm((LineObject*)contour->GetClone(COPYFLAGS_NO_HIERARCHY,NULL));
		Transform(pp->GetIsoparm(),ml);
	}

	return pp;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_TRIANGULATEOBJECT 1001159

Bool RegisterTriangulate(void)
{
	return RegisterObjectPlugin(ID_TRIANGULATEOBJECT,GeLoadString(IDS_TRIANGULATE),OBJECT_GENERATOR|OBJECT_INPUT,TriangulateData::Alloc,"",AutoBitmap("triangulate.tif"),0);
}
