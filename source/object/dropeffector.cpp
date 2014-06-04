/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2006 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// this example demonstrates how to implement a more complex direct control effector
// and utilize falloff and strength directly
// the effector drops the particles to a surface

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_collider.h"
#include "c4d_baseeffectorplugin.h"
#include "c4d_falloffplugin.h"
#include "oedrop.h"

struct DropEffectorData
{
	BaseObject *target;
	LONG mode;
	Real maxdist;
	Matrix genmg, igenmg;
	Matrix targmg, itargmg;
};

class DropEffector : public EffectorData
{
public:
		DropEffectorData				ed;
		AutoAlloc<GeRayCollider>rcol;
		GeRayColResult					rcolres;

		virtual Bool						InitEffector(GeListNode *node);

		virtual void						InitPoints(BaseObject *op,BaseObject *gen,BaseDocument *doc,EffectorDataStruct *data,MoData *md,BaseThread *thread);
		virtual void						ModifyPoints(BaseObject *op,BaseObject *gen,BaseDocument *doc,EffectorDataStruct *data,MoData *md,BaseThread *thread);

		static NodeData* Alloc(void) { return gNew DropEffector; }
};

Bool DropEffector::InitEffector(GeListNode *node)
{
	if (!rcol || !node) return FALSE;

	BaseObject *op = ( BaseObject* )node;
	if (!op) return FALSE;

	BaseContainer *bc = op->GetDataInstance();
	if (!bc) return FALSE;

	bc->SetReal( DROPEFFECTOR_DISTANCE, 1000.0 );

	return TRUE;
}

void DropEffector::InitPoints(BaseObject *op,BaseObject *gen,BaseDocument *doc,EffectorDataStruct *data,MoData *md,BaseThread *thread)
{
	BaseContainer *bc = op->GetDataInstance();
	if (!bc) return;

	if (!rcol) return;

	ed.mode = bc->GetLong( DROPEFFECTOR_MODE );
	ed.maxdist = bc->GetReal( DROPEFFECTOR_DISTANCE );
	ed.target = bc->GetObjectLink( DROPEFFECTOR_TARGET, doc );
	if (!ed.target) return;

	ed.targmg = ed.target->GetMg();
	ed.itargmg = !ed.targmg;
	ed.genmg = gen->GetMg();
	ed.igenmg = !ed.genmg;

	//Add a dependency so that the effector will update if the target changes
	AddEffectorDependence(ed.target);

	//Can't init raycollider or the target isn't polygonal, then skip
	if (!rcol->Init(ed.target)) ed.target=NULL;
	else if (!ed.target->IsInstanceOf(Opolygon)) ed.target=NULL;
}

void DropEffector::ModifyPoints(BaseObject *op,BaseObject *gen,BaseDocument *doc,EffectorDataStruct *data,MoData *md,BaseThread *thread)
{
	if (!ed.target || !rcol || data->strength == 0.0) return;

	C4D_Falloff *falloff=GetFalloff();
	if (!falloff) return;

	LONG i = 0;
	Real fall = 0.0;
	Vector off = 0.0;
	Vector ray_p = 0.0, ray_dir = 0.0;
	Vector targ_off = 0.0, targ_hpb = 0.0;
	
	MDArray<LONG>flag_array = md->GetLongArray(MODATA_FLAGS);
	MDArray<Matrix>mat_array = md->GetMatrixArray(MODATA_MATRIX);
	MDArray<Real>weight_array = md->GetRealArray(MODATA_WEIGHT);

	if (!mat_array) return;

	LONG mdcount = md->GetCount();
	for (i=0;i<mdcount;i++)
	{
		//If the particle isn't visible, don't calculate
		if (!(flag_array[i]&MOGENFLAG_CLONE_ON) || (flag_array[i]&MOGENFLAG_DISABLE)) continue;

		//Multiply into global space
		off = mat_array[i].off;
		off *= ed.genmg;

		//Sample the falloff
		falloff->Sample( off, &fall, TRUE, weight_array[i] );
		if (fall==0.0) continue;

		//Set up the ray for the collision
		ray_p = off * ed.itargmg;
		switch (ed.mode)
		{
			default:
			case DROPEFFECTOR_MODE_PNORMAL:		ray_dir = ( mat_array[i].v3 ^ ed.genmg );						break;
			case DROPEFFECTOR_MODE_NNORMAL:		ray_dir = ( -mat_array[i].v3 ^ ed.genmg );					break;
			case DROPEFFECTOR_MODE_AXIS:			ray_dir = ( ed.targmg.off - off );									break;
			case DROPEFFECTOR_MODE_SELFAXIS:	ray_dir = ( ed.genmg.off - off );										break;
			case DROPEFFECTOR_MODE_PX:				ray_dir = Vector( 1.0, 0.0, 0.0 );									break;
			case DROPEFFECTOR_MODE_PY:				ray_dir = Vector( 0.0, 1.0, 0.0 );									break;
			case DROPEFFECTOR_MODE_PZ:				ray_dir = Vector( 0.0, 0.0, 1.0 );									break;
			case DROPEFFECTOR_MODE_NX:				ray_dir = Vector( -1.0, 0.0, 0.0 );									break;
			case DROPEFFECTOR_MODE_NY:				ray_dir = Vector( 0.0, -1.0, 0.0 );									break;
			case DROPEFFECTOR_MODE_NZ:				ray_dir = Vector( 0.0, 0.0, -1.0 );									break;
		}
		ray_dir ^= ed.itargmg;

		//Calculate an intersection
		if (rcol->Intersect(ray_p, !ray_dir, ed.maxdist, FALSE))
		{
			if (rcol->GetNearestIntersection(&rcolres))
			{
				fall *= data->strength;

				targ_off = Mix( mat_array[i].off, ( rcolres.hitpos * ed.targmg ) * ed.igenmg , fall);
				targ_hpb = VectorToHPB( ( rcolres.s_normal ^ ed.targmg ) ^ ed.igenmg );

				mat_array[i] = HPBToMatrix( Mix( MatrixToHPB( mat_array[i],ROTATIONORDER_DEFAULT ), targ_hpb, fall),ROTATIONORDER_DEFAULT );
				mat_array[i].off = targ_off;;
			}
		}
	}
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_DROPEFFECTOR 1019571

Bool RegisterDropEffector(void)
{
	return RegisterEffectorPlugin(ID_DROPEFFECTOR,GeLoadString(IDS_DROPEFFECTOR),OBJECT_CALL_ADDEXECUTION,DropEffector::Alloc,"oedrop",AutoBitmap("dropeffector.tif"),0);
}
