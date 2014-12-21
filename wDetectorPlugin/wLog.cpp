#include "wLog.h"
#include <fstream>

bool wLog(int type, wchar_t* text)
{
	std::wofstream log;
	wchar_t* type0;

	log.open(L"wDetector.log", std::ofstream::out | std::ofstream::app);
	if (!log.is_open())
		return false;

	if (type == LOG_INFO)
		type0 = L"INFO: ";
	else if (type == LOG_ERROR)
		type0 = L"ERROR: ";
	else
	{
		log.close();
		return false;
	}

	log << type0 << text << std::endl;

	log.close();

	return true;
}