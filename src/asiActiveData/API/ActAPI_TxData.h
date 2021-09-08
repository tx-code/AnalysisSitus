//-----------------------------------------------------------------------------
// Created on: July 2012
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

#ifndef ActAPI_TxData_HeaderFile
#define ActAPI_TxData_HeaderFile

// Active Data (API) includes
#include <ActData.h>

// OCCT includes
#include <NCollection_Sequence.hxx>
#include <Standard_ProgramError.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

#define ERR_INCONSISTENT_DATA_TYPES "Inconsistent data types"

//-----------------------------------------------------------------------------
// Shared Integer
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Integer value for chaining into Transaction Data.
class ActAPI_TxPrimDataInt : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_TxPrimDataInt, Standard_Transient)

public:

  //! Factory method ensuring allocation of class instance in a heap.
  //! \param theValue [in] value.
  //! \return class instance.
  inline static Handle(ActAPI_TxPrimDataInt) Instance(const Standard_Integer theValue)
  {
    return new ActAPI_TxPrimDataInt(theValue);
  }

public:

  //! Shared value.
  Standard_Integer Value;

private:

  //! Default constructor.
  ActAPI_TxPrimDataInt() : Standard_Transient() {}

  //! Constructor accepting the value to set.
  //! \param theValue [in] value to set.
  ActAPI_TxPrimDataInt(const Standard_Integer theValue) : Standard_Transient(), Value(theValue) {}

};

//-----------------------------------------------------------------------------
// Shared Real
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Real value for chaining into Transaction Data.
class ActAPI_TxPrimDataReal : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_TxPrimDataReal, Standard_Transient)

public:

  //! Factory method ensuring allocation of class instance in a heap.
  //! \param theValue [in] value.
  //! \return class instance.
  ActData_EXPORT static Handle(ActAPI_TxPrimDataReal) Instance(const Standard_Real theValue)
  {
    return new ActAPI_TxPrimDataReal(theValue);
  }

public:

  //! Shared value.
  Standard_Real Value;

private:

  //! Default constructor.
  ActAPI_TxPrimDataReal() : Standard_Transient() {}

  //! Constructor accepting the value to set.
  //! \param theValue [in] value to set.
  ActAPI_TxPrimDataReal(const Standard_Real theValue) : Standard_Transient(), Value(theValue) {}

};

//-----------------------------------------------------------------------------
// Shared Boolean
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Boolean value for chaining into Transaction Data.
class ActAPI_TxPrimDataBool : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_TxPrimDataBool, Standard_Transient)

public:

  //! Factory method ensuring allocation of class instance in a heap.
  //! \param theValue [in] value.
  //! \return class instance.
  inline static Handle(ActAPI_TxPrimDataBool) Instance(const Standard_Boolean theValue)
  {
    return new ActAPI_TxPrimDataBool(theValue);
  }

public:

  //! Shared value.
  Standard_Boolean Value;

private:

  //! Default constructor.
  ActAPI_TxPrimDataBool() : Standard_Transient() {}

  //! Constructor accepting the value to set.
  //! \param theValue [in] value to set.
  ActAPI_TxPrimDataBool(const Standard_Boolean theValue) : Standard_Transient(), Value(theValue) {}

};

//-----------------------------------------------------------------------------
// Shared String
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! ASCII string value for chaining into Transaction Data.
class ActAPI_TxPrimDataString : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_TxPrimDataString, Standard_Transient)

public:

  //! Factory method ensuring allocation of class instance in a heap.
  //! \param theStr [in] string.
  //! \return class instance.
  inline static Handle(ActAPI_TxPrimDataString) Instance(const TCollection_AsciiString& theStr)
  {
    return new ActAPI_TxPrimDataString(theStr);
  }

public:

  //! Shared string.
  TCollection_AsciiString Value;

private:

  //! Default constructor.
  ActAPI_TxPrimDataString() : Standard_Transient() {}

  //! Constructor accepting the string to set.
  //! \param theStr [in] string to set.
  ActAPI_TxPrimDataString(const TCollection_AsciiString& theStr) : Standard_Transient(), Value(theStr) {}

};

//-----------------------------------------------------------------------------
// Addendum
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Collection of Python variables
typedef NCollection_Sequence<Handle(Standard_Transient)> ActAPI_TxPrimDataSeq;
typedef NCollection_Shared<ActAPI_TxPrimDataSeq>         ActAPI_HTxPrimDataSeq;

//-----------------------------------------------------------------------------
// Application-specific Transaction Data
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Container for associating Data Model Transactions with
//! application-specific data.
class ActAPI_TxData
{
public:

  //! Actual ordered data collection.
  ActAPI_TxPrimDataSeq TxPrimDataSeq;

public:

  //! Default constructor accepting integer value for the sake of
  //! convenient conversion.
  ActAPI_TxData(Standard_Integer = 0) : m_iSeekIndx(1) {}

  //! Copy constructor.
  ActAPI_TxData(const ActAPI_TxData& theData) : m_iSeekIndx(1) // (!)
  {
    TxPrimDataSeq = theData.TxPrimDataSeq;
  }

  //! Returns true if the Data container is empty.
  //! \return true/false.
  Standard_Boolean IsEmpty() const
  {
    return TxPrimDataSeq.IsEmpty();
  }

public:

  //! INPUT streaming method for integer values.
  //! \param theData [in] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator<<(const Standard_Integer theData)
  {
    TxPrimDataSeq.Append( ActAPI_TxPrimDataInt::Instance(theData) );
    return *this;
  }

  //! INPUT streaming method for real values.
  //! \param theData [in] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator<<(const Standard_Real theData)
  {
    TxPrimDataSeq.Append( ActAPI_TxPrimDataReal::Instance(theData) );
    return *this;
  }

  //! INPUT streaming method for Boolean values.
  //! \param theData [in] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator<<(const Standard_Boolean theData)
  {
    TxPrimDataSeq.Append( ActAPI_TxPrimDataBool::Instance(theData) );
    return *this;
  }

  //! INPUT streaming method for string values.
  //! \param theData [in] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator<<(const Standard_CString theData)
  {
    TxPrimDataSeq.Append( ActAPI_TxPrimDataString::Instance(theData) );
    return *this;
  }

  //! INPUT streaming method for string values.
  //! \param theData [in] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator<<(const TCollection_AsciiString& theData)
  {
    return this->operator<<( theData.ToCString() );
  }

  //! OUTPUT streaming method for integer values.
  //! \param theData [out] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator>>(Standard_Integer& theData)
  {
    if ( m_iSeekIndx > TxPrimDataSeq.Length() )
      return *this;

    Handle(Standard_Transient) aTxData = TxPrimDataSeq.Value(m_iSeekIndx++);

    if ( !aTxData->IsInstance( STANDARD_TYPE(ActAPI_TxPrimDataInt) ) )
      Standard_ProgramError::Raise(ERR_INCONSISTENT_DATA_TYPES);

    theData = Handle(ActAPI_TxPrimDataInt)::DownCast(aTxData)->Value;
    return *this;
  }

  //! OUTPUT streaming method for real values.
  //! \param theData [out] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator>>(Standard_Real& theData)
  {
    if ( m_iSeekIndx > TxPrimDataSeq.Length() )
      return *this;

    Handle(Standard_Transient) aTxData = TxPrimDataSeq.Value(m_iSeekIndx++);

    if ( !aTxData->IsInstance( STANDARD_TYPE(ActAPI_TxPrimDataReal) ) )
      Standard_ProgramError::Raise(ERR_INCONSISTENT_DATA_TYPES);

    theData = Handle(ActAPI_TxPrimDataReal)::DownCast(aTxData)->Value;
    return *this;
  }

  //! OUTPUT streaming method for Boolean values.
  //! \param theData [out] value to stream.
  //! \return this instance.
  ActAPI_TxData& operator>>(Standard_Boolean& theData)
  {
    if ( m_iSeekIndx > TxPrimDataSeq.Length() )
      return *this;

    Handle(Standard_Transient) aTxData = TxPrimDataSeq.Value(m_iSeekIndx++);

    if ( !aTxData->IsInstance( STANDARD_TYPE(ActAPI_TxPrimDataBool) ) )
      Standard_ProgramError::Raise(ERR_INCONSISTENT_DATA_TYPES);

    theData = Handle(ActAPI_TxPrimDataBool)::DownCast(aTxData)->Value;
    return *this;
  }

  //! OUTPUT streaming method for String values.
  //! \param theStr [out] string to stream.
  //! \return this instance.
  ActAPI_TxData& operator>>(TCollection_AsciiString& theStr)
  {
    if ( m_iSeekIndx > TxPrimDataSeq.Length() )
      return *this;

    Handle(Standard_Transient) aTxData = TxPrimDataSeq.Value(m_iSeekIndx++);

    if ( !aTxData->IsInstance( STANDARD_TYPE(ActAPI_TxPrimDataString) ) )
      Standard_ProgramError::Raise(ERR_INCONSISTENT_DATA_TYPES);

    theStr = Handle(ActAPI_TxPrimDataString)::DownCast(aTxData)->Value;
    return *this;
  }

private:

  //! Internal seek index used for OUTPUT streaming.
  Standard_Integer m_iSeekIndx;

};

//-----------------------------------------------------------------------------
// Addendum
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Type definition for collection of Transaction Data structures.
typedef NCollection_Sequence<ActAPI_TxData>  ActAPI_TxDataSeq;
typedef NCollection_Shared<ActAPI_TxDataSeq> ActAPI_HTxDataSeq;

#endif
