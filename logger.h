#pragma once
#include <c4d.h>
class Logger
{
public:
	Logger(void);
	~Logger(void);
	static void Init();
	static void Destroy();
	static void AddLine(const char *line,int indent = 0);

	static char *m_filename;
};

