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

#ifndef ActData_BaseNode_HeaderFile
#define ActData_BaseNode_HeaderFile

// Active Data includes
#include <ActData_IntParameter.h>
#include <ActData_MetaParameter.h>
#include <ActData_NameParameter.h>
#include <ActData_NodeFactory.h>
#include <ActData_ReferenceListParameter.h>
#include <ActData_ReferenceParameter.h>
#include <ActData_TreeFunctionParameter.h>
#include <ActData_TreeNodeParameter.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <NCollection_Handle.hxx>
#include <NCollection_SparseArray.hxx>
#include <TDataStd_ChildNodeIterator.hxx>
#include <TDF_Label.hxx>

// Active Data forward declarations
class ActData_TreeFunctionParameter;

DEFINE_STANDARD_HANDLE(ActData_BaseNode,          ActAPI_INode)
DEFINE_STANDARD_HANDLE(ActData_BaseParamIterator, ActAPI_IParamIterator)
DEFINE_STANDARD_HANDLE(ActData_BaseChildIterator, ActAPI_IChildIterator)

//! \ingroup AD_DF
//!
//! \todo provide comment here
class ActData_ParameterScope
{
public:

  typedef NCollection_DataMap<Standard_Integer, Standard_Integer> TagRelation;

public:

  //! Meta Parameter.
  Handle(ActData_MetaParameter) Meta;

  //! Custom Parameters specified by Data Framework user.
  Handle(ActAPI_HIndexedParameterMap) User;

  //! Mapping between tags for expressible Parameters and their
  //! correspondent Tree Function evaluator Parameters.
  TagRelation ExpressibleParams;

public:

  //! Returns the casted instance of Parameter corresponding to the
  //! given relative ID.
  //! \param theId      [in] ID of the user Parameter to access.
  //! \param isInternal [in] indicates whether the Parameter to access is
  //!                        internal (true) or the user's one (false).
  //! \return Parameter.
  template<typename T>
  T SafeCast(const Standard_Integer theId,
             const Standard_Boolean isInternal)
  {
    Handle(ActAPI_IUserParameter) aBaseParam = (isInternal ? this->Meta->Evaluators()->find(theId)->second :
                                                             this->User->find(theId)->second );
    return T::DownCast(aBaseParam);
  }

};

//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Default implementation of Iterator interface for Nodal Parameters.
class ActData_BaseParamIterator : public ActAPI_IParamIterator
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseParamIterator, ActAPI_IParamIterator)

public:

  ActData_EXPORT
    ActData_BaseParamIterator(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual void
    Init(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Standard_Boolean
    More() const;

  ActData_EXPORT virtual void
    Next();

  ActData_EXPORT virtual Standard_Integer
    Key() const;

  ActData_EXPORT virtual const Handle(ActAPI_IUserParameter)&
    Value() const;

private:

  Handle(ActData_BaseNode)                   m_node; //!< Owning Node.
  ActAPI_IndexedParameterMap::const_iterator m_it;   //!< Internal iterator for Parameters.

};

//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Default implementation of Iterator interface for child Nodes.
class ActData_BaseChildIterator : public ActAPI_IChildIterator
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseChildIterator, ActAPI_IChildIterator)

public:

  ActData_EXPORT
    ActData_BaseChildIterator(const Handle(ActAPI_INode)& theNode,
                              const Standard_Boolean isAllLevels = Standard_False);

  ActData_EXPORT void
    Init(const Handle(ActAPI_INode)& theNode,
         const Standard_Boolean isAllLevels = Standard_False);

  ActData_EXPORT Standard_Boolean
    More() const;

  ActData_EXPORT void
    Next();

  ActData_EXPORT Handle(ActAPI_INode)
    Value() const;

  ActData_EXPORT virtual TDF_Label
    ValueLabel() const;

private:

  Handle(ActData_BaseNode)   m_node; //!< Owning Node.
  TDataStd_ChildNodeIterator m_it;   //!< Internal iterator for TDataStd_TreeNode.

};

//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Base abstract class for Active Data Nodes. Provides a common interface
//! for setting up an arbitrary collection of Parameters and consulting
//! them via Iterator pattern. The caller class should be aware of
//! actual type of the Node in order to retrieve data encapsulated in
//! Parameters. You're also free to implement your own iterators and
//! data accessors in the derived classes.
//!
//! Technically, each Data Node is mapped onto CAF structure in the following
//! way:
//!
//! <pre>
//! +------------+
//! | Node Root  |
//! +------------+
//!              |
//!              +----------------+
//!              | Meta Parameter |
//!              +----------------+
//!              |
//!              +-----------------+
//!              | User Parameters |
//!              +-----------------+
//!                               |
//!                               +---------------------+
//!                               |   Any Parameter A   |
//!                               +---------------------+
//!                               |
//!                               +---------------------+
//!                               |   Any Parameter B   |
//!                               +---------------------+
//!                               |
//!                              ...
//!                               |
//!                               +---------------------+
//!                               |   Any Parameter Z   |
//!                               +---------------------+
//! </pre>
class ActData_BaseNode : public ActAPI_INode
{
friend class ActData_BaseModel;
friend class ActData_BasePartition;
friend class ActData_UserParameter;
friend class ActData_BaseParamIterator;
friend class ActData_BaseChildIterator;
friend class ActData_CAFConversionAsset;
friend class ActData_CAFConversionCtx;
friend class ActData_CAFConversionModel;
friend class ActData_CAFDumper;
friend class ActData_CopyPasteEngine;
friend class ActData_MetaParameter;
friend class ActData_NodeFactory;
friend class ActData_ReferenceListParameter;
friend class ActData_TreeNodeParameter;
friend class ActData_TransactionEngine;
friend class ActData_Utils;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseNode, ActAPI_INode)

  //! Stores a number of reserved tags for future extensions.
  static const Standard_Integer RESERVED_PARAM_RANGE = 100;

public:

  static const Standard_Integer TagInternal = 1;
  static const Standard_Integer TagUser     = 2;

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
    IsValidDataWithChildren(const Standard_Boolean forAllLevels = Standard_False);

  ActData_EXPORT virtual Standard_Boolean
    IsPendingData();

  ActData_EXPORT virtual ActAPI_DataObjectId
    GetId() const;

// Interface for common Parameters:
public:

  ActData_EXPORT virtual Handle(ActAPI_IParamIterator)
    GetParamIterator() const;

  ActData_EXPORT virtual Handle(ActAPI_HIndexedParameterMap)
    Parameters() const;

  ActData_EXPORT virtual Handle(ActAPI_IUserParameter)
    Parameter(const Standard_Integer theId) const;

  ActData_EXPORT virtual void
    InitParameter(const Standard_Integer theId,
                  const TCollection_ExtendedString& theName,
                  const TCollection_AsciiString& theSemanticId = TCollection_AsciiString(),
                  const Standard_Integer theUFlags = 0x0000,
                  const Standard_Boolean isSilent = Standard_False);

// Base naming functionality:
public:

  virtual TCollection_ExtendedString
    GetName() = 0;

  virtual void
    SetName(const TCollection_ExtendedString& theName) = 0;

// Accessors for some open internal properties:
public:

  ActData_EXPORT virtual TCollection_AsciiString
    GetTypeName();

  ActData_EXPORT virtual Standard_Integer
    GetUserFlags();

  ActData_EXPORT virtual Standard_Boolean
    HasUserFlags(const Standard_Integer theUFlags);

  ActData_EXPORT virtual void
    SetUserFlags(const Standard_Integer theUFlags);

  ActData_EXPORT virtual void
    AddUserFlags(const Standard_Integer theUFlags);

  ActData_EXPORT virtual void
    RemoveUserFlags(const Standard_Integer theUFlags);

// Alternative hierarchy of Nodes:
public:

  ActData_EXPORT virtual Handle(ActAPI_IChildIterator)
    GetChildIterator(const Standard_Boolean isAllLevels = Standard_False) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    GetChildNode(const Standard_Integer oneBased_idx) const;

  ActData_EXPORT virtual int
    GetChildren(Handle(ActAPI_HNodeList)& theChildren) const;

  ActData_EXPORT virtual void
    AddChildNode(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Standard_Boolean
    RemoveChildNode(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Handle(ActAPI_INode)
    GetParentNode();

// OCAF internals:
public:

  ActData_EXPORT virtual TDF_Label
    RootLabel() const;

// Tree Functions:
public:

  ActData_EXPORT virtual void
    ConnectTreeFunction(const Standard_Integer theId,
                        const Standard_GUID& theGUID,
                        const Handle(ActAPI_HParameterList)& theArgsIN,
                        const Handle(ActAPI_HParameterList)& theArgsOUT);

  ActData_EXPORT virtual Standard_Boolean
    HasConnectedTreeFunction(const Standard_Integer theId);

  ActData_EXPORT virtual void
    DisconnectTreeFunction(const Standard_Integer theId);

// Variables evaluation mechanism:
public:

  ActData_EXPORT virtual void
    ConnectEvaluator(const Standard_Integer theId,
                     const Handle(ActAPI_HParameterList)& theVarsIN);

  ActData_EXPORT virtual Standard_Boolean
    HasConnectedEvaluator(const Standard_Integer theId);

  ActData_EXPORT virtual Handle(ActAPI_IUserParameter)
    Evaluator(const Standard_Integer theId);

  ActData_EXPORT virtual Standard_Boolean
    IsEvaluable(const Standard_Integer theId);

  ActData_EXPORT virtual void
    DisconnectEvaluator(const Standard_Integer theId);

// Plain references:
public:

  ActData_EXPORT virtual void
    ConnectReference(const Standard_Integer theId,
                     const Handle(ActAPI_IDataCursor)& theTarget);

  ActData_EXPORT virtual Standard_Boolean
    HasConnectedReference(const Standard_Integer theId);

  ActData_EXPORT virtual void
    DisconnectReference(const Standard_Integer theId);

// Multiple references:
public:

  ActData_EXPORT virtual void
    ConnectReferenceToList(const Standard_Integer theId,
                           const Handle(ActAPI_IDataCursor)& theTarget,
                           const Standard_Integer theAfterIndex = 0);

  ActData_EXPORT virtual Standard_Boolean
    HasConnectedReferenceList(const Standard_Integer theId);

  ActData_EXPORT virtual void
    DisconnectReferenceList(const Standard_Integer theId);

  ActData_EXPORT virtual Standard_Boolean
    DisconnectReferenceFromList(const Standard_Integer theId,
                                const Standard_Integer theIndex);

// Back-references:
public:

  ActData_EXPORT virtual Handle(ActAPI_HParameterList)
    GetInputReaders();

  ActData_EXPORT virtual Handle(ActAPI_HParameterList)
    GetOutputWriters();

  ActData_EXPORT virtual Handle(ActAPI_HParameterList)
    GetReferrers();

protected:

  ActData_EXPORT
    ActData_BaseNode();

// Data Cursor internals:
protected:

  ActData_EXPORT void
    attach(const TDF_Label& theLabel,
           const Standard_Boolean isExpanding);

  ActData_EXPORT Standard_Boolean
    canSettleOn(const TDF_Label& theLabel);

  ActData_EXPORT virtual void
    expandOn(const TDF_Label& theLabel);

  ActData_EXPORT virtual void
    settleOn(const TDF_Label& theLabel);

// Construction internals:
protected:

  ActData_EXPORT void
    registerParameter(const Standard_Integer theId,
                      const Handle(ActAPI_IUserParameter)& theParam,
                      const Standard_Boolean isExpressible = Standard_False);

  ActData_EXPORT void
    remove(const Standard_Boolean canAffectExGraph);

  ActData_EXPORT virtual void
    beforeRemove();

  ActData_EXPORT virtual void
    beforeRemoveMyReference(const Handle(ActData_ReferenceParameter)& theReferrer);

  ActData_EXPORT virtual void
    beforeRemoveMyReference(const Handle(ActData_ReferenceListParameter)& theReferrer,
                            const Handle(ActAPI_IDataCursor)& theDC);

protected:

  //! WARBIBG: if you add any default user Parameters here, the grouping with
  //!          Group Parameter will become problematic as Group Parameters are
  //!          defined by user.
  enum UserParamId
  {
    UserParam_Undefined = 0,
    UserParam_Last = UserParam_Undefined + RESERVED_PARAM_RANGE
  };

protected:

  //! Observer types which can be connected with the Node.
  enum ObserverType
  {
    Observer_InputReaders = 1, //!< Tree Function Parameters.
    Observer_OutputWriters,    //!< Tree Function Parameters.
    Observer_Referrers         //!< Reference (including Lists) Parameters.
  };

// Tree Function & References internals:
protected:

  ActData_EXPORT void
    disconnectTreeFunction(const Standard_Integer theId,
                           const Standard_Boolean isInternal,
                           const Standard_Boolean doKillCompletely);

  ActData_EXPORT void
    disconnectTreeFunction(const Handle(ActData_TreeFunctionParameter)& thePFunc,
                           const Standard_Boolean                       doKillCompletely);

  ActData_EXPORT void
    connectReader(const Handle(ActData_TreeFunctionParameter)& theReader,
                  const Standard_Boolean                       isPrepend = Standard_False);

  ActData_EXPORT void
    connectWriter(const Handle(ActData_TreeFunctionParameter)& theWriter,
                  const Standard_Boolean                       isPrepend = Standard_False);

  ActData_EXPORT void
    connectReferrer(const Handle(ActAPI_IUserParameter)& theReferrer,
                    const Standard_Boolean               isPrepend = Standard_False);

  ActData_EXPORT void
    disconnectObserver(const ObserverType                   theObsType,
                       const Handle(ActAPI_IUserParameter)& theObserver);

  ActData_EXPORT void
    disconnectReader(const Handle(ActData_TreeFunctionParameter)& theReader);

  ActData_EXPORT void
    disconnectWriter(const Handle(ActData_TreeFunctionParameter)& theWriter);

  ActData_EXPORT void
    disconnectReferrer(const Handle(ActAPI_IUserParameter)& theReferrer);

  ActData_EXPORT void
    disconnectReferrerFor(const Handle(ActAPI_IUserParameter)& theReference,
                          const Handle(ActAPI_IDataCursor)&    theDC);

  ActData_EXPORT void
    disconnectReferrerSmart(const Handle(ActData_ReferenceListParameter)& theReferrer,
                            const Handle(ActAPI_INode)&                   theReferredNode);

// Internal accessors:
protected:

  Handle(ActAPI_HNodalParameterList)
    accessAllParameters();

  Handle(ActData_TreeFunctionParameter)
    accessFuncParameter(const Standard_Integer theId,
                        const Standard_Boolean isInternal);

  Handle(ActData_ReferenceParameter)
    accessRefParameter(const Standard_Integer theId);

  Handle(ActData_ReferenceListParameter)
    accessRefListParameter(const Standard_Integer theId);

  Handle(ActData_TreeNodeParameter)
    accessTreeNodeParameter(const Standard_Integer theId);

  Handle(ActAPI_HParameterList)
    accessObservers(const ObserverType theObsType);

protected:

  //! Slots for Parameters and relations between slots.
  ActData_ParameterScope m_paramScope;

  //! Label the Node is attached to (can be NULL in DETACHED mode).
  TDF_Label m_label;

};

#endif
