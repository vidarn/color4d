/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// morph mixing example

#include "c4d.h"
#include "c4d_symbols.h"
#include "omorphmixer.h"

#define MAXTARGETS 100

class MorphMixerObject : public ObjectData
{
	INSTANCEOF(MorphMixerObject,ObjectData)

	private:
		void MagpieImport(BaseObject *op);
		
	public:
		virtual Bool Message(GeListNode *node, LONG type, void *data);
		virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
		virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);

		static NodeData *Alloc(void) { return gNew MorphMixerObject; }
};

static Bool ReadLine(BaseFile *bf, String *v)
{
	CHAR ch,line[1024];
	LONG i = 0, len = bf->TryReadBytes(&ch, 1);
	
	if (len == 0)
		return FALSE; // end of file
	
	while (i<1024 && len == 1 && ch != '\n' && ch != '\r') 
	{
		line[i++] = ch;
		len = bf->TryReadBytes(&ch, 1);
	}
#ifdef __PC
	if (ch == '\r') 
	{
		len = bf->TryReadBytes(&ch, 1);
		if (len == 1 && ch != '\n')
			bf->Seek(-1);
	}
#endif
	v->SetCString(line, i);
	return TRUE;
}

static Bool CreateKey(BaseDocument *doc, BaseObject *op, const BaseTime &time, LONG index, Real value)
{
	// check if track exists
	CTrack *track = op->FindCTrack(DescLevel(index,DTYPE_REAL,0));
	if (!track)
	{
		track = CTrack::Alloc(op,DescLevel(index,DTYPE_REAL,0)); if (!track) return FALSE;
		op->InsertTrackSorted(track);
	}

	CKey *key = track->GetCurve()->AddKey(time); if (!key) return FALSE;
	key->SetValue(track->GetCurve(),value);
	return TRUE;
}

void MorphMixerObject::MagpieImport(BaseObject *op)
{
	// import a Magpie Pro file
	BaseDocument *doc=GetActiveDocument(); 
	if (!doc) return;

	Filename		f;
	LONG				err=0,frame_cnt,expression_cnt=0,target_cnt=0;
	String			line;
	LONG				expression_index[MAXTARGETS];
	LONG				i, j;
	BaseObject	*target[MAXTARGETS];
	BaseObject	*pp;

	if (!f.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, GeLoadString(IDS_MORPHSELECT))) return;

	AutoAlloc<BaseFile> bf;
	if (!bf || !bf->Open(f)) goto Error;
	
	ReadLine(bf,&line);
	expression_cnt = line.ToLong(&err);
	
	if (err || expression_cnt<=0) goto Error;

	for (pp=op->GetDown();pp && target_cnt<expression_cnt;pp=pp->GetNext())
	{
		if (pp->GetType()!=Opolygon) continue;
		target[target_cnt++] = pp;
	}
	
	for (i=0; i<expression_cnt; i++) 
	{
		ReadLine(bf,&line);

		if (i>=target_cnt) continue;

		line = line.ToUpper();
		expression_index[i]=-1;
		for (j=0; j<target_cnt; j++) 
		{
			if (target[j]->GetName().ToUpper()==line)
				expression_index[i]=j;
		}
	}

	ReadLine(bf,&line);
	frame_cnt = line.ToLong(&err);
	if (err) goto Error;

	for (i=0; i<frame_cnt; i++) 
	{
		ReadLine(bf,&line);
		Real time = (Real)line.ToLong() / doc->GetFps();

		for (j = 0; j < expression_cnt; j++) 
		{
			ReadLine(bf,&line);
			line.Delete(0, 1);

			Real val = line.ToReal(&err);
			if (err) goto Error;

			if (expression_index[j]==-1) continue;
			if (!CreateKey(doc,op,BaseTime(time),MORPHMIXER_POSEOFFSET+expression_index[j],val)) goto Error;
		}
	}

	op->Message(MSG_UPDATE);
	return;

Error:
	op->Message(MSG_UPDATE);
	GeOutString(GeLoadString(IDS_IMPORTFAILED),GEMB_OK);
}

Bool MorphMixerObject::GetDDescription(GeListNode *node, Description *description,DESCFLAGS_DESC &flags)
{
	if (!description->LoadDescription(node->GetType())) return FALSE;

	// important to check for speedup c4d!
	const DescID *singleid = description->GetSingleDescID();

	BaseObject *pp;
	Bool first=TRUE;
	LONG index=MORPHMIXER_POSEOFFSET;
	
	Bool initbc2 = FALSE;
	BaseContainer bc2;

	for (pp=(BaseObject*)node->GetDown(); pp; pp=pp->GetNext())
	{
		if (pp->GetType()!=Opolygon) continue;

		if (first) { first=FALSE; continue; }

		DescID cid = DescLevel(index,DTYPE_REAL,0);
		if (!singleid || cid.IsPartOf(*singleid,NULL)) // important to check for speedup c4d!
		{
			if (!initbc2)
			{
				initbc2 = TRUE;
				bc2 = GetCustomDataTypeDefault(DTYPE_REAL);
				bc2.SetLong(DESC_CUSTOMGUI,CUSTOMGUI_REALSLIDER);
				bc2.SetReal(DESC_MIN,0.0);
				bc2.SetReal(DESC_MAX,1.0);
				bc2.SetReal(DESC_STEP,0.01);
				bc2.SetLong(DESC_UNIT,DESC_UNIT_PERCENT);
				bc2.SetLong(DESC_ANIMATE,DESC_ANIMATE_ON);
				bc2.SetBool(DESC_REMOVEABLE,FALSE);
			}
			bc2.SetString(DESC_NAME,pp->GetName());
			bc2.SetString(DESC_SHORT_NAME,pp->GetName());
			if (!description->SetParameter(cid,bc2,DescLevel(ID_OBJECTPROPERTIES))) return FALSE;
		}
		index++;
	}

	flags |= DESCFLAGS_DESC_LOADED;

	return SUPER::GetDDescription(node,description,flags);
}

Bool MorphMixerObject::Message(GeListNode *node, LONG type, void *data)
{
	switch (type)
	{
		case MSG_DESCRIPTION_COMMAND:
		{
			DescriptionCommand *dc = (DescriptionCommand*) data;
			if (dc->id[0].id==MORPHMIXER_RECORD)
			{
				Bool					first=TRUE;
				BaseObject		*op = (BaseObject*)node,*pp;
				BaseDocument	*doc = node->GetDocument(); if (!doc) break;
				BaseContainer *dt = op->GetDataInstance();

				LONG index=MORPHMIXER_POSEOFFSET;
				for (pp=op->GetDown();pp;pp=pp->GetNext())
				{
					if (pp->GetType()!=Opolygon) continue;

					if (first) { first=FALSE; continue; }

					if (!CreateKey(doc,op,doc->GetTime(),index,dt->GetReal(index))) break;
					index++;
				}

				EventAdd();
			}
			else if (dc->id[0].id==MORPHMIXER_IMPORT)
			{
				MagpieImport((BaseObject*)node);
				EventAdd(EVENT_ANIMATE);
			}
		}
	}

	return TRUE;
}

// create morphed object
BaseObject *MorphMixerObject::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	Vector				*destadr=NULL;
	const Vector	*baseadr=NULL,*childadr=NULL;
	LONG					i,j,pcnt;
	Real					strength;
	PolygonObject	*childs[MAXTARGETS],*base=NULL;
	LONG					child_cnt=0;
	BaseObject		*ret=NULL,*pp=NULL,*orig=op->GetDown();
	BaseContainer	*data=op->GetDataInstance();

	// if no child is available, return NULL
	if (!orig) return NULL;

	// start new list
	op->NewDependenceList();

	// check cache for validity and check master object for changes
	Bool dirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS_DATA);

	// for each child
	for (pp=orig; pp; pp=pp->GetNext())
	{
		// if object is polygonal and has not been processed yet
		if (pp->GetType()==Opolygon && !pp->GetBit(BIT_CONTROLOBJECT))
		{
			// add object to list
			op->AddDependence(hh,pp);
			childs[child_cnt++]=(PolygonObject*)pp;
		}
	}

	// no child object found
	if (!child_cnt) return NULL;

	// if child list has been modified somehow
	if (!dirty) dirty = !op->CompareDependenceList();

	// mark child objects as processed
	op->TouchDependenceList();

	// if no change has been detected, return original cache
	if (!dirty) return op->GetCache(hh);

	// set morphing base
	base=childs[0];

	// clone this object
	ret=(BaseObject*)base->GetClone(COPYFLAGS_NO_HIERARCHY|COPYFLAGS_NO_ANIMATION|COPYFLAGS_NO_BITS,NULL);
	if (!ret) goto Error;

	// and transfer tags
	if (!op->CopyTagsTo(ret,TRUE,FALSE,FALSE,NULL)) goto Error;

	// transfer name
	ret->SetName(op->GetName());

	// retrieve destination and base points
	destadr = ((PolygonObject*)ret)->GetPointW();
	baseadr = base->GetPointR();

	// for each child, except the child base object (j==0)
	for (j=1; j<child_cnt; j++)
	{
		// get minimum number of shared points
		pcnt=LMin(base->GetPointCount(),childs[j]->GetPointCount());

		// get morph percentage
		strength=data->GetReal(MORPHMIXER_POSEOFFSET+j-1);

		// get point address of child
		childadr = childs[j]->GetPointR();

		// add weighted morph
		for (i=0; i<pcnt; i++)
			destadr[i]+=(childadr[i]-baseadr[i])*strength;
	}

	// send update message
	ret->Message(MSG_UPDATE);

	return ret;

Error:
	BaseObject::Free(ret);
	return NULL;
}

// be sure to use unique IDs obtained from www.plugincafe.com
#define ID_MORPHMIXER_OBJECT	1001156

Bool RegisterMorphMixer(void)
{
	return RegisterObjectPlugin(ID_MORPHMIXER_OBJECT,GeLoadString(IDS_MORPHMIXER),OBJECT_GENERATOR|OBJECT_INPUT,MorphMixerObject::Alloc,"Omorphmixer",AutoBitmap("morphmixer.tif"),0);
}
