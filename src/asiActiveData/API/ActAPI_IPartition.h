//-----------------------------------------------------------------------------
// Created on: March 2012
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

#ifndef ActAPI_IPartition_HeaderFile
#define ActAPI_IPartition_HeaderFile

// Active Data (API) includes
#include <ActAPI_INode.h>

// OCCT includes
#include <Standard_Type.hxx>

//! \ingroup AD_API
//!
//! Interface for ACT data Partitions.
class ActAPI_IPartition : public ActAPI_IDataCursor
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_IPartition, ActAPI_IDataCursor)

public:

  ActData_EXPORT virtual
    ~ActAPI_IPartition();

public:

  //! Iterator for underlying Nodes.
  class Iterator 
  {
  public:

    ActData_EXPORT virtual
      ~Iterator();

  public:

    virtual void                 Init  (const Handle(ActAPI_IPartition)& theIP)       = 0;
    virtual Standard_Boolean     More  ()                                       const = 0;
    virtual void                 Next  ()                                             = 0;
    virtual Handle(ActAPI_INode) Value ()                                       const = 0;

  };

public:

  virtual ActAPI_DataObjectId
    AddNode(const Handle(ActAPI_INode)& theNode) = 0;

  virtual Standard_Boolean
    GetNode(const Standard_Integer theNodeNum,
            const Handle(ActAPI_INode)& theNode) const = 0;

  virtual Handle(ActAPI_INode)
    GetNode(const Standard_Integer theNodeNum) const = 0;

  virtual Handle(Standard_Type)
    GetNodeType() const = 0;

};

//! \ingroup AD_API
//!
//! Shortcuts for list of Parameters.
typedef NCollection_Sequence<Handle(ActAPI_IPartition)> ActAPI_PartitionList;
typedef NCollection_Shared<ActAPI_PartitionList> ActAPI_HPartitionList;

#endif
