//-----------------------------------------------------------------------------
// Created on: April 2014
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

// Active Data includes
#include <ActData_RealArrayOwnerAPI.h>

// Active Data (auxiliary) includes
#include <ActAux_ArrayUtils.h>

//! Appends the passed element to the end of the stored array.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theVal [in] data to add.
void ActData_RealArrayOwnerAPI::AddElement(const Standard_Integer theArrPID,
                                           const Standard_Real theVal)
{
  Handle(HRealArray) aSourceArr = this->arrParam(theArrPID)->GetArray();
  Handle(HRealArray) aResArr =
    ActAux_ArrayUtils::Append<HRealArray, Handle(HRealArray), Standard_Real>(aSourceArr, theVal);
  this->arrParam(theArrPID)->SetArray(aResArr);
}

//! Prepends the passed data to the beginning of the stored array.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theVal [in] data to prepend.
void ActData_RealArrayOwnerAPI::PrependElement(const Standard_Integer theArrPID,
                                               const Standard_Real theVal)
{
  Handle(HRealArray) aSourceArr = this->arrParam(theArrPID)->GetArray();
  Handle(HRealArray) aResArr =
    ActAux_ArrayUtils::Prepend<HRealArray, Handle(HRealArray), Standard_Real>(aSourceArr, theVal);
  this->arrParam(theArrPID)->SetArray(aResArr);
}

//! Removes the element referred to by the given index.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theIndex [in] index of the element in the internal collection.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_RealArrayOwnerAPI::RemoveElement(const Standard_Integer theArrPID,
                                           const Standard_Integer theIndex)
{
  Handle(HRealArray) aSourceArr = this->arrParam(theArrPID)->GetArray();
  Handle(HRealArray) aResArr =
    ActAux_ArrayUtils::Remove<HRealArray, Handle(HRealArray)>(aSourceArr, theIndex);
  this->arrParam(theArrPID)->SetArray(aResArr);

  const Standard_Integer resLen = ( aResArr.IsNull() ? 0 : aResArr->Length() );
  const Standard_Integer srcLen = ( aSourceArr.IsNull() ? 0 : aSourceArr->Length() );

  return resLen != srcLen;
}

//! Inserts the given data after another item referred to by the passed index.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theIndex [in] index of the element to insert the new one after.
//! \param theVal [in] data to insert.
void ActData_RealArrayOwnerAPI::InsertElementAfter(const Standard_Integer theArrPID,
                                                   const Standard_Integer theIndex,
                                                   const Standard_Real theVal)
{
  if ( theIndex == -1 )
    this->PrependElement(theArrPID, theVal);
  else if ( theIndex == (this->NbElements(theArrPID) - 1) )
    this->AddElement(theArrPID, theVal);
  else
  {
    Handle(HRealArray) aSourceArr = this->arrParam(theArrPID)->GetArray();
    Handle(HRealArray) aResArr =
      ActAux_ArrayUtils::InsertAfter<HRealArray, Handle(HRealArray), Standard_Real>(aSourceArr, theVal, theIndex);
    this->arrParam(theArrPID)->SetArray(aResArr);
  }
}

//! Accessor for the element referred to by the passed index.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theIndex [in] index of the element to access.
//! \return requested data.
Standard_Real
  ActData_RealArrayOwnerAPI::GetElement(const Standard_Integer theArrPID,
                                        const Standard_Integer theIndex) const
{
  Handle(ActData_RealArrayParameter) p = this->arrParam(theArrPID);
  if ( p.IsNull() || !p->IsWellFormed() )
    return DBL_MAX;

  return p->GetElement(theIndex);
}

//! Sets element.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \param theIndex [in] index of the element to set.
//! \param theVal [in] value to set.
void ActData_RealArrayOwnerAPI::SetElement(const Standard_Integer theArrPID,
                                           const Standard_Integer theIndex,
                                           const Standard_Real theVal) const
{
  Handle(ActData_RealArrayParameter) p = this->arrParam(theArrPID);
  if ( p.IsNull() || !p->IsWellFormed() )
    return;

  p->SetElement(theIndex, theVal);
}

//! Returns the number of stored element.
//! \param theArrPID [in] ID of the Real Array Parameter containing
//!        actual data.
//! \return number of elements.
Standard_Integer
  ActData_RealArrayOwnerAPI::NbElements(const Standard_Integer theArrPID) const
{
  Handle(ActData_RealArrayParameter) p = this->arrParam(theArrPID);
  if ( p.IsNull() || !p->IsWellFormed() )
    return 0;

  return p->NbElements();
}

//! Convenient accessor for the Real Array Parameter identified by
//! the given ID.
//! \param theArrPID [in] ID of the Real Array Parameter to access.
//! \return requested Parameter.
Handle(ActData_RealArrayParameter)
  ActData_RealArrayOwnerAPI::arrParam(const Standard_Integer theArrPID) const
{
  return ActParamTool::AsRealArray( this->RealArraySource()->Parameter(theArrPID) );
}
