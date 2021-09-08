//-----------------------------------------------------------------------------
// Created on: February 2012
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

#ifndef ActData_BasePartition_HeaderFile
#define ActData_BasePartition_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_IPartition.h>

// OCCT includes
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelList.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>

DEFINE_STANDARD_HANDLE(ActData_BasePartition, ActAPI_IPartition)

//! \ingroup AD_DF
//!
//! Partition class playing a role of container for specific Nodes. Each
//! type of Partition is expected to contain a dedicated kind of Nodes in
//! order to group them by types and avoid data mix-up so. The Data Model
//! with Partitions and underlying Nodes form a three-level conceptual model
//! of physical data storage. Besides the grouping purpose, Partitions
//! provide the very basic services (e.g. accessing a Node by its
//! sequential number) for manipulations with Nodes.
class ActData_BasePartition : public ActAPI_IPartition
{
friend class ActData_BaseModel;
friend class ActData_CAFDumper;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BasePartition, ActAPI_IPartition)

public:

  //! Iterator for underlying Nodes.
  class Iterator 
  {
  public:

    ActData_EXPORT Iterator(const Handle(ActAPI_IPartition)& theIP);
    ActData_EXPORT void Init(const Handle(ActAPI_IPartition)& theIP);
    ActData_EXPORT Standard_Boolean More() const;
    ActData_EXPORT void Next();
    ActData_EXPORT Handle(ActAPI_INode) Value() const;

  private:

    Handle(ActData_BasePartition) m_partition; //!< Owning partition.
    TDF_ListIteratorOfLabelList m_it; //!< Internal iterator for TDF labels.
    TDF_LabelList m_labelList; //!< List being iterated over.

  };

// Transient properties of Data Cursor:
public:

  ActData_EXPORT virtual Standard_Boolean
    IsAttached();

  ActData_EXPORT virtual Standard_Boolean
    IsDetached();

  ActData_EXPORT virtual Standard_Boolean
    IsWellFormed();

  ActData_EXPORT virtual Standard_Boolean
    IsValidData();

  ActData_EXPORT virtual Standard_Boolean
    IsPendingData();

  ActData_EXPORT virtual ActAPI_DataObjectId
    GetId() const;

public:

  ActData_EXPORT virtual ActAPI_DataObjectId
    AddNode(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Standard_Boolean
    GetNode(const Standard_Integer theNodeNum,
            const Handle(ActAPI_INode)& theNode) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    GetNode(const Standard_Integer theNodeNum) const;

  ActData_EXPORT virtual TDF_Label
    RootLabel() const;

protected:

  //! Prohibit instantiation of abstract class.
  ActData_EXPORT ActData_BasePartition();

// Data Cursor internals:
protected:

  void attach(const TDF_Label& theLabel);
  ActData_EXPORT virtual void expandOn(const TDF_Label& theLabel);
  ActData_EXPORT virtual void settleOn(const TDF_Label& theLabel);

private:

  //! Label the Partition is attached to (can be NULL in DETACHED mode).
  TDF_Label m_label;

};

#endif
