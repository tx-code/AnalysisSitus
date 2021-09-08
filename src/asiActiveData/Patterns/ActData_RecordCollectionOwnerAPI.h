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

#ifndef ActData_RecordCollectionOwnerAPI_HeaderFile
#define ActData_RecordCollectionOwnerAPI_HeaderFile

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

//! \ingroup AD_DF
//!
//! Base class for Data Nodes manipulating collections of Records. By Record
//! we understand an internal Node which is normally not supposed to be
//! shown for the user directly.
class ActData_RecordCollectionOwnerAPI
{
// Working with reference lists:
public:

  ActData_EXPORT void
    AddRecord(const Standard_Integer theRefPID,
              const Handle(ActAPI_INode)& theTarget);

  ActData_EXPORT void
    PrependRecord(const Standard_Integer theRefPID,
                  const Handle(ActAPI_INode)& theTarget);

  ActData_EXPORT Standard_Boolean
    RemoveRecord(const Standard_Integer theRefPID,
                 const Standard_Integer theIndex);

  ActData_EXPORT void
    InsertRecordAfter(const Standard_Integer theRefPID,
                      const Standard_Integer theIndex,
                      const Handle(ActAPI_INode)& theTarget);

  ActData_EXPORT Handle(ActAPI_INode)
    GetRecord(const Standard_Integer theRefPID,
              const Standard_Integer theIndex) const;

  ActData_EXPORT Standard_Integer
    NbRecords(const Standard_Integer theRefPID) const;

  ActData_EXPORT Standard_Integer
    HasTarget(const Standard_Integer theRefPID,
              const Handle(ActAPI_INode)& theTarget) const;

  ActData_EXPORT Standard_Boolean
    SwapRecords(const Standard_Integer theRefPID,
                const Standard_Integer theFirstIndex,
                const Standard_Integer theSecondIndex);

public:

  virtual Handle(ActAPI_INode) RecordSource() const = 0;

// Internal methods:
protected:

  Handle(ActData_ReferenceListParameter) refsParam(const Standard_Integer theRefPID) const;

protected:

  //! Allocation is allowed only via Instance method.
  ActData_RecordCollectionOwnerAPI() {};

};

#endif
