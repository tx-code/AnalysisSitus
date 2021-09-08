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

#ifndef ActData_SamplerTreeNode_HeaderFile
#define ActData_SamplerTreeNode_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_Utils.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

DEFINE_STANDARD_HANDLE(ActData_SamplerTreeNode, Standard_Transient)

//! Represents Sampler Tree of IDs used as a pattern for recovering
//! domain-specific connectivity between Data Nodes.
class ActData_SamplerTreeNode : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_SamplerTreeNode, Standard_Transient)

public:

  //! Iterator for ID tree.
  class Iterator
  {
  public:

    //! Constructor accepting the root node of the tree to iterate over.
    //! \param theNode [in] the root node.
    //! \param isAllLevels [in] indicates whether the full LEFT traversal
    //!        must be performed (TRUE) or only the direct children of the
    //!        target root are the subject of iteration.
    Iterator(const ActData_SamplerTreeNode& theNode,
             const Standard_Boolean isAllLevels = Standard_False)
    {
      this->Init(theNode, isAllLevels);
    }

    //! Initializes iterator with the root node of the tree.
    //! \param theNode [in] the root node.
    //! \param isAllLevels [in] indicates whether the full LEFT traversal
    //!        must be performed (TRUE) or only the direct children of the
    //!        target root are the subject of iteration.
    void Init(const ActData_SamplerTreeNode& theNode,
              const Standard_Boolean isAllLevels = Standard_False)
    {
      m_pRoot = &theNode;
      m_bAllLevels = isAllLevels;
      m_pCurrent = const_cast<ActData_SamplerTreeNode*>(m_pRoot);
    }

    //! Checks whether next item exists.
    //! \return true/false.
    Standard_Boolean More() const
    {
      return m_pCurrent != nullptr;
    }

    //! Moves iterator to the next item in the tree.
    void Next()
    {
      // Move down if it is ALLOWED and POSSIBLE
      if ( m_bAllLevels && !m_pCurrent->Children.IsEmpty() )
      {
        m_pCurrent = &m_pCurrent->Children.ChangeValue(1);
        return;
      }

      // Move forward if possible (bottom level is already reached)
      if ( m_pCurrent->NextSiblingPtr )
      {
        m_pCurrent = m_pCurrent->NextSiblingPtr;
      }
      // If the ultimate child in a branch is reached, rollback to
      // the father node attempting to find non-traversed sibling
      else
      {
        do
        {
          m_pCurrent = m_pCurrent->ParentPtr;
        }
        while ( m_pCurrent && !m_pCurrent->NextSiblingPtr );
        if ( m_pCurrent )
          m_pCurrent = m_pCurrent->NextSiblingPtr;
      }
    }

    //! Returns current item.
    const ActData_SamplerTreeNode* Current()
    {
      return m_pCurrent;
    }

  private:

    //! Root node of the tree being iterated.
    const ActData_SamplerTreeNode* m_pRoot;

    //! Currently iterated tree node.
    ActData_SamplerTreeNode* m_pCurrent;

    //! Indicates whether the iterator must visit only the direct children
    //! of the target root (FALSE) or traverse the nested nodes as
    //! well (TRUE).
    Standard_Boolean m_bAllLevels;

  };

public:

  ActData_EXPORT
    ActData_SamplerTreeNode( const ActAPI_DataObjectId& theID = ActAPI_DataObjectId() );

  ActData_EXPORT ActData_SamplerTreeNode&
    NewChild();

public:

  ActData_EXPORT static Handle(ActData_SamplerTreeNode)
    Build(const Handle(ActAPI_INode)& theNode);

public:

  //! Child IDs.
  NCollection_Sequence<ActData_SamplerTreeNode> Children;

  //! Stored ID.
  ActAPI_DataObjectId ID;

  //! Pointer to the parent node.
  ActData_SamplerTreeNode* ParentPtr;

  //! Pointer to the next sibling node.
  ActData_SamplerTreeNode* NextSiblingPtr;

private:

  static void build(const Handle(ActAPI_INode)& theNode,
                    ActData_SamplerTreeNode& theSNode);

};

#endif
