/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// this example demonstrates how to implement a generator
// with input objects. the first sub-object (including childs)
// of the atom is taken as polygonized input. The atom places
// a sphere at input points and cylinders at input edges.
// depending on the user options one single mesh or several
// single objects (hierarchically grouped) are built.

#include "c4d.h"
#include "c4d_symbols.h"
#include "oatom.h"

class AtomObject : public ObjectData
{
	public:
		virtual Bool Init(GeListNode *node);

		virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
		virtual Bool Message(GeListNode *node, LONG type, void *t_data);

		static NodeData *Alloc(void) { return gNew AtomObject; }
};

// initialize settings
Bool AtomObject::Init(GeListNode *node)
{
	BaseObject		*op		= (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetReal(ATOMOBJECT_SRAD,5.0);
	data->SetReal(ATOMOBJECT_CRAD,2.0);
	data->SetLong(ATOMOBJECT_SUB,8);
	data->SetBool(ATOMOBJECT_SINGLE,FALSE);

	return TRUE;
}

Bool AtomObject::Message(GeListNode *node, LONG type, void *t_data)
{
	if (type==MSG_DESCRIPTION_VALIDATE)
	{
		BaseContainer *data = ((BaseObject*)node)->GetDataInstance();
		CutReal(*data,ATOMOBJECT_CRAD,0.0,data->GetReal(ATOMOBJECT_SRAD));
	}
	return TRUE;
}

// build a rectangular matrix system with a given normal
static void RectangularSystem(const Vector &n, Vector *v1, Vector *v2)
{
	if (n.x>-0.6 && n.x<0.6) 
		*v2=Vector(1.0,0.0,0.0);
	else if (n.y>-0.6 && n.y<0.6)
		*v2=Vector(0.0,1.0,0.0);
	else
		*v2=Vector(0.0,0.0,1.0);

	*v1 = !((*v2)%n);
	*v2 = !(n%(*v1));
}

// build a single polygonal atom object
static PolygonObject *BuildPolyHull(PolygonObject *op, const Matrix &ml, Real srad, Real crad, LONG sub,
																		Real lod, Neighbor *n, BaseThread *bt)
{
	BaseContainer bc,cc;
	LONG     spcnt,svcnt,cpcnt,cvcnt,poff,voff,i,j,a=0,b=0,side;
	const Vector *spadr=NULL,*cpadr=NULL;
	Vector *rpadr=NULL;
	Vector off,pa,pb;
	const CPolygon *svadr=NULL,*cvadr=NULL;
	CPolygon *rvadr=NULL;
	UVWTag   *suvw =NULL,*cuvw =NULL,*ruvw =NULL;
	const Vector *padr = op->GetPointR();
	const CPolygon *vadr = op->GetPolygonR();
	LONG     pcnt  = op->GetPointCount();
	LONG		 vcnt  = op->GetPolygonCount();
	PolyInfo *pli  = NULL;
	Bool     ok=FALSE;
	Matrix   m;
	UVWHandle suvwptr,duvwptr;

	// set sphere default values
	bc.SetReal(PRIM_SPHERE_RAD,srad);
	bc.SetReal(PRIM_SPHERE_SUB,sub);

	// set cylinder default values (cylinders are a special case of cone objects)
	cc.SetReal(PRIM_CYLINDER_RADIUS,crad);
	cc.SetReal(PRIM_CYLINDER_HEIGHT,1.0);
	cc.SetLong(PRIM_CYLINDER_CAPS,FALSE);
	cc.SetLong(PRIM_CYLINDER_HSUB,1);
	cc.SetLong(PRIM_CYLINDER_SEG,sub);
	cc.SetReal(PRIM_AXIS,4);

	// generate both primitives
	PolygonObject *sphere=(PolygonObject*)GeneratePrimitive(NULL,Osphere,bc,lod,FALSE,bt),*pp=NULL;
	PolygonObject *cyl=(PolygonObject*)GeneratePrimitive(NULL,Ocylinder,cc,lod,FALSE,bt);
	if (!sphere || !cyl) goto Error;

	spcnt = sphere->GetPointCount();
	svcnt = sphere->GetPolygonCount();
	spadr = sphere->GetPointR();
	svadr = sphere->GetPolygonR();
	suvw  = (UVWTag*)sphere->GetTag(Tuvw);

	cpcnt = cyl->GetPointCount();
	cvcnt = cyl->GetPolygonCount();
	cpadr = cyl->GetPointR();
	cvadr = cyl->GetPolygonR();
	cuvw  = (UVWTag*)cyl->GetTag(Tuvw);

	// allocate main object
	pp=PolygonObject::Alloc(spcnt*pcnt+cpcnt*n->GetEdgeCount(),svcnt*pcnt+cvcnt*n->GetEdgeCount());
	if (!pp) goto Error;

	// add phong tag
	if (!pp->MakeTag(Tphong)) goto Error;

	// add UVW tag
	ruvw=(UVWTag*)pp->MakeVariableTag(Tuvw,pp->GetPolygonCount());
	if (!ruvw) goto Error;

	// copy sphere geometry for each point
	rpadr = pp->GetPointW();
	rvadr = pp->GetPolygonW();
	poff  = 0;
	voff  = 0;

	suvwptr = suvw->GetDataAddressR();
	duvwptr = ruvw->GetDataAddressW();
	for (i=0; i<pcnt; i++)
	{
		// test every 256th time if there has been a user break, delete object in this case
		if (!(i&255) && bt && bt->TestBreak()) goto Error;

		off=padr[i]*ml;
		for (j=0; j<spcnt; j++)
			rpadr[poff+j] = off + spadr[j];

		for (j=0; j<svcnt; j++)
		{
			rvadr[voff+j] = CPolygon(svadr[j].a+poff,svadr[j].b+poff,svadr[j].c+poff,svadr[j].d+poff);
			ruvw->Copy(duvwptr,voff+j,suvwptr,j);
		}

		poff+=spcnt;
		voff+=svcnt;
	}

	// copy cylinder geometry for each edge
	suvwptr = cuvw->GetDataAddressR();
	duvwptr = ruvw->GetDataAddressW();
	for (i=0; i<vcnt; i++)
	{
		pli = n->GetPolyInfo(i);

		// test every 256th time if there has been a user break, delete object in this case
		if (!(i&255) && bt && bt->TestBreak()) goto Error;

		for (side=0; side<4; side++)
		{
			// only proceed if edge has not already been processed
			// and edge really exists (for triangles side 2 from c..d does not exist as c==d)
			if (pli->mark[side] || side==2 && vadr[i].c==vadr[i].d) continue;

			switch (side)
			{
				case 0: a=vadr[i].a; b=vadr[i].b; break;
				case 1: a=vadr[i].b; b=vadr[i].c; break;
				case 2: a=vadr[i].c; b=vadr[i].d; break;
				case 3: a=vadr[i].d; b=vadr[i].a; break;
			}

			// build edge matrix
			pa = padr[a]*ml;
			pb = padr[b]*ml;

			m.off=(pa+pb)*0.5;
			RectangularSystem(!(pb-pa),&m.v1,&m.v2);
			m.v3=pb-pa;

			for (j=0; j<cpcnt; j++)
				rpadr[poff+j] = cpadr[j]*m;

			for (j=0; j<cvcnt; j++)
			{
				rvadr[voff+j] = CPolygon(cvadr[j].a+poff,cvadr[j].b+poff,cvadr[j].c+poff,cvadr[j].d+poff);
				ruvw->Copy(duvwptr,voff+j,suvwptr,j);
			}

			poff+=cpcnt;
			voff+=cvcnt;
		}
	}

	// update object as point geometry has changed
	pp->Message(MSG_UPDATE);

	ok=TRUE;
Error:
	blDelete(sphere);
	blDelete(cyl);

	if (!ok) blDelete(pp);
	return pp;
}

// build a single isoparm atom object
static LineObject *BuildIsoHull(PolygonObject *op, const Matrix &ml, Real srad, Real crad, LONG sub,
																Real lod, Neighbor *n, BaseThread *bt)
{
	LONG     poff,soff,i,j,a=0,b=0,side;
	Vector   *rpadr=NULL,off,pa,pb;
	Segment  *rsadr=NULL;
	const Vector *padr = op->GetPointR();
	const CPolygon *vadr = op->GetPolygonR();
	LONG		 vcnt  = op->GetPolygonCount();
	PolyInfo *pli  = NULL;
	Matrix   m;
	Vector   p[8];

	// allocate isoparm object
	LineObject *pp=LineObject::Alloc(8*n->GetEdgeCount(),4*n->GetEdgeCount());
	if (!pp) return NULL;

	rpadr = pp->GetPointW();
	rsadr = pp->GetSegmentW();
	poff  = 0;
	soff  = 0;

	p[0]=Vector(-crad,  0.0,-0.5);
	p[1]=Vector(-crad,  0.0, 0.5);
	p[2]=Vector( crad,  0.0,-0.5);
	p[3]=Vector( crad,  0.0, 0.5);
	p[4]=Vector(  0.0,-crad,-0.5);
	p[5]=Vector(  0.0,-crad, 0.5);
	p[6]=Vector(  0.0, crad,-0.5);
	p[7]=Vector(  0.0, crad, 0.5);

	for (i=0; i<vcnt; i++)
	{
		// test every 256th time if there has been a user break, delete object in this case
		if (!(i&255) && bt && bt->TestBreak())
		{
			blDelete(pp);
			return NULL;
		}

		pli = n->GetPolyInfo(i);

		for (side=0; side<4; side++)
		{
			// only proceed if edge has not already been processed
			// and edge really exists (for triangles side 2 from c..d does not exist as c==d)
			if (pli->mark[side] || side==2 && vadr[i].c==vadr[i].d) continue;

			switch (side)
			{
				case 0: a=vadr[i].a; b=vadr[i].b; break;
				case 1: a=vadr[i].b; b=vadr[i].c; break;
				case 2: a=vadr[i].c; b=vadr[i].d; break;
				case 3: a=vadr[i].d; b=vadr[i].a; break;
			}

			// build edge matrix
			pa = padr[a]*ml;
			pb = padr[b]*ml;

			m.off=(pa+pb)*0.5;
			RectangularSystem(!(pb-pa),&m.v1,&m.v2);
			m.v3=pb-pa;

			for (j=0; j<8; j++)
				rpadr[poff+j] = p[j]*m;

			for (j=0; j<4; j++)
			{
				rsadr[soff+j].closed = FALSE;
				rsadr[soff+j].cnt    = 2;
			}

			poff+=8;
			soff+=4;
		}
	}

	// update object as point geometry has changed
	pp->Message(MSG_UPDATE);

	return pp;
}

// go through every (child) object
static Bool Recurse(HierarchyHelp *hh, BaseThread *bt, BaseObject *main, BaseObject *op, const Matrix &ml, Real srad, Real crad, LONG sub, Bool single)
{
	// test if input object if polygonal
	if (op->GetType()==Opolygon)
	{
		BaseObject *tp   = NULL;
		PolyInfo	 *pli  = NULL;
		const Vector *padr = ToPoly(op)->GetPointR();
		Vector pa,pb;
		LONG       pcnt  = ToPoly(op)->GetPointCount(),i,side,a=0,b=0;
		const CPolygon *vadr = ToPoly(op)->GetPolygonR();
		LONG       vcnt  = ToPoly(op)->GetPolygonCount();
		Matrix     m;
		Neighbor	 n;

		// load names from resource
		String		 pstr = GeLoadString(IDS_ATOM_POINT);
		String		 estr = GeLoadString(IDS_ATOM_EDGE);

		// initialize neighbor class
		if (!n.Init(pcnt,vadr,vcnt,NULL)) return FALSE;

		// create separate objects
		// if this option is enabled no polygonal geometry is build - more parametric objects
		// are returned instead
		if (single)
		{
			for (i=0; i<pcnt; i++)
			{
				// alloc sphere primitive
				tp=BaseObject::Alloc(Osphere);
				if (!tp) return FALSE;

				// add phong tag
				if (!tp->MakeTag(Tphong)) return FALSE;
				tp->SetName(pstr+" "+LongToString(i));

				// set object parameters
				BaseContainer *bc = tp->GetDataInstance();
				bc->SetReal(PRIM_SPHERE_RAD,srad);
				bc->SetReal(PRIM_SPHERE_SUB,sub);

				// insert as last object under main
				tp->InsertUnderLast(main);

				// set position in local coordinates
				tp->SetRelPos(padr[i]*ml);
			}

			for (i=0; i<vcnt; i++)
			{
				// get polygon info for i-th polygon
				pli = n.GetPolyInfo(i);

				for (side=0; side<4; side++)
				{
					// only proceed if edge has not already been processed
					// and edge really exists (for triangles side 2 from c..d does not exist as c==d)
					if (pli->mark[side] || side==2 && vadr[i].c==vadr[i].d) continue;

					// alloc cylinder primitive
					tp=BaseObject::Alloc(Ocylinder);
					if (!tp) return FALSE;

					// add phong tag
					if (!tp->MakeTag(Tphong)) return FALSE;

					switch (side)
					{
						case 0: a=vadr[i].a; b=vadr[i].b; break;
						case 1: a=vadr[i].b; b=vadr[i].c; break;
						case 2: a=vadr[i].c; b=vadr[i].d; break;
						case 3: a=vadr[i].d; b=vadr[i].a; break;
					}

					tp->SetName(estr+" "+LongToString(pli->edge[side]));

					pa = padr[a]*ml;
					pb = padr[b]*ml;

					// set object parameters
					BaseContainer *bc = tp->GetDataInstance();
					bc->SetReal(PRIM_CYLINDER_RADIUS,crad);
					bc->SetReal(PRIM_CYLINDER_HEIGHT,Len(pb-pa));
					bc->SetReal(PRIM_AXIS,4);
					bc->SetLong(PRIM_CYLINDER_CAPS,FALSE);
					bc->SetLong(PRIM_CYLINDER_HSUB,1);
					bc->SetLong(PRIM_CYLINDER_SEG,sub);

					// place cylinder at edge center
					tp->SetRelPos((pa+pb)*0.5);

					// build edge matrix
					m.v3=!(pb-pa);
					RectangularSystem(m.v3,&m.v1,&m.v2);
					tp->SetRelRot(MatrixToHPB(m, tp->GetRotationOrder()));

					// insert as last object under main
					tp->InsertUnderLast(main);
				}
			}
		}
		else
		{
			// check if polygonal geometry has to be built
			tp = BuildPolyHull(ToPoly(op),ml,srad,crad,sub,hh->GetLOD(),&n,bt);

			if (tp)
			{
				tp->SetName(op->GetName());
				tp->InsertUnderLast(main);

				// check if isoparm geometry has to be built
				if (hh->GetBuildFlags()&BUILDFLAGS_ISOPARM)
				{
					LineObject *ip = BuildIsoHull(ToPoly(op),ml,srad,crad,sub,hh->GetLOD(),&n,bt);

					// isoparm always needs to be set into a polygon object
					if (ip) tp->SetIsoparm(ip);
				}
			}
		}
	}

	for (op=op->GetDown(); op; op=op->GetNext())
		if (!Recurse(hh,bt,main,op,ml*op->GetMl(),srad,crad,sub,single)) return FALSE;

	// check for user break
	return !bt || !bt->TestBreak();
}

// main routine: build virtual atom objects
BaseObject *AtomObject::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	BaseObject *orig = op->GetDown();

	// return if no input object is available
	if (!orig) return NULL;

	Bool dirty = FALSE;

	// generate polygonalized clone of input object
	BaseObject *main=NULL,*res=op->GetAndCheckHierarchyClone(hh,orig,HIERARCHYCLONEFLAGS_ASPOLY,&dirty,NULL,FALSE);

	// if !dirty object is already cached and doesn't need to be rebuilt
	if (!dirty) return res;
	if (!res)   return NULL;

	LONG sub;
	Bool single;
	Real srad,crad;

	// get object container
	BaseContainer *bc=op->GetDataInstance();
	BaseThread    *bt=hh->GetThread();

	// group all further objects with this null object
	main = BaseObject::Alloc(Onull);
	if (!main) goto Error;

	// get object settings
	srad   = bc->GetReal(ATOMOBJECT_SRAD);
	crad   = bc->GetReal(ATOMOBJECT_CRAD);
	sub    = bc->GetLong(ATOMOBJECT_SUB);
	single = bc->GetBool(ATOMOBJECT_SINGLE);

	// go through all child hierarchies
	if (!Recurse(hh,bt,main,res,orig->GetMl(),srad,crad,sub,single)) goto Error;
	blDelete(res);

	return main;

Error:
	blDelete(res);
	blDelete(main);
	return NULL;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_ATOMOBJECT 1001153

Bool RegisterAtomObject(void)
{
	return RegisterObjectPlugin(ID_ATOMOBJECT,GeLoadString(IDS_ATOM),OBJECT_GENERATOR|OBJECT_INPUT,AtomObject::Alloc,"Oatom",AutoBitmap("atom.tif"),0);
}
