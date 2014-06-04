
#include "c4d.h"
#include "c4d_graphview.h"

#include "dexample.h"

#define DATATYPE_DEFAULTLONG		1000
#define DATATYPE_DEFAULTSTRING	1001

CustomProperty datatypeprops[] =
{
	{ CUSTOMTYPE_LONG,   DATATYPE_DEFAULTLONG, "DEFAULTLONG" },
	{ CUSTOMTYPE_STRING, DATATYPE_DEFAULTLONG, "DEFAULTSTRING" },
	{	CUSTOMTYPE_END, 0, NULL }	
};

class ExampleDataType
{
};
 
class iExampleDataType : public iCustomDataType<ExampleDataType>
{
	friend class ExampleDataTypeClass;

		LONG		ldata;
		String	sdata;
		Vector  vdata;

	public:

		iExampleDataType(void)
		{
			ldata = 0;
		}
};

#define CUSTOMDATATYPE_EXAMPLE		123123123

LONG convfrom[] =
{
	ID_GV_VALUE_TYPE_STRING,//DA_STRING,
	ID_GV_VALUE_TYPE_INTEGER,//DA_LONG,
	CUSTOMDATATYPE_EXAMPLE,
	CUSTOMDATATYPE_SPLINE,
	400006000,//ID_GV_VALUE_TYPE_GENERAL_OBJECT
};

LONG convto[] =
{
	ID_GV_VALUE_TYPE_STRING,//	DA_STRING,
	ID_GV_VALUE_TYPE_INTEGER,//DA_LONG,
//	CUSTOMDATATYPE_SPLINE,
	400006000,
};

struct GvObject
{
	BaseList2D*	object;
	LONG				type;
};

class ExampleDataTypeClass : public CustomDataTypeClass
{
		INSTANCEOF(ExampleDataTypeClass,CustomDataTypeClass)

	public:

		virtual LONG GetId() 
		{ 
			return CUSTOMDATATYPE_EXAMPLE; 
		}

		virtual CustomDataType* AllocData()
		{
			return gNew iExampleDataType;
		};

		virtual void FreeData(CustomDataType* data)
		{
			iExampleDataType* d = (iExampleDataType*)data;
			gDelete(d);
		}

		virtual Bool CopyData(const CustomDataType* src,CustomDataType* dst,AliasTrans *aliastrans)
		{
			iExampleDataType* s = (iExampleDataType*)src;
			iExampleDataType* d = (iExampleDataType*)dst;
			if (!s || !d) return FALSE;

			d->ldata = s->ldata;
			d->sdata = s->sdata;
			d->vdata = s->vdata;

			return TRUE;
		}
		
		virtual LONG Compare(const CustomDataType* d1,const CustomDataType* d2)
		{
			iExampleDataType* s = (iExampleDataType*)d1;
			iExampleDataType* d = (iExampleDataType*)d2;
			if (!s || !d) return FALSE;

			if (s->ldata<d->ldata) return -1;
			if (s->ldata>d->ldata) return 1;

			if (s->vdata!=d->vdata) return 1;
			if (s->sdata!=d->sdata) return 1;

			return 0;
		}

		virtual Bool WriteData(const CustomDataType *t_d,HyperFile *hf)
		{
			iExampleDataType* d = (iExampleDataType*)t_d;

			hf->WriteLong(d->ldata);
			hf->WriteString(d->sdata);
			hf->WriteVector(d->vdata);

			return TRUE;
		}

		virtual Bool ReadData(CustomDataType *t_d,HyperFile *hf,LONG level)
		{
			iExampleDataType* d = (iExampleDataType*)t_d;

			if (level>0)
			{
				hf->ReadLong(&d->ldata);
				hf->ReadString(&d->sdata);
				hf->ReadVector(&d->vdata);
			}
			return TRUE;
		}

		virtual const CHAR *GetResourceSym()
		{
			return "ExampleDataType";
		}

		virtual CustomProperty*	GetProperties()
		{
			return datatypeprops;
		}

		virtual void GetDefaultProperties(BaseContainer &data) // fill default DESC_xxx values
		{
			data.SetLong(DESC_ANIMATE,DESC_ANIMATE_ON);
			data.SetLong(DATATYPE_DEFAULTLONG,1);
			data.SetString(DATATYPE_DEFAULTSTRING,"Hello World");
		}

		virtual LONG GetConversionsFrom(LONG *&table)
		{
			table = convfrom;
			return sizeof(convfrom)/sizeof(LONG);	
		}

		virtual GvError ConvertFromGv(LONG type,const void *const src,LONG cpu_id,CustomDataType *dst)
		{
			iExampleDataType *d = (iExampleDataType*)dst;
			switch (type)
			{
				case ID_GV_VALUE_TYPE_STRING:		d->ldata = ((String*)src)[cpu_id].ToLong(); d->sdata = ((String*)src)[cpu_id]; return GV_CALC_ERR_NONE;
				case ID_GV_VALUE_TYPE_INTEGER:	d->ldata = ((LONG*)src)[cpu_id]; d->sdata = LongToString(d->ldata); return GV_CALC_ERR_NONE;

				case CUSTOMDATATYPE_SPLINE:
					{
						SplineData *spline = (SplineData*)((GvHelper*)src)->data[cpu_id];
						if (!spline) return GV_CALC_ERR_UNDEFINED;
						d->ldata = spline->GetKnotCount();
						d->sdata = LongToString(d->ldata);
						return GV_CALC_ERR_NONE;
					}

				case 400006000:
					{
						GvObject *s = &((GvObject*)src)[cpu_id];
						if (s->object) d->sdata = s->object->GetName();
						else d->sdata = "<<no object>>";

						d->ldata = -1;

						return GV_CALC_ERR_NONE;					
					}
			}
			return SUPER::ConvertFromGv(type,src,cpu_id,dst);
		}

		virtual LONG GetConversionsTo(LONG *&table)
		{
			table = convto;
			return sizeof(convto)/sizeof(LONG);	
		}

		virtual GvError ConvertToGv(LONG type,const CustomDataType *src,void *dst,LONG cpu_id)
		{
			iExampleDataType *s = (iExampleDataType*)src;
			switch (type)
			{
				case ID_GV_VALUE_TYPE_STRING:		((String*)dst)[cpu_id] = s->sdata; return GV_CALC_ERR_NONE;
				case ID_GV_VALUE_TYPE_INTEGER:	((LONG*)dst)[cpu_id] = s->ldata; return GV_CALC_ERR_NONE;

				case CUSTOMDATATYPE_SPLINE:
					{
//						((Helper*)dst)->data[cpu_id] = NULL;
//						dst = GeData(CUSTOMDATATYPE_SPLINE,DEFAULTVALUE);
						return GV_CALC_ERR_NONE;
					}

				case 400006000:
					{
						((GvObject*)dst)[cpu_id].object = NULL;
						return GV_CALC_ERR_NONE;
					}
			}
			return SUPER::ConvertToGv(type,src,dst,cpu_id);
		}

		virtual GvValueFlags GetCalculationFlags()
		{
			return SUPER::GetCalculationFlags();
		}

		virtual GvError Calculate(LONG calculation,const CustomDataType *src1, const CustomDataType *src2, CustomDataType *dst, Real parm1)
		{
			return SUPER::Calculate(calculation,src1,src2,dst,parm1);
		}

		virtual Bool _GetDescription(const CustomDataType *data,Description &desc,DESCFLAGS_DESC &flags,const BaseContainer &parentdescription,DescID *singledescid)
		{
			Bool res = desc.LoadDescription(GetId());
			if (res)
			{
				flags |= DESCFLAGS_DESC_LOADED;
			}
			return SUPER::_GetDescription(data,desc,flags,parentdescription,singledescid);
		}

		virtual Bool GetParameter(const CustomDataType *data,const DescID &id,GeData &t_data,DESCFLAGS_GET &flags)
		{
			const iExampleDataType *s = (iExampleDataType*)data;
			if (id[0].id==EXAMPLE_LONG)
			{
				t_data = GeData(s->ldata);
				flags |= DESCFLAGS_GET_PARAM_GET;
			}
			else if (id[0].id==EXAMPLE_STRING)
			{
				t_data = GeData(s->sdata);
				flags |= DESCFLAGS_GET_PARAM_GET;
			}
			else if (id[0].id==EXAMPLE_VECTOR)
			{
				HandleDescGetVector(id,s->vdata,t_data,flags);
			}
			return SUPER::GetParameter(data,id,t_data,flags);
		}

		virtual Bool SetDParameter(CustomDataType *data,const DescID &id,const GeData &t_data,DESCFLAGS_SET &flags)
		{
			iExampleDataType *s = (iExampleDataType*)data;
			if (id[0].id==EXAMPLE_LONG)
			{
				s->ldata = t_data.GetLong();
				s->sdata = LongToString(s->ldata);
				flags |= DESCFLAGS_SET_PARAM_SET;
			}
			else if (id[0].id==EXAMPLE_STRING)
			{
				s->sdata = t_data.GetString();
				s->ldata = s->sdata.ToLong();
				flags |= DESCFLAGS_SET_PARAM_SET;
			}
			else if (id[0].id==EXAMPLE_VECTOR)
			{
				HandleDescSetVector(s->vdata,id,s->vdata,t_data,flags);
			}
			return SUPER::SetDParameter(data,id,t_data,flags);
		}

		virtual Bool GetEnabling(const CustomDataType *data,const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE &flags,const BaseContainer *itemdesc)
		{ 
			return SUPER::GetEnabling(data,id,t_data,flags,itemdesc);
		}
};


Bool RegisterExampleDataType(void)
{
	RegisterDescription(CUSTOMDATATYPE_EXAMPLE,"Dexample");

	if (!RegisterCustomDataTypePlugin(
				"C++ SDK ExampleDataType", 
					CUSTOMDATATYPE_INFO_LOADSAVE |
					CUSTOMDATATYPE_INFO_TOGGLEDISPLAY|
					CUSTOMDATATYPE_INFO_HASSUBDESCRIPTION|
						//CUSTOMDATATYPE_INFO_NEEDDATAFORSUBDESC|
							0,
				gNew ExampleDataTypeClass,
				1))
					return FALSE;

	return TRUE;
}
