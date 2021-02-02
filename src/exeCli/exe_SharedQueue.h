//-----------------------------------------------------------------------------
// Created on: 02 February 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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

#ifndef exe_SharedQueue_HeaderFile
#define exe_SharedQueue_HeaderFile

// Win API
#include <windows.h>

// OpenCascade includes
#include <Standard_Transient.hxx>

//! Thread-safe queue.
template<class T, unsigned max = 256>
class exe_SharedQueue : public Standard_Transient
{
private:

  //! Semaphores representing 'available slots' and 'available data' shared
  //! resources. Once any thread starts waiting for such a resource, the
  //! corresponding semaphore's count is decreased. The semaphore keeps
  //! being signaled until the count reaches 0.
  HANDLE m_hAvailableSlots, m_hAvailableData;

  //! Critical section barrier for threads in order to prevent them
  //! from running the same push/pop functionality simultaneously.
  CRITICAL_SECTION m_mutex;

  //! Buffer for the queued objects.
  T m_buff[max];

  //! Indices to access the elements of the queue.
  long m_iPos_in, m_iPos_out;

public:

  //! Default constructor.
  exe_SharedQueue() : Standard_Transient(),
                      m_iPos_in(0),
                      m_iPos_out(0)
  {
    // All slots are available at the very beginning
    m_hAvailableSlots = CreateSemaphore(NULL, max, max, NULL);

    // No data is available at the very beginning
    m_hAvailableData = CreateSemaphore(NULL, 0, max, NULL);

    // Initialize mutex
    InitializeCriticalSection(&m_mutex);
  }

  //! Destructor.
  virtual ~exe_SharedQueue()
  {
    DeleteCriticalSection(&m_mutex);
    CloseHandle(m_hAvailableSlots);
    CloseHandle(m_hAvailableData);
  }

public:

  //! Accessor for the last element in the queue.
  //! \return last element in queue.
  const T& Last()
  {
    return m_buff[m_iPos_out];
  }

  //! Pushes new element to the queue.
  //! \param Obj [in] element to push.
  void Push(const T& Obj)
  {
    // Wait until some slot is free (attempt to decrease semaphore's counter)
    WaitForSingleObject(m_hAvailableSlots, INFINITE);

    // Critical section
    EnterCriticalSection(&m_mutex);
    m_buff[m_iPos_in] = Obj;
    m_iPos_in = (m_iPos_in + 1) % max;
    LeaveCriticalSection(&m_mutex);

    // Increment data counter so that some thread can pop it
    ReleaseSemaphore(m_hAvailableData, 1, NULL);
  }

  //! Pops pending element from the queue.
  void Pop()
  {
    // Wait until some slot is free (attempt to decrease semaphore's counter)
    WaitForSingleObject(m_hAvailableData, INFINITE);

    // Critical section
    EnterCriticalSection(&m_mutex);
    m_iPos_out = (m_iPos_out + 1) % max;
    LeaveCriticalSection(&m_mutex);

    // Increment data counter so that some thread can pop it
    ReleaseSemaphore(m_hAvailableSlots, 1, NULL);
  }

};

#endif
