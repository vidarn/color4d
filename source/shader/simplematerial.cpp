/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example for an easy implementation of a volume (surface) shader
// It modifies the preview scenes and adds its own scene

#include "c4d.h"
#include "c4d_symbols.h"
#include "msimplematerial.h"
#include "customgui_matpreview.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_SIMPLEMAT 1001164

class SimpleMaterial : public MaterialData
{
	INSTANCEOF(SimpleMaterial,MaterialData)

	private:
		Vector color;
	public:
		virtual Bool Init		(GeListNode *node);
		virtual	void CalcSurface			(BaseMaterial *mat, VolumeData *vd);
		virtual	INITRENDERRESULT InitRender(BaseMaterial *mat, const InitRenderStruct &irs);
    virtual Bool GetDParameter(GeListNode *node, const DescID &id,GeData &t_data,DESCFLAGS_GET &flags);
    virtual Bool SetDParameter(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_SET &flags);
    virtual Bool Message(GeListNode* node, LONG type, void* data);
    virtual Bool CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, COPYFLAGS flags, AliasTrans *trn);

		static NodeData *Alloc(void) { return gNew SimpleMaterial; }

    LONG updatecount;
};

Bool SimpleMaterial::Init(GeListNode *node)
{
	BaseContainer *data=((BaseMaterial*)node)->GetDataInstance();
	data->SetVector(SIMPLEMATERIAL_COLOR,1.0);
  updatecount = 0;

  GeData previewData(CUSTOMDATATYPE_MATPREVIEW, DEFAULTVALUE);
  MaterialPreviewData* preview = (MaterialPreviewData*)previewData.GetCustomDataType(CUSTOMDATATYPE_MATPREVIEW);
  if (!preview)
    return FALSE;
  data->SetData(SIMPLEMATERIAL_MAT_PREVIEW, previewData);

  return TRUE;
}

INITRENDERRESULT SimpleMaterial::InitRender(BaseMaterial *mat, const InitRenderStruct &irs)
{
	BaseContainer *data=mat->GetDataInstance();
	color = data->GetVector(SIMPLEMATERIAL_COLOR);
	return INITRENDERRESULT_OK;
}

static void SimpleIllumModel(VolumeData *sd, RayLightCache *rlc, void *dat)
{
	Bool						nodif,nospec;
	LONG						i;
	Real						cosa,cosb,exponent=5.0;
	const LVector	&v=sd->ray->v;

	rlc->diffuse=rlc->specular=0.0;

	for (i=0; i<rlc->cnt; i++)
	{
		RayLightComponent *lc=rlc->comp[i];
		if (lc->lv==0.0) continue; // light invisible

		RayLight *ls=lc->light;

		nodif=nospec=FALSE;
		if (ls->lr.object) CalcRestrictionInc(&ls->lr,sd->op,nodif,nospec);

		lc->rdiffuse=lc->rspecular=0.0;
		if (ls->ambient)
			lc->rdiffuse = 1.0;
		else if (ls->arealight)
			sd->CalcArea(ls,nodif,nospec,exponent,v,sd->p,sd->bumpn,sd->orign,sd->raybits,&lc->rdiffuse,&lc->rspecular);
		else
		{
			cosa=sd->bumpn*lc->lv;
			if (!(ls->nodiffuse || nodif) && sd->cosc*cosa>=0.0)
			{
				Real trn=ls->trn;
				if (trn!=1.0)
					lc->rdiffuse = Pow(Abs(cosa),trn);
				else
					lc->rdiffuse = Abs(cosa);
			}

			if (!(ls->nospecular || nospec))
			{
				cosb=v * (lc->lv - sd->bumpn*(2.0 * cosa));

				if (cosb>0.0)
					lc->rspecular = Pow(cosb,exponent);
			}
		}

		rlc->diffuse  += lc->rdiffuse^lc->col;
		rlc->specular += lc->rspecular^lc->col;
	}
}

void SimpleMaterial::CalcSurface(BaseMaterial *mat, VolumeData *vd)
{
	Vector diff,spec,att_spc,att_dif;
	LONG i;

	//sd->Illuminance1(&diff,&spec,5.0);
	vd->IlluminanceSimple(&diff,&spec,0.0,SimpleIllumModel,this); // replace standard model by custom model

	att_spc = 0.5+0.5*Turbulence(vd->uvw*2.5,4.0,TRUE);
	att_dif = att_spc^color;

	vd->col = (att_dif^(diff+vd->ambient)) + (att_spc^spec);

	// process multipass data
	Multipass *buf = vd->multipass;
	if (!buf) return;

	*buf->vp_mat_color					= att_dif;
	*buf->vp_mat_specularcolor	= att_spc;
	*buf->vp_mat_specular				= 0.4; // 2.0/exponent (or similar value)

	// values have only to be filled if != 0.0
	// *buf->vp_mat_luminance			= 0.0;
	// *buf->vp_mat_environment		= 0.0;
	// *buf->vp_mat_transparency		= 0.0;
	// *buf->vp_mat_reflection			= 0.0;
	// *buf->vp_mat_diffusion			= 0.0;

	// calculate ambient component
	*buf->vp_ambient = att_dif^vd->ambient;

	// attenuate diffuse components
	for (i=0; i<buf->diffuse_cnt; i++)
		*buf->diffuse[i] = att_dif^(*buf->diffuse[i]);

	// attenuate specular components
	for (i=0; i<buf->specular_cnt; i++)
		*buf->specular[i] = att_spc^(*buf->specular[i]);
}


Bool SimpleMaterial::GetDParameter(GeListNode *node, const DescID &id,GeData &t_data,DESCFLAGS_GET &flags)
{
  BaseContainer *data = ((BaseMaterial*)node)->GetDataInstance();


  switch (id[0].id)
	{
    case SIMPLEMATERIAL_MAT_PREVIEW:
		  return GetDParameterPreview(data,(GeData*)&t_data,flags,SIMPLEMATERIAL_MAT_PREVIEW,updatecount,(BaseMaterial*)node);
      break;
	}

  return MaterialData::GetDParameter(node, id, t_data, flags);
}

Bool SimpleMaterial::SetDParameter(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_SET &flags)
{
  BaseContainer *data = ((BaseMaterial*)node)->GetDataInstance();

  updatecount++;

  switch (id[0].id)
	{
	case SIMPLEMATERIAL_MAT_PREVIEW:
		return SetDParameterPreview(data,&t_data,flags,SIMPLEMATERIAL_MAT_PREVIEW);
      break;
  }

  return MaterialData::SetDParameter(node, id, t_data, flags);
}

Bool SimpleMaterial::Message(GeListNode* node, LONG type, void* data)
{
  if (type == MSG_UPDATE)
    updatecount++;

	switch (type)
	{
	case MATPREVIEW_GET_OBJECT_INFO:
		{
      MatPreviewObjectInfo* info = (MatPreviewObjectInfo*)data;
			info->bHandlePreview = TRUE; // own preview handling
			info->bNeedsOwnScene = TRUE; // we need our own entry in the preview scene cache
      info->bNoStandardScene = FALSE; // we modify the standard scene
      info->lFlags = 0;
			return TRUE;
		}
		break;
	case MATPREVIEW_MODIFY_CACHE_SCENE:
    // modify the preview scene here. We have a pointer to a scene inside the preview scene cache.
    // our scene contains the object
		{
      MatPreviewModifyCacheScene* scene = (MatPreviewModifyCacheScene*)data;
      // get the striped plane from the preview
      BaseObject* plane = scene->pDoc->SearchObject("Polygon");
      if (plane)
        plane->SetRelScale(0.1); // scale it a bit
			return TRUE;
		}
		break;
	case MATPREVIEW_PREPARE_SCENE:
    // let the preview handle the rest...
		return TRUE;
		break;
	case MATPREVIEW_GENERATE_IMAGE:
		{
		  MatPreviewGenerateImage* image = (MatPreviewGenerateImage*)data;
	      if (image->pDoc)
		  {
            LONG w = image->pDest->GetBw();
            LONG h = image->pDest->GetBh();
            BaseContainer bcRender = image->pDoc->GetActiveRenderData()->GetData();
            bcRender.SetReal(RDATA_XRES, w);
            bcRender.SetReal(RDATA_YRES, h);
            bcRender.SetLong(RDATA_ANTIALIASING, ANTI_GEOMETRY);
            if (image->bLowQuality)
              bcRender.SetBool(RDATA_RENDERENGINE, RDATA_RENDERENGINE_PREVIEWSOFTWARE);
            image->pDest->Clear(0, 0, 0);
            image->lResult = RenderDocument(image->pDoc, bcRender, NULL, NULL, image->pDest,
            RENDERFLAGS_EXTERNAL | RENDERFLAGS_PREVIEWRENDER, image->pThread);
          }
	      return TRUE;
		}
		break;
  case MATPREVIEW_GET_PREVIEW_ID:
    *((LONG*)data) = SIMPLEMATERIAL_MAT_PREVIEW;
    return TRUE;
	}

  return TRUE;
}

Bool SimpleMaterial::CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, COPYFLAGS flags, AliasTrans *trn)
{
  ((SimpleMaterial*)dest)->updatecount = updatecount;
  return NodeData::CopyTo(dest, snode, dnode, flags, trn);
}


Bool RegisterSimpleMaterial(void)
{
  String name = GeGetDefaultFilename(DEFAULTFILENAME_SHADER_VOLUME)+GeLoadString(IDS_SIMPLEMATERIAL); // place in default Shader section

  // add a preview scene that can only be selected in the Simple Material's preview
  AddUserPreviewScene(GeGetPluginPath() + String("res") + String("scene") + String("Stairs.c4d"), ID_SIMPLEMAT, NULL);

	return RegisterMaterialPlugin(ID_SIMPLEMAT,name,0,SimpleMaterial::Alloc,"Msimplematerial",0);
}
