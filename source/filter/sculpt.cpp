/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2012 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// Sculpt Data import and export

#include <stdio.h>
#include <string.h>
#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_sculpt.h"

#define FILE_VERSION 0

class SculptLoaderData : public SceneLoaderData
{
	public:
		virtual Bool Init(GeListNode *node);
		virtual Bool Identify(BaseSceneLoader *node, const Filename &name, UCHAR *probe, LONG size);
		virtual FILEERROR Load(BaseSceneLoader *node, const Filename &name, BaseDocument *doc, SCENEFILTER filterflags, String *error, BaseThread *bt);

		static NodeData *Alloc(void) { return gNew SculptLoaderData; }
};

Bool SculptLoaderData::Init(GeListNode *node)
{
	return TRUE;
}

class SculptSaverData : public SceneSaverData
{
	public:
		virtual Bool Init(GeListNode *node);
		virtual FILEERROR Save(BaseSceneSaver *node, const Filename &name, BaseDocument *doc, SCENEFILTER filterflags);

		static NodeData *Alloc(void) { return gNew SculptSaverData; }
};

Bool SculptSaverData::Init(GeListNode *node)
{
	return TRUE;
}

Bool SculptLoaderData::Identify(BaseSceneLoader *node, const Filename &name, UCHAR *probe, LONG size)
{
	if (!name.CheckSuffix("SCP")) return FALSE;
	return TRUE;
}

FILEERROR SculptLoaderData::Load(BaseSceneLoader *node, const Filename &name, BaseDocument *doc, SCENEFILTER flags, String *error, BaseThread *thread)
{
	if (!(flags&SCENEFILTER_OBJECTS)) return FILEERROR_NONE;

	AutoAlloc<BaseFile> file;
	if(!file) return FILEERROR_OUTOFMEMORY;
	if (!file->Open(name,FILEOPEN_READ,FILEDIALOG_NONE,BYTEORDER_INTEL)) return file->GetError();

	LONG version = 0;
	LONG polyCount = 0;
	LONG pointCount = 0;
	if(!file->ReadLong(&version)) return file->GetError();

	if(version < FILE_VERSION) return FILEERROR_READ;

	if(!file->ReadLong(&polyCount)) return file->GetError();
	if(!file->ReadLong(&pointCount)) return file->GetError();

	PolygonObject *pPoly = PolygonObject::Alloc(pointCount,polyCount);
	if(!pPoly) return FILEERROR_OUTOFMEMORY;

	LONG a;
	CPolygon *pPolys = pPoly->GetPolygonW();
	for(a=0;a<polyCount;a++)
	{
		CPolygon &p = pPolys[a];
		if(!file->ReadLong(&p.a)) return file->GetError();
		if(!file->ReadLong(&p.b)) return file->GetError();
		if(!file->ReadLong(&p.c)) return file->GetError();
		if(!file->ReadLong(&p.d)) return file->GetError();
	}

	Vector *pPoints = pPoly->GetPointW();
	for(a=0;a<pointCount;a++)
	{
		//Using real instead of vector to match the Melange library
		if(!file->ReadLReal(&pPoints[a].x)) return file->GetError();
		if(!file->ReadLReal(&pPoints[a].y)) return file->GetError();
		if(!file->ReadLReal(&pPoints[a].z)) return file->GetError();
	}

	doc->InsertObject(pPoly,NULL,NULL);

	SculptObject *pSculptObject = MakeSculptObject(pPoly,doc);
	if(!pSculptObject) return FILEERROR_INVALID;

	LONG subdivisionCount = 0;
	if(!file->ReadLong(&subdivisionCount)) return file->GetError();

	for(a=0;a<=subdivisionCount;a++)
	{
		SculptLayer *pBaseLayer = pSculptObject->GetBaseLayer();
		if(!pBaseLayer) return FILEERROR_OUTOFMEMORY;
		{
			LONG level, layerPointCount;
			if(!file->ReadLong(&level)) return file->GetError();
			if(!file->ReadLong(&layerPointCount)) return file->GetError();

			if(level != a) return FILEERROR_INVALID;
			if(layerPointCount != pBaseLayer->GetPointCount()) return FILEERROR_INVALID;

			//First read in the baselayer data for this level
			LONG b;
			for(b=0;b<layerPointCount;b++)
			{
				Vector p;
				//Using Real instead of Vector to match the Melange Library
				if(!file->ReadLReal(&p.x)) return file->GetError();
				if(!file->ReadLReal(&p.y)) return file->GetError();
				if(!file->ReadLReal(&p.z)) return file->GetError();
				pBaseLayer->SetOffset(b,p);
			}

			LONG hasMask=FALSE;
			if(!file->ReadLong(&hasMask)) return file->GetError();
			if(hasMask)
			{
				for(b=0;b<layerPointCount;b++)
				{
					SReal mask = 0;
					if(!file->ReadSReal(&mask)) return file->GetError();
					pBaseLayer->SetMask(b,mask);
				}
			}
		}

		LONG layerCount = 0;
		if(!file->ReadLong(&layerCount)) return file->GetError();

		LONG lay = 0;
		for(lay=0;lay<layerCount;lay++)
		{
			LONG type;
			if(!file->ReadLong(&type)) return file->GetError();
			if(type == SCULPT_FOLDER_ID)
			{

			}
			else if(type == SCULPT_LAYER_ID)
			{
				LONG level;
				LONG layerPointCount = 0;
				if(!file->ReadLong(&level)) return file->GetError();
				if(!file->ReadLong(&layerPointCount)) return file->GetError();
				
				String layerName;
				if(!file->ReadString(&layerName)) return file->GetError();

				LONG visible = TRUE;
				if(!file->ReadLong(&visible)) return file->GetError();

				Real strength = 1.0;
				if(!file->ReadLReal(&strength)) return file->GetError();

				if(level != pSculptObject->GetCurrentLevel())
				{
					GeAssert(FALSE);
					return FILEERROR_INVALID;
				}

				SculptLayer *pLayer = pSculptObject->AddLayer();
				if(!pLayer) return FILEERROR_OUTOFMEMORY;

				pLayer->SetName(layerName);
				pLayer->SetStrength(strength);
				pLayer->SetVisible(visible);

				LONG actualLayerPC =  pLayer->GetPointCount();
				if(actualLayerPC != layerPointCount)
				{
					GeAssert(false);
					return FILEERROR_INVALID;
				}

				LONG b;
				for(b=0;b<layerPointCount;b++)
				{
					Vector p;
					//Using Real instead of Vector to match the Melange Library
					if(!file->ReadLReal(&p.x)) return file->GetError();
					if(!file->ReadLReal(&p.y)) return file->GetError();
					if(!file->ReadLReal(&p.z)) return file->GetError();
					pLayer->SetOffset(b,p);
				}

				LONG hasMask=FALSE;
				if(!file->ReadLong(&hasMask)) return file->GetError();
				if(hasMask)
				{
					for(b=0;b<layerPointCount;b++)
					{
						SReal mask = 0;
						if(!file->ReadSReal(&mask)) return file->GetError();
						pLayer->SetMask(b,mask);
					}

					LONG maskEnabled = FALSE;
					if(!file->ReadLong(&maskEnabled)) return file->GetError();
					pLayer->SetMaskEnabled(maskEnabled);
				}
			}
		}

		pSculptObject->Update();

		if(a!=subdivisionCount)
			pSculptObject->Subdivide();
	}

	file->Close();
	return file->GetError();
}

FILEERROR SculptSaverData::Save(BaseSceneSaver *node, const Filename &name, BaseDocument *doc, SCENEFILTER flags)
{
	if (!(flags&SCENEFILTER_OBJECTS)) return FILEERROR_NONE;

	AutoAlloc<BaseFile> file;
	if(!file) return FILEERROR_OUTOFMEMORY;
	if (!file->Open(name,FILEOPEN_WRITE,FILEDIALOG_NONE,BYTEORDER_INTEL)) return file->GetError();

	if(!file->WriteLong(FILE_VERSION)) return file->GetError();

	SculptObject *pSculpt = GetSelectedSculptObject(doc);
	if(pSculpt)
	{
		PolygonObject *pBaseMesh = pSculpt->GetOriginalObject();
		if(pBaseMesh)
		{
			LONG polyCount = pBaseMesh->GetPolygonCount();
			LONG pointCount = pBaseMesh->GetPointCount();
			if(!file->WriteLong(polyCount)) return file->GetError();
			if(!file->WriteLong(pointCount)) return file->GetError();

			if (polyCount>0)
			{
				const CPolygon *pPolygons = pBaseMesh->GetPolygonR();
				LONG a;
				for(a=0;a<polyCount;a++)
				{
					if(!file->WriteLong(pPolygons[a].a)) return file->GetError();
					if(!file->WriteLong(pPolygons[a].b)) return file->GetError();
					if(!file->WriteLong(pPolygons[a].c)) return file->GetError();
					if(!file->WriteLong(pPolygons[a].d)) return file->GetError();
				}
			}

			if(pointCount > 0)
			{
				const Vector *pPoints = pBaseMesh->GetPointR();
				LONG a;
				for(a=0;a<pointCount;a++)
				{
					//Using Real instead of Vector to match the Melange Library
					if(!file->WriteLReal(pPoints[a].x)) return file->GetError();
					if(!file->WriteLReal(pPoints[a].y)) return file->GetError();
					if(!file->WriteLReal(pPoints[a].z)) return file->GetError();
				}
			}
		}

		LONG subdivisionCount = pSculpt->GetSubdivisionCount();
		if(!file->WriteLong(subdivisionCount)) return file->GetError();

		LONG a;
		for(a=0;a<=subdivisionCount;a++)
		{
			SculptLayer *pBaseLayer = pSculpt->GetBaseLayer();
			if(pBaseLayer)
			{
				SculptLayerData *pLayerData = pBaseLayer->GetSculptLayerData();
				while(pLayerData && pLayerData->GetSubdivisionLevel() != a)
				{
					pLayerData = (SculptLayerData*)pLayerData->GetNext();
				}
				if(pLayerData)
				{
					LONG pointCount = pLayerData->GetPointCount();
					if(!file->WriteLong(a)) return file->GetError();
					if(!file->WriteLong(pointCount)) return file->GetError();
					LONG b;
					for(b=0;b<pointCount;b++)
					{
						Vector offset;
						pLayerData->GetOffset(b,offset);

						//Using real instead of vector to match the melange library
						if(!file->WriteLReal(offset.x)) return file->GetError();
						if(!file->WriteLReal(offset.y)) return file->GetError();
						if(!file->WriteLReal(offset.z)) return file->GetError();
					}

					LONG hasMask = pLayerData->HasMask();
					if(!file->WriteLong(hasMask)) return file->GetError();
					if(hasMask)
					{
						for(b=0;b<pointCount;b++)
						{
							SReal mask = 0;
							pLayerData->GetMask(b,mask);
							if(!file->WriteSReal(mask)) return file->GetError();
						}
					}
				}
			}

			//Save data for all other layers that have information at this level
			//First count how many layers we have at this level (currently not recursing to children but this should be done).
			LONG layerCount = 0;
			SculptLayerBase *pLayer = pSculpt->GetFirstLayer();
			while(pLayer)
			{
				LONG type = pLayer->GetType();
				if(type == SCULPT_LAYER_ID)
				{
					SculptLayer *pRealLayer = (SculptLayer *)pLayer;
					Bool isBaseLayer = pRealLayer->IsBaseLayer();
					SculptLayerData *pData = pRealLayer->GetSculptLayerData();
					if(!isBaseLayer && pData && pData->GetSubdivisionLevel() == a)
					{
						layerCount++;
					}
				}
				pLayer = (SculptLayerBase *)pLayer->GetNext();
			}

			//Write out how many layers we have at this level
			if(!file->WriteLong(layerCount)) return file->GetError();

			//Write out the layer data
			pLayer = pSculpt->GetFirstLayer();
			while(pLayer)
			{
				LONG type = pLayer->GetType();
				if(type == SCULPT_LAYER_ID)
				{
					SculptLayer *pRealLayer = (SculptLayer *)pLayer;
					Bool isBaseLayer = pRealLayer->IsBaseLayer();
					SculptLayerData *pData = pRealLayer->GetSculptLayerData();
					if(!isBaseLayer && pData && pData->GetSubdivisionLevel() == a)
					{
						LONG level = pData->GetSubdivisionLevel();
						LONG pointCount = pData->GetPointCount();

						if(!file->WriteLong(type)) return file->GetError();
						if(!file->WriteLong(level)) return file->GetError();
						if(!file->WriteLong(pointCount)) return file->GetError();
						if(!file->WriteString(pRealLayer->GetName())) return file->GetError();

						LONG visible = pRealLayer->IsVisible();
						if(!file->WriteLong(visible)) return file->GetError();

						Real strength= pRealLayer->GetStrength();
						if(!file->WriteLReal(strength)) return file->GetError();

						LONG b;
						for(b=0;b<pointCount;b++)
						{
							Vector offset;
							pData->GetOffset(b,offset);

							//Using real instead of vector to match the melange library
							if(!file->WriteLReal(offset.x)) return file->GetError();
							if(!file->WriteLReal(offset.y)) return file->GetError();
							if(!file->WriteLReal(offset.z)) return file->GetError();
						}

						LONG hasMask = pData->HasMask();
						if(!file->WriteLong(hasMask)) return file->GetError();

						if(hasMask)
						{
							for(b=0;b<pointCount;b++)
							{
								SReal mask = 0;
								pData->GetMask(b,mask);
								if(!file->WriteSReal(mask)) return file->GetError();
							}
							LONG maskEnabled = pRealLayer->IsMaskEnabled();
							if(!file->WriteLong(maskEnabled)) return file->GetError();
						}
					}
				}
				pLayer = (SculptLayerBase *)pLayer->GetNext();
			}
		}
	}
	file->Close();
	return file->GetError();
}

Bool RegisterSculpt(void)
{
	String name=GeLoadString(IDS_SCULPT);
	if (!RegisterSceneLoaderPlugin(1027977,name,0,SculptLoaderData::Alloc,"",NULL)) return FALSE;
	if (!RegisterSceneSaverPlugin(1027978 ,name,0,SculptSaverData::Alloc,"","scp")) return FALSE;
	return TRUE;
}
