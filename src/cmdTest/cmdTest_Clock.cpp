//-----------------------------------------------------------------------------
// Created on: 27 June 2022
// Created by: Andrey Voevodin
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// cmdTest includes
#include <cmdTest.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// OCCT includes
#include <OSD.hxx>
#include <OSD_Chronometer.hxx>
#include <OSD_Environment.hxx>
#include <OSD_Timer.hxx>

//-----------------------------------------------------------------------------

#ifdef _WIN32
  #include <windows.h>
  #include <winbase.h>
  #include <process.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>
  #include <limits>

  #define RLIM_INFINITY   0x7fffffff

  static clock_t CPU_CURRENT; // cpu time already used at last
                              // cpulimit call. (sec.) 

  static HANDLE hThread;
#else /* _WIN32 */
  #include <sys/resource.h>
  #include <signal.h>
  #include <unistd.h>

  #if defined (__hpux) || defined ( HPUX )
  #define RLIM_INFINITY   0x7fffffff
  #define	RLIMIT_CPU	0

  static pthread_t cpulimitThread;
  #endif
#endif /* _WIN32 */

static bool    isTestThreadStoped = true;
static clock_t CPU_LIMIT;   // Cpu_limit in Sec.
static OSD_Timer aTimer;

//-----------------------------------------------------------------------------

#ifdef _WIN32
static unsigned int __stdcall CpuFunc (void * /*param*/)
{
  clock_t anElapCurrent;
  clock_t aCurrent;

  for(;;)
  {
    if (isTestThreadStoped)
    {
      break;
    }

    Sleep (5);
    Standard_Real anUserSeconds, aSystemSeconds;
    OSD_Chronometer::GetProcessCPU (anUserSeconds, aSystemSeconds);
    aCurrent = clock_t(anUserSeconds + aSystemSeconds);
    anElapCurrent = clock_t(aTimer.ElapsedTime());
    
    if (CPU_LIMIT > 0 && (aCurrent - CPU_CURRENT) >= CPU_LIMIT)
    {
      aTimer.Stop();
      if (IsDebuggerPresent())
      {
        std::cout << "Info: CPU limit (" << CPU_LIMIT << " sec) has been reached but ignored because of attached Debugger" << std::endl;
        return 0;
      }
      else
      {
        std::cout << "ERROR: Process killed by CPU limit (" << CPU_LIMIT << " sec)" << std::endl;
        ExitProcess (2);
      }
    }
    if (CPU_LIMIT > 0 && anElapCurrent >= CPU_LIMIT)
    {
      aTimer.Stop();
      if (IsDebuggerPresent())
      {
        std::cout << "Info: Elapsed limit (" << CPU_LIMIT << " sec) has been reached but ignored because of attached Debugger" << std::endl;
        return 0;
      }
      else
      {
        std::cout << "ERROR: Process killed by elapsed limit (" << CPU_LIMIT << " sec)" << std::endl;
        ExitProcess (2);
      }
    }
  }

  aTimer.Stop();
  isTestThreadStoped = false;
  _endthreadex(0);
  return 0;
}
#else
static void cpulimitSignalHandler (int)
{
  std::cout << "Process killed by CPU limit  (" << CPU_LIMIT << " sec)" << std::endl;
  exit(2);
}
static void *CpuFunc(void* /*threadarg*/)
{
  clock_t anElapCurrent;
  for(;;)
  {
    if (isTestThreadStoped)
    {
      break;
    }

    sleep (5);
    anElapCurrent = clock_t(aTimer.ElapsedTime());
    if (CPU_LIMIT >0 && (anElapCurrent) >= CPU_LIMIT) {
      aTimer.Stop();
      std::cout << "Process killed by elapsed limit  (" << CPU_LIMIT << " sec)" << std::endl;
      exit(2);
    }
  }
  aTimer.Stop();
  pthread_exit(NULL);
  return NULL;
}
#endif

//-----------------------------------------------------------------------------

// Returns time in seconds defined by the argument string,
// multiplied by factor defined in environment variable
// CSF_CPULIMIT_FACTOR (if it exists, 1 otherwise)
static clock_t GetCpuLimit(const Standard_CString theParam)
{
  clock_t aValue = std::atoi(theParam);

  OSD_Environment aEnv("CSF_CPULIMIT_FACTOR");
  TCollection_AsciiString aEnvStr = aEnv.Value();
  if (!aEnvStr.IsEmpty())
  {
    aValue *= std::atoi(aEnvStr.ToCString());
  }
  return aValue;
}

//-----------------------------------------------------------------------------

int TEST_CPULimit(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  isTestThreadStoped = false;

#ifdef _WIN32
  // Windows specific code
  unsigned int __stdcall CpuFunc (void *);
  unsigned aThreadID;

  if (argc <= 1){
    CPU_LIMIT = RLIM_INFINITY;
  } else {
    CPU_LIMIT = GetCpuLimit (argv[1]);
    Standard_Real anUserSeconds, aSystemSeconds;
    OSD_Chronometer::GetProcessCPU (anUserSeconds, aSystemSeconds);
    CPU_CURRENT = clock_t(anUserSeconds + aSystemSeconds);
    aTimer.Reset();
    aTimer.Start();
    if (hThread == nullptr)
    {
      hThread = (HANDLE)_beginthreadex(NULL, 0, CpuFunc, NULL, 0, &aThreadID);
    }
  }

#else 
  // Unix & Linux
  rlimit rlp;
  rlp.rlim_max = RLIM_INFINITY;
  if (n <= 1)
    rlp.rlim_cur = RLIM_INFINITY;
  else
    rlp.rlim_cur = GetCpuLimit (a[1]);
  CPU_LIMIT = rlp.rlim_cur;

  int status;
  status=setrlimit(RLIMIT_CPU,&rlp);
  if (status !=0)
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Status cpulimit setrlimit : %1." << status);

  // set signal handler to print a message before death
  struct sigaction act, oact;
  memset (&act, 0, sizeof(act));
  act.sa_handler = cpulimitSignalHandler;
  sigaction (SIGXCPU, &act, &oact);

  // cpulimit for elapsed time
  aTimer.Reset();
  aTimer.Start();
  pthread_create(&cpulimitThread, NULL, CpuFunc, NULL);
#endif
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "CPU and elapsed time limit set to %1 seconds." << (double)CPU_LIMIT);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_StopCPULimit(const Handle(asiTcl_Interp)& interp,
                      int                          cmdTest_NotUsed(argc),
                      const char**                 cmdTest_NotUsed(argv))
{
  isTestThreadStoped = true;

#ifdef _WIN32
  if (hThread == nullptr)
  {
    return TCL_OK;
  }
  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hThread);
  hThread = nullptr;
#else
  rc = pthread_join(&cpulimitThread, NULL);
  if (rc)
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to join thread.");
    ExitProcess(2);
  }
#endif

  if (aTimer.IsStarted())
  {
    aTimer.Stop();
  }

  isTestThreadStoped = false;
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Limit of cpu time is removed.");
  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdTest::Commands_Clock(const Handle(asiTcl_Interp)&      interp,
                             const Handle(Standard_Transient)& cmdTest_NotUsed(data))
{
  static const char* group = "cmdTest";

  //-------------------------------------------------------------------------//
  interp->AddCommand("cpulimit",
    //
    "cpulimit <nbSeconds>\n"
    "\t Sets limit of cpu time.",
    //
    __FILE__, group, TEST_CPULimit);

  //-------------------------------------------------------------------------//
  interp->AddCommand("cpulimit-stop",
    //
    "cpulimit-stop <nbSeconds>\n"
    "\t Delete limit of cpu time.",
    //
    __FILE__, group, TEST_StopCPULimit);

}
