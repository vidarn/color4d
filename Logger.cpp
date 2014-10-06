#include "logger.h"

char *Logger::m_filename = 0;

Logger::Logger(void)
{
}


Logger::~Logger(void)
{
}

void Logger::Init()
{
	Filename dir = GeGetStartupWritePath();
	dir += Filename(String("color4d_log.txt"));
	String filename = dir.GetString();
#ifdef _WINDOWS
	Int32 fnLength =  filename.GetCStringLen(STRINGENCODING_XBIT);
	m_filename = NewMem(char,fnLength+1);
	filename.GetCString(m_filename, fnLength+1, STRINGENCODING_XBIT);
#else
	Int32 fnLength =  filename.GetCStringLen(STRINGENCODING_UTF8);
	m_filename = NewMem(char,fnLength+1);
	filename.GetCString(m_filename, fnLength+1, STRINGENCODING_UTF8);
#endif
	FILE *f = fopen(m_filename,"wt");
	if(f){
		fprintf(f,"----- Color4D Log -----\n");
		fclose(f);
	}
}

void Logger::Destroy()
{
	DeleteMem(m_filename);
}

void Logger::AddLine(const char *line,int indent)
{
	FILE *f = fopen(m_filename,"at");
	if(f){
		for(int i=0;i<indent;i++){
			fprintf(f,"    ");
		}
		fprintf(f,"%s\n",line);
		fclose(f);
	}
}