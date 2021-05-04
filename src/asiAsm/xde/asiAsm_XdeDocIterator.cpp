//-----------------------------------------------------------------------------
// Created on: 22 September 2020 (*)
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

// Own include
#include <asiAsm_XdeDocIterator.h>

// OCCT includes
#include <TDF_LabelSequence.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

DocIterator::DocIterator(const Handle(Doc)& asmDoc,
                         const int          level)
: m_asmDoc(asmDoc), m_iMaxLevel(level)
{
  // We start from those shapes which are "free" in terms of XDE.
  TDF_LabelSequence roots;
  //
  m_asmDoc->GetShapeTool()->GetFreeShapes(roots);
  //
  for ( int l = 1; l <= roots.Length(); ++l )
    m_fringe.push( this->createItem( roots(l) ) );
}

//-----------------------------------------------------------------------------

DocIterator::DocIterator(const AssemblyItemId& root,
                         const Handle(Doc)&    asmDoc,
                         const int             level)
: m_asmDoc(asmDoc), m_iMaxLevel(level)
{
  TDF_Label original;
  AssemblyItemId seed(root);
  seed.m_label = m_asmDoc->GetLabel(root);

  if ( seed.m_label.IsNull() )
    return;

  if ( m_asmDoc->IsInstance(seed.m_label, original) )
  {
    if ( m_asmDoc->IsPart(seed.m_label) || !m_asmDoc->IsAssembly(seed.m_label ))
    {
      seed.m_jumpLabel = seed.m_label;
      seed.m_label     = original;
    }
    else
    {
      seed.Remove( seed.GetPathLength() ); // Take father to feed iterator properly.
    }
  }

  // Initialize iterator.
  m_fringe.push(seed);
}

//-----------------------------------------------------------------------------

bool DocIterator::More() const
{
  return !m_fringe.empty();
}

//-----------------------------------------------------------------------------

void DocIterator::Next()
{
  // Let's throw an exception if there is nothing else to iterate.
  if ( !this->More() )
    Standard_ProgramError::Raise("No next item");

  // Take current.
  AssemblyItemId current = this->Current();
  m_fringe.pop(); // Top item is done.

  // Check current depth of iteration (root level is 0-level by convention).
  const int currentDepth = current.GetPathLength() - 1;

  if ( currentDepth < m_iMaxLevel )
  {
    // If current item is an assembly, then the next items to iterate in
    // depth-first order are the components of this assembly.
    TDF_LabelSequence components;
    if ( m_asmDoc->GetShapeTool()->IsAssembly(current.m_label) )
    {
      m_asmDoc->GetShapeTool()->GetComponents(current.m_label, components);
    }
    else if ( m_asmDoc->GetShapeTool()->IsComponent(current.m_label) )
    {
      components.Append(current.m_label);
    }

    // Put all labels pending for iteration to the fringe.
    for ( int l = components.Length(); l >= 1; --l )
    {
      TDF_Label      IL    = components(l); // Insertion-level label.
      AssemblyItemId IItem = this->createItem(IL);
      //
      for ( int k = current.GetPathLength(); k >= 1; --k )
        IItem.Prepend( current(k) );

      // Set item to iterate.
      m_fringe.push(IItem);
    }
  }
}

//-----------------------------------------------------------------------------

AssemblyItemId DocIterator::Current() const
{
  return m_fringe.top();
}

//-----------------------------------------------------------------------------

AssemblyItemId
  DocIterator::createItem(const TDF_Label& L) const
{
  AssemblyItemId result;

  // Populate label and entry.
  result.m_label = L;
  TDF_Tool::Entry(L, result.m_entry);
  //
  result << result.m_entry;

  TDF_Label origin;
  if ( m_asmDoc->IsInstance(result.m_label, origin) )
  {
    // Fill jump item properly.
    result.m_jumpLabel = result.m_label;
    result.m_label     = origin;
    //
    TDF_Tool::Entry(origin, result.m_entry);
  }

  return result;
}
