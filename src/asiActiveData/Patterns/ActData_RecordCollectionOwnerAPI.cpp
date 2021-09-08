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

// Own include
#include <ActData_RecordCollectionOwnerAPI.h>

//! Appends the passed data to the end of the stored list of data records.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theTarget [in] data to add.
void ActData_RecordCollectionOwnerAPI::AddRecord(const Standard_Integer theRefPID,
                                                 const Handle(ActAPI_INode)& theTarget)
{
  this->RecordSource()->ConnectReferenceToList( theRefPID, theTarget, this->NbRecords(theRefPID) );
}

//! Prepends the passed data to the beginning of the stored list of records.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theTarget [in] data to prepend.
void ActData_RecordCollectionOwnerAPI::PrependRecord(const Standard_Integer theRefPID,
                                                     const Handle(ActAPI_INode)& theTarget)
{
  this->RecordSource()->ConnectReferenceToList(theRefPID, theTarget, 0);
}

//! Removes the data record referred to by the given index.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theIndex [in] index of the data record in the internal collection.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_RecordCollectionOwnerAPI::RemoveRecord(const Standard_Integer theRefPID,
                                                 const Standard_Integer theIndex)
{
  return this->RecordSource()->DisconnectReferenceFromList(theRefPID, theIndex);
}

//! Inserts the given data after another item referred to by the passed index.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theIndex [in] index of the data record to insert the new one after.
//! \param theTarget [in] data to insert.
void ActData_RecordCollectionOwnerAPI::InsertRecordAfter(const Standard_Integer theRefPID,
                                                         const Standard_Integer theIndex,
                                                         const Handle(ActAPI_INode)& theTarget)
{
  this->RecordSource()->ConnectReferenceToList(theRefPID, theTarget, theIndex);
}

//! Accessor for the data record referred to by the passed index.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theIndex [in] index of the data record to access.
//! \return requested data.
Handle(ActAPI_INode)
  ActData_RecordCollectionOwnerAPI::GetRecord(const Standard_Integer theRefPID,
                                              const Standard_Integer theIndex) const
{
  Standard_Integer aNbRecords = this->NbRecords(theRefPID);

  if ( theIndex > aNbRecords || theIndex < 1 )
    return NULL;

  Handle(ActAPI_IDataCursor)
    aTargetDC = this->refsParam(theRefPID)->GetTarget(theIndex);

  return Handle(ActAPI_INode)::DownCast(aTargetDC);
}

//! Swap the records with the given indexes.
//! Does nothing if indexes are not valid.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \param theFirstIndex [in] the index of the first record.
//! \param theSecondIndex [in] the index of the second record.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_RecordCollectionOwnerAPI::SwapRecords(const Standard_Integer theRefPID,
                                                               const Standard_Integer theFirstIndex,
                                                               const Standard_Integer theSecondIndex)
{
  Standard_Integer aNbRecords = this->NbRecords(theRefPID);

  // Indexes are valid?
  if ( theFirstIndex == theSecondIndex)
    return Standard_False;

  if ( theFirstIndex > aNbRecords || theFirstIndex < 1 )
    return Standard_False;

  if ( theSecondIndex > aNbRecords || theSecondIndex < 1 )
    return Standard_False;

  // Do swap on ReferenceListParameter
  return this->refsParam(theRefPID)->SwapTargets(theFirstIndex,theSecondIndex);
}

//! Returns the number of stored data records.
//! \param theRefPID [in] ID of the Reference List Parameter containing
//!        actual data.
//! \return number of data records.
Standard_Integer
  ActData_RecordCollectionOwnerAPI::NbRecords(const Standard_Integer theRefPID) const
{
  return this->refsParam(theRefPID)->NbTargets();
}

//! Checks whether the passed target is referenced by the Reference List
//! Parameter identified by the given ID.
//! \param theRefPID [in] ID of the Reference List Parameter to access.
//! \param theTarget [in] Data Cursor to check.
//! \return index of the target or 0 if nothing was found.
Standard_Integer 
  ActData_RecordCollectionOwnerAPI::HasTarget(const Standard_Integer theRefPID,
                                              const Handle(ActAPI_INode)& theTarget) const
{
  return this->refsParam(theRefPID)->HasTarget(theTarget);
}

//! Convenient accessor for the Reference List Parameter identified by
//! the given ID.
//! \param theRefPID [in] ID of the Reference List Parameter to access.
//! \return requested Parameter.
Handle(ActData_ReferenceListParameter)
  ActData_RecordCollectionOwnerAPI::refsParam(const Standard_Integer theRefPID) const
{
  return ActParamTool::AsReferenceList( this->RecordSource()->Parameter(theRefPID) );
}
