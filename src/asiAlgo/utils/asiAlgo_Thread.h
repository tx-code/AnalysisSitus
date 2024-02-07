// Created on: 2006-03-10
// Created by: Andrey BETENEV
// Modified by Sergey SLYADNEV 2024-02-07

#ifndef asiAlgo_Thread_HeaderFile
#define asiAlgo_Thread_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <OSD_ThreadFunction.hxx>
#include <OSD_PThread.hxx>
#include <Standard_ThreadId.hxx>
#include <Standard_Integer.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Address.hxx>

//-----------------------------------------------------------------------------

//! A simple platform-independent interface to execute
//! and control threads. This class is a patched copy of original
// OpenCascade's OSD_Thread that fixes timout on Linux.
class asiAlgo_Thread
{
public:

  //! Empty constructor
  Standard_EXPORT asiAlgo_Thread();

  //! Initialize the tool by the thread function
  //!
  //! Note: On Windows, you might have to take an address of the thread
  //! function explicitly to pass it to this constructor without compiler error
  Standard_EXPORT asiAlgo_Thread(const OSD_ThreadFunction& func);

  //! Copy constructor
  Standard_EXPORT asiAlgo_Thread(const asiAlgo_Thread& other);

  //! Copy thread handle from other OSD_Thread object.
  Standard_EXPORT void Assign (const asiAlgo_Thread& other);
void operator = (const asiAlgo_Thread& other)
{
  Assign(other);
}

  //! Destructor. Detaches the thread if it wasn't done already.
  Standard_EXPORT ~asiAlgo_Thread();

  Standard_EXPORT void SetPriority (const Standard_Integer thePriority);

  //! Initialize the tool by the thread function.
  //! If the current thread handle is not null, nullifies it.
  //!
  //! Note: On Windows, you might have to take an address of the thread
  //! function explicitly to pass it to this method without compiler error
  Standard_EXPORT void SetFunction (const OSD_ThreadFunction& func);

  //! Starts a thread with thread function given in constructor,
  //! passing the specified input data (as void *) to it.
  //! The parameter \a WNTStackSize (on Windows only)
  //! specifies size of the stack to be allocated for the thread
  //! (by default - the same as for the current executable).
  //! Returns True if thread started successfully
  Standard_EXPORT Standard_Boolean Run (const Standard_Address data = 0, const Standard_Integer WNTStackSize = 0);

  //! Detaches the execution thread from this Thread object,
  //! so that it cannot be waited.
  //! Note that mechanics of this operation is different on
  //! UNIX/Linux (the thread is put to detached state) and Windows
  //! (the handle is closed).
  //! However, the purpose is the same: to instruct the system to
  //! release all thread data upon its completion.
  Standard_EXPORT void Detach();

  //! Waits till the thread finishes execution.
  Standard_Boolean Wait()
  {
    Standard_Address aRes = 0;
    return Wait (aRes);
  }

  //! Wait till the thread finishes execution.
  //! Returns True if wait was successful, False in case of error.
  //!
  //! If successful and \a result argument is provided, saves the pointer
  //! (void*) returned by the thread function in \a result.
  //!
  //! Note however that it is advisable not to rely upon returned result
  //! value, as it is not always the value actually returned by the thread
  //! function. In addition, on Windows it is converted via DWORD.
  Standard_EXPORT Standard_Boolean Wait (Standard_Address& theResult);

  //! Waits for some time and if the thread is finished,
  //! it returns the result.
  //! The function returns false if the thread is not finished yet.
  Standard_EXPORT Standard_Boolean Wait (const Standard_Integer time, Standard_Address& theResult);

  //! Returns ID of the currently controlled thread ID,
  //! or 0 if no thread is run
  Standard_EXPORT Standard_ThreadId GetId() const;

  //! Auxiliary: returns ID of the current thread
  Standard_EXPORT static Standard_ThreadId Current();

private:

  OSD_ThreadFunction myFunc;
  OSD_PThread myThread;
  Standard_ThreadId myThreadId;
  Standard_Integer myPriority;

};

#endif // asiAlgo_Thread_HeaderFile
