// Created on: 2006-04-12
// Created by: Andrey BETENEV
// Modified by Sergey SLYADNEV 2024-02-07

// Own include
#include <asiAlgo_Thread.h>

// Standard includes
#include <iostream>
#include <thread>

//-----------------------------------------------------------------------------

#ifndef _WIN32

/*
 * The following code has been taken from https://gist.github.com/BinaryPrison/1112092
 * for "fast add nanoseconds to timespec structure."
 */

static inline uint32_t __iter_div_u64_rem(uint64_t dividend, uint32_t divisor, uint64_t *remainder)
{
  uint32_t ret = 0;

  while (dividend >= divisor) {
    /* The following asm() prevents the compiler from
       optimising this loop into a modulo operation.  */
    asm("" : "+rm"(dividend));

    dividend -= divisor;
    ret++;
  }

  *remainder = dividend;

  return ret;
}

#define NSEC_PER_SEC  1000000000L
static inline void timespec_add_ns(struct timespec *a, uint64_t ns)
{
  a->tv_sec += __iter_div_u64_rem(a->tv_nsec + ns, NSEC_PER_SEC, &ns);
  a->tv_nsec = ns;
}

#endif

//-----------------------------------------------------------------------------

asiAlgo_Thread::asiAlgo_Thread ()
  : myFunc(0), myThread(0), myThreadId(0), myPriority(0)
{}

//-----------------------------------------------------------------------------

asiAlgo_Thread::asiAlgo_Thread (const OSD_ThreadFunction &func)
  : myFunc(func), myThread(0), myThreadId(0), myPriority(0)
{}

//-----------------------------------------------------------------------------

asiAlgo_Thread::asiAlgo_Thread (const asiAlgo_Thread &other)
  : myFunc(other.myFunc), myThread(0), myThreadId(0)
{
  Assign ( other );
}

//-----------------------------------------------------------------------------

void asiAlgo_Thread::Assign (const asiAlgo_Thread &other)
{
  // copy function pointer
  myFunc = other.myFunc;
  myPriority = other.myPriority;

  // detach current thread
  Detach();

#ifdef _WIN32
  // duplicate the source handle
  if ( other.myThread ) {
    HANDLE hProc = GetCurrentProcess(); // we are always within the same process
    DuplicateHandle ( hProc, other.myThread, hProc, &myThread,
                      0, TRUE, DUPLICATE_SAME_ACCESS );
  }
#else
  // On Unix/Linux, just copy the thread id
  myThread = other.myThread;
#endif

  myThreadId = other.myThreadId;
}

//-----------------------------------------------------------------------------

asiAlgo_Thread::~asiAlgo_Thread()
{
  Detach();
}

//-----------------------------------------------------------------------------

// Set the thread priority relative to the caller's priority
void asiAlgo_Thread::SetPriority (const Standard_Integer thePriority)
{
  myPriority = thePriority;
#ifdef _WIN32
  if (myThread)
    SetThreadPriority (myThread, thePriority);
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_Thread::SetFunction (const OSD_ThreadFunction &func)
{
  // close current handle if any
  Detach();
  myFunc = func;
}

//-----------------------------------------------------------------------------

#ifdef _WIN32
#include <malloc.h>
// On Windows the signature of the thread function differs from that on UNIX/Linux.
// As we use the same definition of the thread function on all platforms (POSIX-like),
// we need to introduce appropriate wrapper function on Windows.
struct WNTthread_data { void *data; OSD_ThreadFunction func; };
static DWORD WINAPI WNTthread_func (LPVOID data)
{
  WNTthread_data *adata = (WNTthread_data*)data;
  void* ret = adata->func ( adata->data );
  free ( adata );
  return PtrToLong (ret);
}
#endif

//-----------------------------------------------------------------------------

Standard_Boolean asiAlgo_Thread::Run (const Standard_Address data,
#ifdef _WIN32
                                      const Standard_Integer WNTStackSize
#else
                                      const Standard_Integer
#endif
                                     )
{
  if ( ! myFunc ) return Standard_False;

  // detach current thread, if open
  Detach();

#ifdef _WIN32

  // allocate intermediate data structure to pass both data parameter and address
  // of the real thread function to Windows thread wrapper function
  WNTthread_data *adata = (WNTthread_data*)malloc ( sizeof(WNTthread_data) );
  if ( ! adata ) return Standard_False;
  adata->data = data;
  adata->func = myFunc;

  // then try to create a new thread
  DWORD aThreadId = DWORD();
  myThread = CreateThread ( NULL, WNTStackSize, WNTthread_func,
                            adata, 0, &aThreadId );
  myThreadId = aThreadId;
  if ( myThread )
    SetThreadPriority (myThread, myPriority);
  else {
    memset ( adata, 0, sizeof(WNTthread_data) );
    free ( adata );
  }

#else

  if (pthread_create (&myThread, 0, myFunc, data) != 0)
  {
    myThread = 0;
  }
  else
  {
    myThreadId = (Standard_ThreadId)myThread;
  }
#endif
  return myThread != 0;
}

//-----------------------------------------------------------------------------

void asiAlgo_Thread::Detach ()
{
#ifdef _WIN32

  // On Windows, close current handle
  if ( myThread )
    CloseHandle ( myThread );

#else

  // On Unix/Linux, detach a thread
  if ( myThread )
    pthread_detach ( myThread );

#endif

  myThread = 0;
  myThreadId = 0;
}

//-----------------------------------------------------------------------------

Standard_Boolean asiAlgo_Thread::Wait (Standard_Address& theResult)
{
  // check that thread handle is not null
  theResult = 0;
  if (!myThread)
  {
    return Standard_False;
  }

#ifdef _WIN32
  // On Windows, wait for the thread handle to be signaled
  if (WaitForSingleObject (myThread, INFINITE) != WAIT_OBJECT_0)
  {
    return Standard_False;
  }

  // and convert result of the thread execution to Standard_Address
  DWORD anExitCode;
  if (GetExitCodeThread (myThread, &anExitCode))
  {
    theResult = ULongToPtr (anExitCode);
  }

  CloseHandle (myThread);
  myThread   = 0;
  myThreadId = 0;
  return Standard_True;
#else
  // On Unix/Linux, join the thread
  if (pthread_join (myThread, &theResult) != 0)
  {
    return Standard_False;
  }

  myThread   = 0;
  myThreadId = 0;
  return Standard_True;
#endif
}

//-----------------------------------------------------------------------------

Standard_Boolean
  asiAlgo_Thread::Wait(const Standard_Integer theTimeMs,
                       Standard_Address&      theResult)
{
  // check that thread handle is not null
  theResult = 0;
  if (!myThread)
  {
    return Standard_False;
  }

#ifdef _WIN32
  // On Windows, wait for the thread handle to be signaled
  DWORD ret = WaitForSingleObject (myThread, theTimeMs);
  if (ret == WAIT_OBJECT_0)
  {
    DWORD anExitCode;
    if (GetExitCodeThread (myThread, &anExitCode))
    {
      theResult = ULongToPtr (anExitCode);
    }

    CloseHandle (myThread);
    myThread   = 0;
    myThreadId = 0;
    return Standard_True;
  }
  else if (ret == WAIT_TIMEOUT)
  {
    return Standard_False;
  }

  return Standard_False;
#else
  #if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
    #if __GLIBC_PREREQ(2,4)
      #define HAS_TIMED_NP
    #endif
  #endif

  #ifdef HAS_TIMED_NP

    time_t seconds      = (theTimeMs / 1000);
    long   microseconds = (theTimeMs - seconds * 1000) * 1000;
    long   nanoseconds  = microseconds*1000;

    int iret;
    struct timespec wait_time = { 0 };

    iret = pthread_tryjoin_np(myThread, NULL);
    //
    if ( ( iret != 0) && (iret != EBUSY) )
    {
      std::cout << "Error: unexpected iret code." << std::endl;
    }
    if (iret == EBUSY)
    {
      // Make sure to add nanoseconds while controlling the overflow in
      // seconds. If overflow is not handled, then `pthread_timedjoin_np()`
      // will not timeout properly (long threads will freeze).
      clock_gettime(CLOCK_REALTIME, &wait_time);
      wait_time.tv_sec += seconds;
      timespec_add_ns(&wait_time, nanoseconds);

      iret = pthread_timedjoin_np(
              myThread,
              NULL,
              &wait_time
      );
      std::cout << "pthread_timedjoin_np returns " << iret << " code" << std::endl;
      switch (iret) {
        case 0: // Everything's fine.
          break;
        case ETIMEDOUT:
        case EINVAL:
        default:
          break;
      }
    }

  #else
    // join the thread without timeout
    (void )theTimeMs;
    if (pthread_join (myThread, &theResult) != 0)
    {
      return Standard_False;
    }
  #endif
    myThread   = 0;
    myThreadId = 0;
    return Standard_True;
#endif
}

//-----------------------------------------------------------------------------

Standard_ThreadId asiAlgo_Thread::GetId () const
{
  return myThreadId;
}

//-----------------------------------------------------------------------------

Standard_ThreadId asiAlgo_Thread::Current ()
{
#ifdef _WIN32
  return GetCurrentThreadId();
#else
  return (Standard_ThreadId)pthread_self();
#endif
}
