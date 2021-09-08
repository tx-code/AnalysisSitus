//-----------------------------------------------------------------------------
// Created on: May 2012
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
#include <ActData_NodeFactory.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_AsciiString.hxx>

ActData_NodeFactory::NodeAllocMap ActData_NodeFactory::m_allocMap;

//! Pushes Node type into the global static registry of Nodal factory
//! methods.
//! \param theType      [in] Node type to register.
//! \param theAllocFunc [in] factory function for the given Node type.
//! \return global static registry for convenience.
const ActData_NodeFactory::NodeAllocMap&
  ActData_NodeFactory::RegisterNodeType(const TCollection_AsciiString& theType,
                                        const ActData_NodeAllocator&   theAllocFunc)
{
  if ( !m_allocMap.IsBound(theType) )
    m_allocMap.Bind(theType, theAllocFunc);

  return m_allocMap;
}

//! Returns global static registry of Nodal factory methods.
//! \return Nodal registry.
const ActData_NodeFactory::NodeAllocMap& ActData_NodeFactory::GetAllocMap()
{
  return m_allocMap;
}

//! Checks whether the given Label represents Nodal data or not.
//! \param theLab      [in]  raw OCAF Label to check.
//! \param theNodeType [out] detected Node type if any.
//! \return true/false.
Standard_Boolean
  ActData_NodeFactory::IsNode(const TDF_Label&         theLab,
                              TCollection_AsciiString& theNodeType)
{
  if ( theLab.IsNull() )
    return Standard_False;

  // Attempt to access META container
  TDF_Label aNodeMeta =
    theLab.FindChild(ActData_BaseNode::TagInternal, Standard_False);
  //
  if ( aNodeMeta.IsNull() )
    return Standard_False;

  // Attempt to access ASCII String attribute which is designed to contain
  // the type of the Node
  Handle(TDataStd_AsciiString) aTypeNameAttr;
  if ( !aNodeMeta.FindAttribute(TDataStd_AsciiString::GetID(), aTypeNameAttr) )
    return Standard_False;

  // Access type name and set it as output
  theNodeType = aTypeNameAttr->Get();
  //
  return Standard_True;
}

//! Checks whether the given Label represents Nodal data or not.
//! \param theLab [in] raw Label to check.
//! \return true/false.
Standard_Boolean ActData_NodeFactory::IsNode(const TDF_Label& theLab)
{
  TCollection_AsciiString aTypeName;
  Standard_Boolean isStructureOk = IsNode(theLab, aTypeName);

  if ( !isStructureOk )
    return Standard_False;

  return m_allocMap.IsBound(aTypeName);
}

//! Creates a DETACHED instance of Data Node Cursor of the given type.
//! \param theNodeType [in] type of Data Node to create.
//! \return Data Node instance.
Handle(ActAPI_INode)
  ActData_NodeFactory::NodeInstanceByType(const TCollection_AsciiString& theNodeType)
{
  const ActData_NodeAllocator& anAllocFunc = m_allocMap.Find(theNodeType);
  return (*anAllocFunc)();
}

//! Checks whether a Nodal instance can be safely settled onto the given
//! Label.
//! \param theLab [in] Label to check.
//! \return true/false.
Standard_Boolean ActData_NodeFactory::CanSettleNode(const TDF_Label& theLab)
{
  /* =======================
   *  Check persistent data
   * ======================= */

  TCollection_AsciiString aTypeName;
  if ( !IsNode(theLab, aTypeName) )
    return Standard_False;

  if ( !m_allocMap.IsBound(aTypeName) )
    Standard_ProgramError::Raise("RTTI is not registered");

  /* =====================================================
   *  Access Node allocator and create the requested Node
   * ===================================================== */

  Handle(ActData_BaseNode) aResult = Handle(ActData_BaseNode)::DownCast( NodeInstanceByType(aTypeName) );
  return aResult->canSettleOn(theLab);
}

//! Settles down a Node to the given Label. If the passed Label does not
//! contain Nodal data, an exception is thrown. This method is READ-ONLY.
//! \param theLab [in] raw Label to settle down the Node to.
//! \return Node instance.
Handle(ActAPI_INode) ActData_NodeFactory::NodeSettle(const TDF_Label& theLab)
{
  /* =======================
   *  Check persistent data
   * ======================= */

  TCollection_AsciiString aTypeName;
  if ( !IsNode(theLab, aTypeName) )
    return NULL;

  if ( !m_allocMap.IsBound(aTypeName) )
    Standard_ProgramError::Raise("RTTI is not registered");

  /* =====================================================
   *  Access Node allocator and create the requested Node
   * ===================================================== */

  Handle(ActData_BaseNode) aResult = Handle(ActData_BaseNode)::DownCast( NodeInstanceByType(aTypeName) );
  aResult->settleOn(theLab);

  return aResult;
}

//! Settles down a Node to the given Label without any checks.
//! This method is READ-ONLY.
//! \param theNode [in] Node instance
//! \param theLab  [in] raw Label to settle down the Node to.
void ActData_NodeFactory::NodeSettle(const Handle(ActData_BaseNode)& theNode,
                                     const TDF_Label&                theLab)
{
  theNode->settleOn(theLab);
}

//! Settles down a Node by the given Parameter's Label. This method is
//! READ-ONLY.
//! \param theParamLabel [in] Parameter's root Label.
//! \return Node instance.
Handle(ActAPI_INode)
  ActData_NodeFactory::NodeByParamSettle(const TDF_Label& theParamLabel)
{
  // NOTICE: User Parameters may be used in META section. If it happens,
  //         then Father() should be called three times

  // Two levels up: check if we have reached META root
  TDF_Label aNodeLabel = theParamLabel.Father().Father();
  //
  Handle(TDataStd_TreeNode) TN;
  if ( aNodeLabel.FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), TN) )
    aNodeLabel = aNodeLabel.Father();

  // Settle Node
  return NodeSettle(aNodeLabel);
}

//! Settles down a Node by the given Parameter Label. This method is
//! READ-ONLY.
//! \param theParam [in] Parameter.
//! \return Node instance.
Handle(ActAPI_INode)
  ActData_NodeFactory::NodeByParamSettle(const Handle(ActAPI_IUserParameter)& theParam)
{
  return NodeByParamSettle( theParam->RootLabel() );
}
