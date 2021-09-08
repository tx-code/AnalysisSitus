//-----------------------------------------------------------------------------
// Created on: November 2012
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
#include <ActData_SamplerTreeNode.h>

//! Constructor accepting entity ID.
//! \param theID [in] entity ID.
ActData_SamplerTreeNode::ActData_SamplerTreeNode(const ActAPI_DataObjectId& theID)
: Standard_Transient(),
  ID(theID),
  ParentPtr(NULL),
  NextSiblingPtr(NULL)
{}

//! Adds new tree node into collection of children.
//! \return reference to the added tree node.
ActData_SamplerTreeNode& ActData_SamplerTreeNode::NewChild()
{
  Children.Append( ActData_SamplerTreeNode() );
  Standard_Integer aLastIndex = Children.Length();
  ActData_SamplerTreeNode& aNewChild = Children.ChangeValue(aLastIndex);

  // Set traversal references
  aNewChild.ParentPtr = this;
  if ( aLastIndex > 1 )
  {
    ActData_SamplerTreeNode& aPrevChild = Children.ChangeValue(aLastIndex - 1);
    aPrevChild.NextSiblingPtr = &aNewChild;
  }

  return aNewChild;
}

//! Builds Sampler Tree model starting from the given Data Node.
//! \param theNode [in] root Node for the modeled branch.
//! \return resulting Sampler Tree.
Handle(ActData_SamplerTreeNode)
  ActData_SamplerTreeNode::Build(const Handle(ActAPI_INode)& theNode)
{
  Handle(ActData_SamplerTreeNode) aResult = new ActData_SamplerTreeNode;
  ActData_SamplerTreeNode* aResultPtr = reinterpret_cast<ActData_SamplerTreeNode*>( aResult.get() );
  build(theNode, *aResultPtr);
  return aResult;
}

//! Internal Sampler Tree construction routine.
//! \param theNode [in] currently processed Data Node.
//! \param theSNode [in/out] Sample Tree Node representation of the currently
//!        processed Data Node.
void ActData_SamplerTreeNode::build(const Handle(ActAPI_INode)& theNode,
                                    ActData_SamplerTreeNode& theSNode)
{
  theSNode.ID = theNode->GetId();
  for ( Handle(ActAPI_IChildIterator) it = theNode->GetChildIterator(); it->More(); it->Next() )
  {
    Handle(ActAPI_INode) aChild = it->Value();
    build( aChild, theSNode.NewChild() );
  }
}
