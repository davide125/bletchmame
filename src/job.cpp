/***************************************************************************

    job.cpp

    Wrappers for Win32 jobs, so we properly clean up

***************************************************************************/

#ifdef WIN32
#include <windows.h>
#endif

#include "job.h"


//**************************************************************************
//  WIN32 IMPLEMENTATION
//**************************************************************************

#ifdef WIN32

//-------------------------------------------------
//  ctor
//-------------------------------------------------

Win32Job::Win32Job()
{
	m_handle = CreateJobObjectW(nullptr, nullptr);
	AssignProcessToJobObject(m_handle, GetCurrentProcess());

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
	info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	SetInformationJobObject(m_handle, JobObjectExtendedLimitInformation, &info, sizeof(info));
}


//-------------------------------------------------
//  AddProcess
//-------------------------------------------------

void Win32Job::AddProcess(Q_PID process_id)
{
	HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id->dwProcessId);
	AssignProcessToJobObject(m_handle, process_handle);
	CloseHandle(process_handle);
}
#endif // WIN32
