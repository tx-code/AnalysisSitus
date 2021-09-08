//-----------------------------------------------------------------------------
// Created on: 2012-2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

#include <ActAux_TimeStamp.h>
#include <Standard_Mutex.hxx>

//! Generates timestamp structure for the current time.
//! \return timestamp structure.
Handle(ActAux_TimeStamp) ActAux_TimeStampTool::Generate()
{
  time_t aTime = -1;
  time(&aTime);

  Standard_Integer anInternal = 0;

#if defined(WIN32) || defined(_WIN32)
  static LONG INTERNAL = 0;
  anInternal = (Standard_Integer) InterlockedIncrement(&INTERNAL);
#else
  static unsigned long INTERNAL = 0;
  static Standard_Mutex MUTEX;

  MUTEX.Lock();
  anInternal = ++INTERNAL;
  MUTEX.Unlock();
#endif

  return new ActAux_TimeStamp(aTime, anInternal);
}

//! Converts the passed timestamp structure to the corresponding array of
//! Integer values.
//! \param theUTime [in] timestamp structure to convert.
//! \return correspondent array of integer data chunks.
Handle(HIntArray)
  ActAux_TimeStampTool::AsChunked(const Handle(ActAux_TimeStamp)& theUTime)
{
  Handle(HIntArray) aResult = new HIntArray(0, 9);

  if ( theUTime->Time != -1 )
  {
    tm aTimeInfo;
#ifdef _WIN32
    localtime_s(&aTimeInfo, &theUTime->Time);
#else
    localtime_r(&theUTime->Time, &aTimeInfo);
#endif

    aResult->ChangeValue(0) = aTimeInfo.tm_sec;
    aResult->ChangeValue(1) = aTimeInfo.tm_min;
    aResult->ChangeValue(2) = aTimeInfo.tm_hour;
    aResult->ChangeValue(3) = aTimeInfo.tm_mday;
    aResult->ChangeValue(4) = aTimeInfo.tm_mon;
    aResult->ChangeValue(5) = aTimeInfo.tm_year;
    aResult->ChangeValue(6) = aTimeInfo.tm_wday;
    aResult->ChangeValue(7) = aTimeInfo.tm_yday;
    aResult->ChangeValue(8) = aTimeInfo.tm_isdst;
    aResult->ChangeValue(9) = theUTime->Internal;
  }
  else
  {
    aResult->ChangeValue(0) = -1;
    aResult->ChangeValue(1) = -1;
    aResult->ChangeValue(2) = -1;
    aResult->ChangeValue(3) = -1;
    aResult->ChangeValue(4) = -1;
    aResult->ChangeValue(5) = -1;
    aResult->ChangeValue(6) = -1;
    aResult->ChangeValue(7) = -1;
    aResult->ChangeValue(8) = -1;
    aResult->ChangeValue(9) = -1;
  }

  return aResult;
}

//! Converts the passed array of integer data chunks to timestamp structure.
//! \param theChunked [in] input array.
//! \return correspondent timestamp structure.
Handle(ActAux_TimeStamp)
  ActAux_TimeStampTool::FromChunked(const Handle(HIntArray)& theChunked)
{
  if ( theChunked.IsNull() )
    return new ActAux_TimeStamp();

  tm aTimeInfo;
  aTimeInfo.tm_sec   = theChunked->Value(0);
  aTimeInfo.tm_min   = theChunked->Value(1);
  aTimeInfo.tm_hour  = theChunked->Value(2);
  aTimeInfo.tm_mday  = theChunked->Value(3);
  aTimeInfo.tm_mon   = theChunked->Value(4);
  aTimeInfo.tm_year  = theChunked->Value(5);
  aTimeInfo.tm_wday  = theChunked->Value(6);
  aTimeInfo.tm_yday  = theChunked->Value(7);
  aTimeInfo.tm_isdst = theChunked->Value(8);
  time_t aTime = mktime(&aTimeInfo);

  return new ActAux_TimeStamp( aTime, theChunked->Value(9) );
}
