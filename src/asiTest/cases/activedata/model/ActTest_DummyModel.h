//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef ActTest_DummyModel_HeaderFile
#define ActTest_DummyModel_HeaderFile

// Active Data unit tests
#include <ActTest_StubAPartition.h>
#include <ActTest_StubBPartition.h>
#include <ActTest_StubCPartition.h>
#include <ActTest_StubMeshPartition.h>

// Active Data includes
#include <ActData_BaseModel.h>

DEFINE_STANDARD_HANDLE(ActTest_DummyModel, ActData_BaseModel)

//! \ingroup AD_TEST
//!
//! Data Model implementation for unit tests.
class ActTest_DummyModel : public ActData_BaseModel
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActTest_DummyModel, ActData_BaseModel)

public:

  ActTest_DummyModel();

public:

  //! Create a cloned instance of Data Model.
  //! \return cloned instance.
  inline virtual Handle(ActAPI_IModel) Clone() const
  {
    return ActData_BaseModel::CloneInstance<ActTest_DummyModel>();
  }

// Accessors:
public:

  //! Accessor for the Partition A.
  inline Handle(ActTest_StubAPartition) StubAPartition() const
  {
    return Handle(ActTest_StubAPartition)::DownCast( this->Partition(Partition_A) );
  }

  //! Accessor for the Partition B.
  inline Handle(ActTest_StubBPartition) StubBPartition() const
  {
    return Handle(ActTest_StubBPartition)::DownCast( this->Partition(Partition_B) );
  }

  //! Accessor for the Partition C.
  inline Handle(ActTest_StubCPartition) StubCPartition() const
  {
    return Handle(ActTest_StubCPartition)::DownCast( this->Partition(Partition_C) );
  }

  //! Accessor for the Mesh Partition.
  inline Handle(ActTest_StubMeshPartition) StubMeshPartition() const
  {
    return Handle(ActTest_StubMeshPartition)::DownCast( this->Partition(Partition_Mesh) );
  }

public:

  //! Initializes a collection of Tree Function GUIDs which are going to pass
  //! out-scope Reference Filter on Copy/Paste operation. Normally, you
  //! are expected to avoid the methods like this, just because there is
  //! already invariantCopyRefs method which is called automatically. The
  //! only benefit from having such additional methods is to provide
  //! dynamic changing of Reference Filter contents. It is very specific
  //! and unlikely valuable in real-world applications. However, this is
  //! really useful in unit tests.
  //! \param theFuncGUIDs [in] Function GUIDs to pass.
  inline void LoadCopyFuncGUIDs(const ActAPI_FuncGUIDStream& theFuncGUIDs)
  {
    m_copyGUIDs = theFuncGUIDs;
  }

  //! Same as LoadCopyFuncGUIDs, however, specialized for direct References,
  //! not for Tree Functions.
  //! \param theRefs [in] References to pass.
  inline void LoadCopyRefs(const ActAPI_ParameterLocatorStream& theRefs)
  {
    m_copyRefs = theRefs;
  }

// Methods we are forced to implement in order to have complete
// mechanism for Data Model construction:
private:

  virtual void
    initPartitions();

  virtual void
    initFunctionDrivers();

  virtual Standard_Boolean
    populate();

// Methods we are forced to implement in order to have complete set of
// Model structure management services:
private:

  Handle(ActAPI_IPartition)
    getVariablePartition(const VariableType& theVarType) const;

  virtual Handle(ActAPI_INode)
    getRootNode() const;

  virtual void
    invariantCopyRefs(ActAPI_FuncGUIDStream& FuncGUIDs,
                      ActAPI_ParameterLocatorStream& Refs) const;

// Methods we are forced to implement in order to have complete set of
// Model versioning services:
private:

  virtual Standard_Integer
    actualVersionApp();

  virtual Handle(ActData_CAFConverter)
    converterApp();

private:

  //! Partition IDs.
  enum PartitionType
  {
    Partition_A = 1,
    Partition_B,
    Partition_C,
    Partition_Mesh,
    Partition_RealVar,
    Partition_IntVar,
    Partition_BoolVar
  };

// Properties for dynamical setting of out-scope filter arguments:
private:

  //! Tree Function GUIDs to pass out-scope filtering.
  ActAPI_FuncGUIDStream m_copyGUIDs;

  //! Reference Parameters to pass out-scope filtering.
  ActAPI_ParameterLocatorStream m_copyRefs;

};

#endif
