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
// Web: http://analysissitus.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_BasePartition.h>

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_Utils.h>

// OCCT includes
#include <TDF_Tool.hxx>

namespace
{
  bool IsEmpty(const TDF_Label& lab)
  {
    if ( lab.HasAttribute() )
      return false;

    bool emptyChildren = true;
    for ( TDF_ChildIterator cit(lab, false); cit.More(); cit.Next() )
    {
      TDF_Label childLab = cit.Value();

      if ( !IsEmpty(childLab) )
      {
        emptyChildren = false;
        break;
      }
    }

    return emptyChildren;
  }
}

/* =========================================================================
 *  Class: ActData_IPartition
 *  Subclass: Iterator
 * ========================================================================= */

ActData_BasePartition::Iterator::Iterator(const Handle(ActAPI_IPartition)& theIP)
{
  this->Init(theIP);
}

void ActData_BasePartition::Iterator::Init(const Handle(ActAPI_IPartition)& theIP)
{
  m_partition = Handle(ActData_BasePartition)::DownCast(theIP);

  // Throw out all "ghost" Labels
  m_labelList.Clear();
  for ( TDF_ChildIterator cit(m_partition->m_label); cit.More(); cit.Next() )
  {
    const TDF_Label aLab = cit.Value();
    if ( ActData_NodeFactory::IsNode(aLab) )
      m_labelList.Append(aLab);
  }

  // Initialize internal iterator
  m_it.Initialize(m_labelList);
}

//! Returns true if next element exists, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_BasePartition::Iterator::More() const
{
  return m_it.More();
}

//! Moves iterator to the next element.
void ActData_BasePartition::Iterator::Next()
{
  return m_it.Next();
}

Handle(ActAPI_INode) ActData_BasePartition::Iterator::Value() const
{
  return ActData_NodeFactory::NodeSettle( m_it.Value() );
}

/* =========================================================================
 *  Class: ActData_BasePartition
 * ========================================================================= */

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------

ActData_BasePartition::ActData_BasePartition() : ActAPI_IPartition()
{}

//-----------------------------------------------------------------------------
// Transient properties of Data Cursor
//-----------------------------------------------------------------------------

Standard_Boolean ActData_BasePartition::IsAttached()
{
  return !m_label.IsNull();
}

Standard_Boolean ActData_BasePartition::IsDetached()
{
  return !this->IsAttached();
}

Standard_Boolean ActData_BasePartition::IsWellFormed()
{
  // TODO: not actually implemented
  return Standard_True;
}

Standard_Boolean ActData_BasePartition::IsValidData()
{
  return Standard_True;
}

Standard_Boolean ActData_BasePartition::IsPendingData()
{
  return Standard_False;
}

ActAPI_DataObjectId ActData_BasePartition::GetId() const
{
  return ActData_Utils::GetEntry(m_label);
}

//-----------------------------------------------------------------------------
// Management of Data Nodes
//-----------------------------------------------------------------------------

ActAPI_DataObjectId ActData_BasePartition::AddNode(const Handle(ActAPI_INode)& N)
{
  // Check type consistency
  TCollection_AsciiString nodeType    = N->DynamicType()->Name();
  TCollection_AsciiString allowedType = TCollection_AsciiString( this->GetNodeType()->Name() );
  //
  if ( !::IsEqual(nodeType, allowedType) )
    Standard_ProgramError::Raise("Unexpected Node type");

  // Find a label to be a root for the Node.
  TDF_Label             nodeLab;
  Handle(TDF_TagSource) tagSourceAttr;
  //
  if ( !m_label.FindAttribute(TDF_TagSource::GetID(), tagSourceAttr) )
  {
    nodeLab = TDF_TagSource::NewChild(m_label);
  }
  else
  {
    // Try to find any "ghost" label.
    for ( TDF_ChildIterator cit(m_label, false); cit.More(); cit.Next() )
    {
      TDF_Label labCandidate = cit.Value();

      if ( ::IsEmpty(labCandidate) )
      {
        nodeLab = labCandidate;
        break;
      }
    }

    if ( nodeLab.IsNull() )
      nodeLab = TDF_TagSource::NewChild(m_label);
  }

  // Create a new Node on the found anchoring label.
  Handle(ActData_BaseNode)::DownCast(N)->expandOn(nodeLab);

  // Return Node ID.
  return ActData_Utils::GetEntry(nodeLab);
}

Standard_Boolean
  ActData_BasePartition::GetNode(const Standard_Integer theNodeNum,
                                 const Handle(ActAPI_INode)& theNode) const
{
  TDF_Label aNodeLabel = m_label.FindChild(theNodeNum, Standard_False);

  if ( aNodeLabel.IsNull() )
    return Standard_False;

  Handle(ActData_BaseNode)::DownCast(theNode)->settleOn(aNodeLabel);

  return Standard_True;
}

Handle(ActAPI_INode)
  ActData_BasePartition::GetNode(const Standard_Integer theNodeNum) const
{
  TDF_Label aNodeLabel = m_label.FindChild(theNodeNum, Standard_False);

  if ( aNodeLabel.IsNull() )
    return NULL;

  return ActData_NodeFactory::NodeSettle(aNodeLabel);
}

//-----------------------------------------------------------------------------
// Accessors for OCAF internal objects
//-----------------------------------------------------------------------------

//! Accessor for the root Label of Partition.
//! \return root Label of Partition.
TDF_Label ActData_BasePartition::RootLabel() const
{
  return m_label;
}

//-----------------------------------------------------------------------------
// Data Cursor internals
//-----------------------------------------------------------------------------

//! Attaches the transient Data Cursor to the CAF Label.
//! \param theLabel [in] Label to attach the cursor to.
void ActData_BasePartition::attach(const TDF_Label& theLabel)
{
  m_label = theLabel;
}

//! Expands the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to expand on.
void ActData_BasePartition::expandOn(const TDF_Label& theLabel)
{
  this->attach(theLabel);
}

//! Settles the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to settle on.
void ActData_BasePartition::settleOn(const TDF_Label& theLabel)
{
  this->attach(theLabel);
}
