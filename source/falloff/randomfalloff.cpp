/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2006 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// this example demonstrates how to implement a new falloff type

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_falloffplugin.h"
#include "ofalloff_random.h"

class RandomFalloff : public FalloffData
{
public:
	Random rnd;
	
	virtual Bool Init(FalloffDataData &falldata,BaseContainer *bc); 
	virtual Bool InitFalloff(BaseContainer *bc, FalloffDataData &falldata);
	virtual void Sample(const Vector &p, const FalloffDataData &data, Real *res);

	static FalloffData *Alloc(void) { return gNew RandomFalloff; }
};

Bool RandomFalloff::Init(FalloffDataData &falldata,BaseContainer *bc)
{
	if (!bc) return FALSE;
	
	if (bc->GetData(RANDOMFALLOFF_SEED).GetType()==DA_NIL)
		bc->SetLong(RANDOMFALLOFF_SEED,1234567);

	return TRUE;
}

Bool RandomFalloff::InitFalloff(BaseContainer *bc, FalloffDataData &falldata)
{
	if (!bc) return FALSE;
	
	rnd.Init(bc->GetLong(RANDOMFALLOFF_SEED));

	return TRUE;
}
 
void RandomFalloff::Sample(const Vector &p, const FalloffDataData &data, Real *res)
{
	(*res) = rnd.Get01();
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_RANDOMFALLOFF 1019569

Bool RegisterRandomFalloff(void)
{
	return RegisterFalloffPlugin(ID_RANDOMFALLOFF,GeLoadString(IDS_RANDOMFALLOFF),0,RandomFalloff::Alloc,"ofalloff_random");
}