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
#include <ActData_RefClassifier.h>

// OCCT includes
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>

//! Performs the classification of the given reference against the passed
//! Sampler Tree.
//! \param theReference [in] reference to classify.
//! \param theSTree [in] Sampler Tree reference.
//! \return classification result.
ActData_RefClassifier::RefScope
  ActData_RefClassifier::Classify(const TDF_Label& theReference,
                                  const Handle(ActData_SamplerTreeNode)& theSTree)
{
  const ActData_SamplerTreeNode* aSTreePtr = reinterpret_cast<const ActData_SamplerTreeNode*>( theSTree.get() );
  return Classify(theReference, *aSTreePtr);
}

//! Performs the classification of the given reference against the passed
//! Sampler Tree.
//! \param theReference [in] reference to classify.
//! \param theSTree [in] Sampler Tree instance.
//! \return classification result.
ActData_RefClassifier::RefScope
  ActData_RefClassifier::Classify(const TDF_Label& theReference,
                                  const ActData_SamplerTreeNode& theSTree)
{
  Standard_Boolean isInScoped = Standard_False;
  for ( ActData_SamplerTreeNode::Iterator it(theSTree, Standard_True); it.More(); it.Next() )
  {
    const ActData_SamplerTreeNode* aSIDPtr = it.Current();
    TDF_Label aSLabel;
    TDF_Tool::Label(theReference.Data(), aSIDPtr->ID, aSLabel, Standard_False);

    if ( IsSameOrSubLabelOf(theReference, aSLabel) )
    {
      isInScoped = Standard_True;
      break;
    }
  }

  return (isInScoped ? RefScope_IN : RefScope_OUT);
}

//! Checks whether the first given Label is a sub-Label of the second one
//! inclusively.
//! \param theFirst [in] first Label.
//! \param theSecond [in] second Label.
//! \return true/false.
Standard_Boolean
  ActData_RefClassifier::IsSameOrSubLabelOf(const TDF_Label& theFirst,
                                            const TDF_Label& theSecond)
{
  TCollection_AsciiString aFirstEntry = ActData_Utils::GetEntry(theFirst);
  TCollection_AsciiString aSecondEntry = ActData_Utils::GetEntry(theSecond);

  if ( ::IsEqual(aFirstEntry, aSecondEntry) )
    return Standard_True;

  for ( TDF_ChildIterator it(theSecond, Standard_True); it.More(); it.Next() )
  {
    TCollection_AsciiString aSecondSubEntry = ActData_Utils::GetEntry( it.Value() );
    if ( ::IsEqual(aFirstEntry, aSecondSubEntry) )
      return Standard_True;
  }

  return Standard_False;
}
