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

#ifndef ActData_UniqueNodeName_HeaderFile
#define ActData_UniqueNodeName_HeaderFile

// Active Data includes
#include <ActData.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

#define NAMEGEN_NONAME "Untitled"

//-----------------------------------------------------------------------------
// Provider of sibling Nodes
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_SiblingNodes, Standard_Transient)

//! \ingroup AD_DF
//!
//! Wraps sibling Nodes for the given one.
class ActData_SiblingNodes : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_SiblingNodes, Standard_Transient)

public:

  ActData_EXPORT static Handle(ActData_SiblingNodes)
    CreateForRecord(const Handle(ActAPI_INode)& theRecord,
                    const Handle(ActAPI_INode)& theOwner,
                    const Standard_Integer theCollectionPID);

  ActData_EXPORT static Handle(ActData_SiblingNodes)
    CreateForChild(const Handle(ActAPI_INode)& theChild,
                   const Handle(ActAPI_INode)& theOwner);

public:

  //! Returns the number of siblings.
  //! \return number of siblings.
  inline Standard_Integer NumberOfNodes() const
  {
    return m_siblings->Length();
  }

  //! Returns sibling Node with the given 1-based index.
  //! \param theIdx [in] 1-based index of sibling to access.
  //! \return requested Node.
  inline const Handle(ActAPI_INode)& At(const Standard_Integer theIdx) const
  {
    return m_siblings->Value(theIdx);
  }

protected:

  ActData_EXPORT ActData_SiblingNodes&
    operator<<(const Handle(ActAPI_INode)& theSibling);

protected:

  ActData_EXPORT
    ActData_SiblingNodes(const Handle(ActAPI_INode)& theItem,
                        const Handle(ActAPI_INode)& theOwner);

protected:

  Handle(ActAPI_INode)    m_owner;    //!< Owner.
  Handle(ActAPI_INode)    m_item;     //!< Selected item to get siblings for.
  Handle(ActAPI_HNodeList) m_siblings; //!< Collection of siblings.

};

//-----------------------------------------------------------------------------
// Generator
//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Unique name generator for Nodes.
class ActData_UniqueNodeName
{
public:

  ActData_EXPORT static TCollection_ExtendedString
    Generate(const Handle(ActData_SiblingNodes)& theNodes,
             const TCollection_ExtendedString& theBaseName = NAMEGEN_NONAME);

private:

  ActData_UniqueNodeName() {}
  ActData_UniqueNodeName(const ActData_UniqueNodeName&) {}

};

#endif
