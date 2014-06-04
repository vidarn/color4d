#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_modeling.h"

#include "c4d_descriptiondialog.h"
#include "tooledgecutsdk.h"

#define SUBDIV_DELTA			30.0f
#define ID_MODELING_EDGECUT_TOOL_SDK			450000025

extern Bool AddUndo(BaseDocument* doc, AtomArray* arr, UNDOTYPE type);

class DynVectorArray
{
public:
	DynVectorArray()
	{
		data = NULL;
		dcnt = 0;
		mcnt = 0;
	}

	~DynVectorArray()
	{
		Free();
	}

	void Free()
	{
		GeFree(data);
		mcnt = 0;
		dcnt = 0;
	}

	Bool Alloc(LONG a)
	{
		Free();
		data = GeAllocType(Vector,a);
		if (!data)
			return FALSE;
		mcnt = a;
		return TRUE;
	}

	Bool Append(const Vector& e)
	{
		if (dcnt < mcnt)
		{
			data[dcnt++] = e;
			return TRUE;
		}
		Vector* d = GeAllocType(Vector,mcnt + 30);
		if (!d)
			return FALSE;
		CopyMemType(data, d, mcnt );
		GeFree(data);
		data = d;
		mcnt += 30;
		data[dcnt++] = e;
		return TRUE;
	}

	void ResetCounter() { dcnt = 0; }
	LONG GetElementCount() { return dcnt; }
	Vector* GetArray() { return data; }

private:
	Vector* data;
	LONG dcnt, mcnt;
};

class EdgeCutTool : public DescriptionToolData
{
		Bool ModelingEdgeCut(AtomArray* arr, MODELINGCOMMANDMODE mode, BaseContainer* data, BaseDocument* doc, EditorWindow *win, const BaseContainer* msg, Bool undo, EdgeCutTool* tool);

	public:
		EdgeCutTool();
		
		virtual LONG				GetToolPluginId() { return ID_MODELING_EDGECUT_TOOL_SDK; }
		virtual const String GetResourceSymbol() { return String("ToolEdgeCutSDK"); }

		virtual Bool				MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg);
		virtual LONG				GetState(BaseDocument *doc);

		virtual void				InitDefaultSettings(BaseDocument *doc, BaseContainer &data);
		virtual Bool				DoCommand(ModelingCommandData &mdat);
		virtual Bool				GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x, Real y, BaseContainer &bc);

		virtual Bool				GetDEnabling(BaseDocument *doc, BaseContainer &data, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);
		virtual TOOLDRAW		Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags);

	protected:
		Bool isdragging;
		DynVectorArray cutpoints;
};

EdgeCutTool::EdgeCutTool()
{
	isdragging = FALSE;
}

Bool EdgeCutTool::GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x, Real y, BaseContainer &bc)
{
	if (bc.GetId()==BFM_CURSORINFO_REMOVE) return TRUE;
	bc.SetString(RESULT_BUBBLEHELP, GeLoadString(IDS_HLP_EDGECUT_SDK));
	return TRUE;
}

void EdgeCutTool::InitDefaultSettings(BaseDocument *doc, BaseContainer &data)
{
	data.SetLong(MDATA_EDGECUTSDK_SUBDIV,1);
	data.SetReal(MDATA_EDGECUTSDK_OFFSET,0.5);
	data.SetReal(MDATA_EDGECUTSDK_SCALE,1.0);
	data.SetBool(MDATA_EDGECUTSDK_CREATENGONS,TRUE);
	DescriptionToolData::InitDefaultSettings(doc,data);
}

Bool EdgeCutTool::GetDEnabling(BaseDocument *doc, BaseContainer &data, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc)
{
	return DescriptionToolData::GetDEnabling(doc,data,id,t_data,flags,itemdesc);
}

LONG EdgeCutTool::GetState(BaseDocument *doc)
{
	AutoAlloc <AtomArray> arr;
	if (!doc || !arr)
		return 0;
	if (doc->GetMode()!=Medges) return 0;
	doc->GetActivePolygonObjects(*arr, TRUE);
	if (arr->GetCount() == 0) return 0;
	return CMD_ENABLED;
}

Bool EdgeCutTool::DoCommand(ModelingCommandData &mdat)
{
	return ModelingEdgeCut(mdat.arr,mdat.mode,mdat.bc,mdat.doc,NULL,NULL, (mdat.doc != NULL) && (mdat.flags & MODELINGCOMMANDFLAGS_CREATEUNDO), NULL);
}

TOOLDRAW EdgeCutTool::Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags)
{
	if (!(flags & TOOLDRAWFLAGS_HIGHLIGHT)) 
		return TOOLDRAW_0;

	if (!isdragging)
		return TOOLDRAW_HANDLES;

	bd->LineZOffset(3);
	bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
	bd->SetMatrix_Matrix(NULL, Matrix());
	LONG cnt = cutpoints.GetElementCount();
	Vector* points = cutpoints.GetArray();
	LONG a;
	for (a = 0; a < cnt; a++)
		bd->DrawHandle(points[a], DRAWHANDLE_MIDDLE, 0);
	bd->LineZOffset(0);

	return TOOLDRAW_HANDLES;
}


struct EdgeCutPoint
{
	LONG ptind; // index of the point in the final object
	LONG pos; // position on the edge
	LONG pt1a, pt2;
	CHAR reverse;
};

class EdgeCutPointArray : public c4d_misc::BaseSort<EdgeCutPointArray>
{
public:
	EdgeCutPointArray()
	{
		data = NULL;
		mcnt = 0;
		dcnt = 0;
	}

	~EdgeCutPointArray()
	{
		GeFree(data);
		mcnt = 0;
		dcnt = 0;
	}

	static Bool LessThan(const EdgeCutPoint& a, const EdgeCutPoint& b)
	{
		return a.ptind < b.ptind;
	}

	Bool Alloc(LONG l)
	{
		data = GeAllocType(EdgeCutPoint,l);
		if (!data) return FALSE;
		mcnt = l;
		return TRUE;
	}

	Bool Append(const EdgeCutPoint& e)
	{
		if (dcnt < mcnt)
		{
			data[dcnt++] = e;
			return TRUE;
		}
		EdgeCutPoint* d = GeAllocType(EdgeCutPoint,mcnt + 30);
		if (!d)
			return FALSE;
		CopyMemType(data, d, mcnt );
		GeFree(data);
		data = d;
		mcnt += 30;
		data[dcnt++] = e;
		return TRUE;
	}

	EdgeCutPoint* GetData() { return data; }
	LONG GetCount() { return dcnt; }

private:
	EdgeCutPoint* data;
	LONG dcnt, mcnt;
};

class ActiveEdgeCutObject
{
public:
	ActiveEdgeCutObject()
	{
		op = NULL;
		mkernel = NULL;
		op = NULL;
	}
	~ActiveEdgeCutObject()
	{
		Modeling::Free(mkernel);
	}
	Bool Prepare(Bool selall, MODELINGCOMMANDMODE mode, LONG subdiv, Bool subdivide);
	Bool ReInit(Bool selall, MODELINGCOMMANDMODE mode, LONG subdiv, Bool subdivide);

	PolygonObject* op;
	EdgeCutPointArray cutpoints;
	BaseSelect* seledges;
	Modeling* mkernel;
};

class ActiveEdgeCutArray
{
public:
	ActiveEdgeCutArray()
	{
		selall = TRUE;
		objects = NULL;
		ocnt = 0;
	}

	~ActiveEdgeCutArray()
	{
		Free();
	}

	void Free()
	{
		bDelete(objects);
		ocnt = 0;
	}

	Bool Init(AtomArray* arr)
	{
		LONG a;
		selall = TRUE;
		LONG cnt = 0;
		for (a = 0; a < arr->GetCount(); a++)
		{
			C4DAtom* at = arr->GetIndex(a);
			if (!at->IsInstanceOf(Opolygon))
			{
				GeAssert(FALSE);
				return FALSE;
			}
			if (((PolygonObject*)at)->GetEdgeS()->GetCount() != 0)
			{
				selall = FALSE;
			}
			cnt++;
		}
		ocnt = cnt;
		GeAssert(ocnt == arr->GetCount());

		objects = bNew ActiveEdgeCutObject[ocnt];
		if (!objects)
			return FALSE;

		for (a = 0; a < ocnt; a++)
		{
			objects[a].mkernel = Modeling::Alloc();
			if (!objects[a].mkernel)
				return FALSE;

			C4DAtom* at = arr->GetIndex(a);
			if (!at->IsInstanceOf(Opolygon))
			{
				GeAssert(FALSE);
				return FALSE;
			}
			objects[a].op = (PolygonObject*)at;
		}
		return GetElementCount() > 0;
	}
	Bool ReInit(AtomArray* arr, MODELINGCOMMANDMODE mode, LONG subdiv, Bool subdivide)
	{
		bDelete(objects);
		ocnt = 0;
		
		if (!Init(arr))
			return FALSE;
		LONG a;
		for (a = 0; a < GetElementCount(); a++)
		{
			if (!GetObject(a).Prepare(selall, mode, subdiv, subdivide))
				return FALSE;
		}
		return TRUE;
	}
	Bool Prepare(MODELINGCOMMANDMODE mode, LONG subdiv, Bool subdivide)
	{
		LONG a;
		for (a = 0; a < GetElementCount(); a++)
		{
			if (!GetObject(a).Prepare(selall, mode, subdiv, subdivide))
				return FALSE;
		}
		return TRUE;
	}

	LONG GetElementCount() { return ocnt; }
	ActiveEdgeCutObject &GetObject(LONG a) { return objects[a]; }

	Bool selall;
	ActiveEdgeCutObject* objects;
	LONG ocnt;
};

Bool ActiveEdgeCutObject::Prepare(Bool selall, MODELINGCOMMANDMODE mode, LONG subdiv, Bool subdivide)
{
	if (!mkernel->InitObject(op)) return FALSE;

	LONG polycnt = op->GetPolygonCount();
	Bool ok=FALSE;
	Bool all = mode != MODELINGCOMMANDMODE_EDGESELECTION || selall;
	if (all)
	{
		op->GetEdgeS()->SelectAll(0, 4 * polycnt - 1);
		op->GetEdgeS()->SelectAll(0, 4 * polycnt - 1);
	}
	seledges = op->GetEdgeS();

	LONG seg=0,a,b,i,p1,p2,lp,steps,stp;
	EdgeCutPoint ep;

	cutpoints.Alloc(seledges->GetCount() * (subdiv + 1));
	Bool isfirst = TRUE;
	CHAR reverse;
	Vector v1, v2, v3, v4, vDir1, vDir2;
	const Vector* points = op->GetPointR();

	while (seledges->GetRange(seg++,polycnt*4,&a,&b))
	{
		for (i=a; i<=b; i++)
		{
			if (mkernel->GetOriginalEdgePoints(op,i,p1,p2))
			{
				if (isfirst)
				{
					v1 = points[p1];
					v2 = points[p2];
					isfirst = FALSE;
					reverse = FALSE;
				}
				else
				{
					v3 = points[p1];
					v4 = points[p2];
					vDir1 = !((v2 - v1) % (v3 - v1));
					vDir2 = !((v3 - v1) % (v4 - v1));
					if (vDir1 * vDir2 <= 0.0)
						reverse = FALSE;
					else
						reverse = TRUE;
				}
				ep.pt1a = p1;
				ep.pt2 = p2;
				LONG nindex=i/4;//mkernel->TranslateNgonIndex(op,i/4);
				if (mkernel->IsValidEdge(op,nindex,p1,p2))
				{
		/*
					if (subdivide)
					{
						LONG lEdgeNgons;
						LONG* plNgons = mkernel->GetEdgeNgons(op, p1, p2, lEdgeNgons);
						while (--lEdgeNgons >= 0)
							mkernel->SetNgonFlags(op, plNgons[lEdgeNgons], MODELING_SETNGON_FLAG_TRIANGULATE);
						GeFree(plNgons);
					}
		  */
					steps = subdiv+1;
					lp=p1;
					for (stp=0;stp<subdiv;stp++,steps--)
					{
						if ((lp=mkernel->SplitEdge(op,lp,p2,1.0f/Real(steps)))==0)
							goto Exit;
						ep.ptind = lp;
						ep.pos = stp;
						ep.reverse = reverse;
						if (!cutpoints.Append(ep))
							goto Exit;
					}
				}
			}
		}
	}
	if (!mkernel->Commit(op, MODELING_COMMIT_UPDATE))
		goto Exit;
	
	ok = TRUE;
Exit:
	return ok;
}


#define POINT_MIN     (0.0)
#define POINT_MAX     (1.0)

Bool EdgeCutTool::ModelingEdgeCut(AtomArray* arr, MODELINGCOMMANDMODE mode, BaseContainer* data, BaseDocument* doc, EditorWindow *win, const BaseContainer* msg, Bool undo, EdgeCutTool* tool)
{
	if (!data) 
		return FALSE;
	arr->FilterObject(-1, Opolygon);
	if (arr->GetCount() < 1)
		return TRUE;
	
	ActiveEdgeCutArray active;
	if (!active.Init(arr))
		return FALSE;
	
	Bool ok = FALSE;
	LONG i, j;

	LONG subdiv = LMax(1, data->GetLong(MDATA_EDGECUTSDK_SUBDIV));
	Real offset = data->GetReal(MDATA_EDGECUTSDK_OFFSET, .5f);
	Real scale = data->GetReal(MDATA_EDGECUTSDK_SCALE, 1.0f);
	Bool subdivide = !data->GetBool(MDATA_EDGECUTSDK_CREATENGONS);
	Real* pos1 = GeAllocType(Real,subdiv);
	Real* pos2 = GeAllocType(Real,subdiv);
	Real temp1, temp2;
	Vector tmp(DC);
	if (!pos1 || !pos2)
		return FALSE;
	if (scale < .0001)
		scale = 0.0001;

	if (undo)
		AddUndo(doc, arr, UNDOTYPE_CHANGE);

	if (!active.Prepare(mode, subdiv, subdivide))
		goto Exit;

	if (!win)
	{
		for (i = 0; i < subdiv; i++)
		{
			pos1[i] = offset + (Real(1 + i) / Real(subdiv + 1)) * scale - .5f;
			pos2[i] = 1 - offset + (Real(1 + i) / Real(subdiv + 1)) * scale - .5f;
		}
		for (i = 0; i < active.GetElementCount(); i++)
		{
			ActiveEdgeCutObject& obj = active.GetObject(i);
			EdgeCutPoint* cutpoints = obj.cutpoints.GetData();
			LONG ptcnt = obj.cutpoints.GetCount();
			for (j = 0; j < ptcnt; j++)
			{
				Real rPos = cutpoints[j].reverse ? pos1[cutpoints[j].pos] : pos2[cutpoints[j].pos];
				if (rPos < POINT_MIN || rPos > POINT_MAX)
				{
					if (!obj.mkernel->DeletePoint(obj.op, obj.mkernel->TranslatePointIndex(obj.op, cutpoints[j].ptind)))
						goto Exit;
				}
				else
				{
					if (!obj.mkernel->SetEdgePoint(obj.op, cutpoints[j].ptind, rPos))
						goto Exit;
				}
			}
			if (!obj.mkernel->Commit(obj.op, MODELING_COMMIT_UPDATE  | ((subdivide) ? MODELING_COMMIT_TRINGONS | MODELING_COMMIT_QUADS : 0)))
				goto Exit;
		}
	}
	else
	{
		BaseContainer backup = *data;
		Bool first = TRUE;
		Real dx, dy;
		BaseContainer device;
		scale = 1.0f;
		offset = .5f;
		Real mousex = msg->GetLong(BFM_INPUT_X);
		Real mousey = msg->GetLong(BFM_INPUT_Y);
		win->MouseDragStart(KEY_MLEFT, mousex, mousey, MOUSEDRAGFLAGS_NOMOVE);
		Bool shift, ctrl;
		Real subdivchange = 0;
		const Vector* points;
		if (!tool)
		{
			GeAssert(FALSE);
			win->MouseDragEnd();
			goto Exit;
		}
		tool->isdragging = TRUE;
		while (win->MouseDrag(&dx, &dy, &device) == MOUSEDRAGRESULT_CONTINUE)
		{
			if (dx == 0 && dy == 0 && !first)
				continue;
			shift = (device.GetLong(BFM_INPUT_QUALIFIER) & QSHIFT);
			ctrl = (device.GetLong(BFM_INPUT_QUALIFIER) & QCTRL);
			if (!first)
			{
				if (shift && !ctrl)
				{
					dx /= 200.0f;
					Real tempscale = scale + dx;
					if (tempscale < .0001)
						tempscale = 0.0001;

					// check that all points are in the [0.05, 0.95] range
					temp1 = offset + (Real(subdiv) / Real(subdiv + 1) - .5f) * tempscale;
					temp2 = offset + (Real(1) / Real(subdiv + 1) - .5f) * tempscale;
					if (temp1 < POINT_MIN || temp1 > POINT_MAX || temp2 < POINT_MIN || temp2 > POINT_MAX)
						continue;
					scale = tempscale;
				}
				else if (!shift && ctrl)
				{
					dx /= 200.0f;
					Real tempoffset = offset + dx;
					// check that all points are in the [0.05, 0.95] range
					temp1 = tempoffset + (Real(subdiv) / Real(subdiv + 1) - .5f) * scale;
					temp2 = tempoffset + (Real(1) / Real(subdiv + 1) - .5f) * scale;
					if (temp1 < POINT_MIN || temp1 > POINT_MAX || temp2 < POINT_MIN || temp2 > POINT_MAX)
						continue;
					offset = tempoffset;
				}
				else
				{
					LONG lSubdivChange = 0;
					subdivchange += dx;
					if (subdivchange <= -SUBDIV_DELTA)
					{
						lSubdivChange = -1;
						while (subdivchange <= -SUBDIV_DELTA)
							subdivchange += SUBDIV_DELTA;
					}
					else if (subdivchange >= SUBDIV_DELTA)
					{
						lSubdivChange = 1;
						while (subdivchange >= SUBDIV_DELTA)
							subdivchange -= SUBDIV_DELTA;
					}
					if (lSubdivChange)
					{
						if (lSubdivChange < 0 && subdiv == 1)
							continue;
						subdiv += lSubdivChange;
						if (!undo || !doc)
						{
							GeAssert(FALSE);
							win->MouseDragEnd();
							goto Exit;
						}
						doc->EndUndo();
						doc->DoUndo();
						doc->GetActivePolygonObjects(*arr, TRUE);
						if (arr->GetCount() < 1)
						{
							win->MouseDragEnd();
							goto Exit;
						}
						doc->StartUndo();
						AddUndo(doc, arr, UNDOTYPE_CHANGE);
						doc->EndUndo();

						if (!active.ReInit(arr, mode, subdiv, subdivide))
						{
							win->MouseDragEnd();
							goto Exit;
						}
						GeFree(pos1);
						GeFree(pos2);
						pos1 = GeAllocType(Real,subdiv);
						pos2 = GeAllocType(Real,subdiv);
						if (!pos1 || !pos2)
						{
							win->MouseDragEnd();
							goto Exit;
						}
						for (i = 0; i < subdiv; i++)
						{
							pos1[i] = offset + ((Real(1 + i) / Real(subdiv + 1)) - .5f) * scale;
							pos2[i] = 1 - offset + ((Real(1 + i) / Real(subdiv + 1)) - .5f) * scale;
						}
					}
				}
			}
			first = FALSE;
			for (i = 0; i < subdiv; i++)
			{
				pos1[i] = offset + ((Real(1 + i) / Real(subdiv + 1)) - .5f) * scale;
				pos2[i] = 1 - offset + ((Real(1 + i) / Real(subdiv + 1)) - .5f) * scale;
				GeAssert(pos1[i] >= POINT_MIN && pos1[i] <= POINT_MAX);
				GeAssert(pos2[i] >= POINT_MIN && pos2[i] <= POINT_MAX);
			}

			j = 0;
			for (i = 0; i < active.GetElementCount(); i++)
			{
				ActiveEdgeCutObject& obj = active.GetObject(i);
				j += obj.cutpoints.GetCount();
			}
			if (!tool->cutpoints.Alloc(j + 1))
			{
				win->MouseDragEnd();
				goto Exit;
			}

			tool->cutpoints.ResetCounter();
			for (i = 0; i < active.GetElementCount(); i++)
			{
				ActiveEdgeCutObject& obj = active.GetObject(i);
				EdgeCutPoint* cutpoints = obj.cutpoints.GetData();
				LONG ptcnt = obj.cutpoints.GetCount();
				Matrix mg = obj.op->GetMg();
				points = obj.op->GetPointR();
				for (j = 0; j < ptcnt; j++)
				{
					Real rPos = cutpoints[j].reverse ? pos1[cutpoints[j].pos] : pos2[cutpoints[j].pos];
					if (!obj.mkernel->SetEdgePoint(obj.op, cutpoints[j].ptind, rPos))
					{
						win->MouseDragEnd();
						goto Exit;
					}
					tmp = Mix(points[cutpoints[j].pt1a], points[cutpoints[j].pt2], rPos) * mg;
					tool->cutpoints.Append(tmp);
				}
				if (!obj.mkernel->Commit(obj.op, MODELING_COMMIT_UPDATE  | ((subdivide) ? MODELING_COMMIT_TRINGONS | MODELING_COMMIT_QUADS : 0)))
				{
					win->MouseDragEnd();
					goto Exit;
				}
			}
			BaseContainer *writeback = GetToolData(doc,GetToolPluginId());
			if (writeback)
			{
				writeback->SetReal(MDATA_EDGECUTSDK_OFFSET, offset);
				writeback->SetReal(MDATA_EDGECUTSDK_SCALE, scale);
				writeback->SetLong(MDATA_EDGECUTSDK_SUBDIV, subdiv);
				GeSyncMessage(EVMSG_TOOLCHANGED, 0);
			}
			DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
		}
		if (win->MouseDragEnd() != MOUSEDRAGRESULT_FINISHED)
		{
			backup.CopyTo(data,COPYFLAGS_0,NULL);
			goto Exit;
		}
		else
			ok = TRUE;
	}
	EventAdd();

	ok = TRUE;
Exit:
	if (tool)
	{
		tool->isdragging = FALSE;
		tool->cutpoints.Free();
	}
	GeFree(pos1);
	GeFree(pos2);

	if (!ok && undo) doc->DoUndo(TRUE);
	
	return ok;
}
	

Bool EdgeCutTool::MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg)
{
	if (!doc) return FALSE;

	if (doc->GetMode() == Medges)
	{
		AutoAlloc<AtomArray> arr;
		if (!arr)
			return FALSE;
		doc->GetActivePolygonObjects(*arr, TRUE);

		BaseContainer* data = GetToolData(doc, ID_MODELING_EDGECUT_TOOL);
		if (!data)
			return FALSE;

		// undo the step before
		InteractiveModeling_Restart(doc);

		ModelingEdgeCut(arr, MODELINGCOMMANDMODE_EDGESELECTION, data, doc, win, &msg, TRUE, this);
		EventAdd();
		return TRUE;
	}
	return TRUE;
}

Bool RegisterEdgeCutTool()
{
	return RegisterToolPlugin(ID_MODELING_EDGECUT_TOOL_SDK, GeLoadString(IDS_EDGECUT_SDK), 0, NULL, GeLoadString(IDS_HLP_EDGECUT_SDK), gNew EdgeCutTool);
}
