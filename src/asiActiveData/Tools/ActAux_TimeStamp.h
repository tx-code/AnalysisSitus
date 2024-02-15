//-----------------------------------------------------------------------------
// Created on: April 2012
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
// Web: http://analysissitus.org
//-----------------------------------------------------------------------------

#ifndef ActAux_TimeStamp_HeaderFile
#define ActAux_TimeStamp_HeaderFile

// Active Data (auxiliary) includes
#include <ActAux_Common.h>

// OCCT includes
#include <NCollection_Array1.hxx>
#include <TCollection_AsciiString.hxx>
#include <NCollection_Handle.hxx>

// C library includes
#include <time.h>

DEFINE_STANDARD_HANDLE(ActAux_TimeStamp, Standard_Transient)

//! \ingroup AD_DF
//!
//! Extends time_t value with addendum integer value for uniqueness.
class ActAux_TimeStamp : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAux_TimeStamp, Standard_Transient)

public:

  //! Time.
  time_t Time;

  //! Additional integer for uniqueness.
  Standard_Integer Internal;

public:

  //! Default constructor.
  ActAux_TimeStamp() : Standard_Transient(), Time(-1), Internal(0)
  {}

  //! Complete constructor.
  //! \param theTime [in] time to initialize the unique timestamp with.
  //! \param theInternal [in] additional integer value to provide
  //!        uniqueness of the generated stamp.
  ActAux_TimeStamp(const time_t theTime, const Standard_Integer theInternal)
    : Standard_Transient(), Time(theTime), Internal(theInternal)
  {}

  //! Checks whether this timestamp is NULL (not initialized).
  //! \return true/false.
  Standard_Boolean IsOrigin()
  {
    return (Time == -1) && (Internal == 0);
  }

  //! Checks whether this timestamp is equal to the passed one.
  //! \param theOther [in] timestamp to compare with.
  //! \return true/false.
  Standard_Boolean IsEqual(const Handle(ActAux_TimeStamp)& theOther) const
  {
    return (Time == theOther->Time) && (Internal == theOther->Internal);
  }

  //! Checks whether this timestamp is less than the passed one.
  //! \param theOther [in] timestamp to compare with.
  //! \return true/false.
  Standard_Boolean IsLess(const Handle(ActAux_TimeStamp)& theOther) const
  {
    return (Time < theOther->Time) || ( (Time == theOther->Time) && (Internal < theOther->Internal) );
  }

  //! Checks whether this timestamp is less or equal to the passed one.
  //! \param theOther [in] timestamp to compare with.
  //! \return true/false.
  Standard_Boolean LessOrEqual(const Handle(ActAux_TimeStamp)& theOther) const
  {
    return this->IsLess(theOther) || this->IsEqual(theOther);
  }

  //! Checks whether this timestamp is greater than the passed one.
  //! \param theOther [in] timestamp to compare with.
  //! \return true/false.
  Standard_Boolean IsGreater(const Handle(ActAux_TimeStamp)& theOther) const
  {
    return (Time > theOther->Time) || ( (Time == theOther->Time) && (Internal > theOther->Internal) );
  }

  //! Checks whether this timestamp is greater or equal to the passed one.
  //! \param theOther [in] timestamp to compare with.
  //! \return true/false.
  Standard_Boolean IsGreaterOrEqual(const Handle(ActAux_TimeStamp)& theOther) const
  {
    return this->IsGreater(theOther) || this->IsEqual(theOther);
  }

  //! Copies the timestamp.
  //! \return copy of the timestamp.
  Handle(ActAux_TimeStamp) Clone() const
  {
    return new ActAux_TimeStamp(Time, Internal);
  }

  //! Dumps this timestamp to the passed output stream.
  //! \param theOut [in/out] stream to dump the timestamp to.
  void Dump(Standard_OStream& theOut)
  {
    theOut << this->ToString().ToCString();
  }

  //! Converts timestamp to string.
  //! \param useInternal [in] indicates whether the internal index must be
  //!        included into the string representation as well.
  //! \param isCompatible [in] indicates whether the requested string
  //!        representation has to contain "safe" characters only. In such
  //!        a form you can use it, for instance, as a filename.
  //! \return string representation of timestamp.
  TCollection_AsciiString ToString(const Standard_Boolean useInternal = Standard_True,
                                   const Standard_Boolean isCompatible = Standard_False) const
  {
    Standard_Character aBuf[26];
#ifdef _WIN32
    ctime_s(aBuf, 26, &Time);
#else
    ctime_r(&Time, aBuf);
#endif
    aBuf[24] = '\0'; // Replace EOL [\n\0 --> \0\0]

    TCollection_AsciiString aRes;
    aRes += aBuf;
    if ( useInternal )
      aRes = aRes.Cat(":: [").Cat(Internal).Cat("]");

    if ( isCompatible )
    {
      aRes.RemoveAll(':');
      aRes.RemoveAll('[');
      aRes.RemoveAll(']');
      aRes.ChangeAll(' ', '_');
    }

    return aRes;
  }
};

//! \ingroup AD_DF
//!
//! Auxiliary class generating unique and guaranteed to increase timestamps.
//! This class operates with standard time_t type not attempting to cast it
//! to any primitive (like integer, long etc). As this timestamp allows
//! to be pushed into CAF data structure, it needs to be represented somehow
//! via simple Standard_Integer basis. Even though straightforward
//! conversion from time_t to Standard_Integer does the trick, it is not
//! really a solution as it leads to injection of Y2K38 problem into the
//! Data Model. The natural alternative solution is to store standard "tm"
//! structure containing only Integer values and therefore being easily
//! mapped onto CAF Array of Integers.
//!
//! Another peculiarity here is that time_t value does not contain milliseconds
//! and cannot provide enough timing granularity for some cases. This is
//! resolved by extending that type with additional atomically incremented
//! static integer value. This value is used to maintain uniqueness only and
//! does not have any specific meaning so.
class ActAux_TimeStampTool
{
public:

  ActData_EXPORT static Handle(ActAux_TimeStamp)
    Generate();

  ActData_EXPORT static Handle(HIntArray)
    AsChunked(const Handle(ActAux_TimeStamp)& theUTime);

  ActData_EXPORT static Handle(ActAux_TimeStamp)
    FromChunked(const Handle(HIntArray)& theChunked);

};

#endif
