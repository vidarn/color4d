/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for a menu plugin and multiprocessing

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_memory_mp.h"


class MPTest : public C4DThread
{
	public:
		Random	rnd;
		LONG		start;
		Real		result;
		Real		duration;
		
		virtual void Main(void);
		virtual const CHAR *GetThreadName(void) { return "SDK MpTest"; }
};

void MPTest::Main(void)
{
	LReal t1=GeGetMilliSeconds();

	// calculate the 10,000,000 th random number

	rnd.Init(start);

	LONG i;
	for (i=0; i<10000000; i++)
		rnd.Get01();

	result=rnd.Get01();
	duration = GeGetMilliSeconds() - t1;
}

class ControlThread : public C4DThread
{
	public:
		virtual void Main(void);
		virtual const CHAR *GetThreadName(void) { return "SDK ControlThread"; }
};

void ControlThread::Main(void)
{
	GeShowMouse(MOUSE_BUSY);

	MPThreadPool	mp;
	LONG			i,cnt=GeGetCPUCount(); 
	MPAlloc<MPTest> thread;
	AutoGeFree<C4DThread*> list = GeAllocType(C4DThread*,cnt);
	if (!list || !thread.Init(cnt))
		return;
		
	for (i=0; i<cnt; i++)
	{
		thread[i].start=i;
		thread[i].result=0.0;
		list[i]=&thread[i];
	}
	
	if (!mp.Init(*this,cnt,list)) return;
	LReal start_time=GeGetMilliSeconds();
	if (!mp.Start(THREADPRIORITY_BELOW)) return;
	LReal	start_duration = GeGetMilliSeconds()-start_time;

	mp.Wait();
	LReal total_duration = GeGetMilliSeconds()-start_time;
	
	String str="Multiprocessing Test on "+LongToString(cnt)+" CPUs returns:";
	for (i=0; i<cnt; i++)
		str+=" "+RealToString(thread[i].result);

	str += "| Duration per thread:";
	for (i=0; i<cnt; i++)
	{
		str+=" "+RealToString(thread[i].duration)+" ms";
		if ( i+1<cnt)
			str+=",";
	}

	str += "|Thread start: "+RealToString(start_duration)+" ms";
	str += "|Total duration: "+RealToString(total_duration)+" ms";

	GeShowMouse(MOUSE_NORMAL);

	MessageDialog(str);
}

class MenuTest : public CommandData
{
	public:
		virtual Bool Execute(BaseDocument *doc);
};

Bool MenuTest::Execute(BaseDocument *doc)
{
	ControlThread ct;
	return ct.Start(THREADMODE_SYNCHRONOUS); // multiprocessing test
}

Bool RegisterMenuTest(void)
{
	// be sure to use a unique ID obtained from www.plugincafe.com
	return RegisterCommandPlugin(1000956,GeLoadString(IDS_MENUTEST),0,AutoBitmap("icon.tif"),String("C++ SDK Menu Test Plugin"),gNew MenuTest);
}

