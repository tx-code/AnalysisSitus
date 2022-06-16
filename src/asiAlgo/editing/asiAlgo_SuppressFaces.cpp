//-----------------------------------------------------------------------------
// Created on: 16 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <asiAlgo_SuppressFaces.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

asiAlgo_SuppressFaces::asiAlgo_SuppressFaces()
{
  m_reShape = new BRepTools_ReShape();
  m_reShape->ModeConsiderLocation() = 0; // We do not care of location today.
}

//-----------------------------------------------------------------------------

asiAlgo_SuppressFaces::asiAlgo_SuppressFaces(const TopoDS_Shape& masterCAD)
{
  m_master  = masterCAD;
  m_reShape = new BRepTools_ReShape();
  m_reShape->ModeConsiderLocation() = 0; // We do not care of location today.
}

//-----------------------------------------------------------------------------

asiAlgo_SuppressFaces::asiAlgo_SuppressFaces(const Handle(asiAlgo_AAG)& aag)
{
  this->SetAAG(aag);

  m_reShape = new BRepTools_ReShape();
  m_reShape->ModeConsiderLocation() = 0; // We do not care of location today.
}

//-----------------------------------------------------------------------------

bool asiAlgo_SuppressFaces::Perform(const asiAlgo_Feature& faceIndices,
                                    const bool             facesOnly)
{
  // Get indices of faces.
  if ( m_faces.IsEmpty() )
  {
    if ( m_aag.IsNull() )
      TopExp::MapShapes(m_master, TopAbs_FACE, m_faces);
    else
      m_faces = m_aag->GetMapOfFaces();
  }

  // Remove requested faces.
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(faceIndices); fit.More(); fit.Next() )
  {
    const int face_idx = fit.Key();
    //
    if ( face_idx < 1 || face_idx > m_faces.Extent() )
      continue;

    const TopoDS_Shape& face = m_faces(face_idx);

    m_reShape->Remove(face);

    if ( !facesOnly )
    {
      // Kill wires.
      for ( TopExp_Explorer exp(face, TopAbs_WIRE); exp.More(); exp.Next() )
      {
        m_reShape->Remove( exp.Current() );
      }

      // Kill edges.
      for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
      {
        m_reShape->Remove( exp.Current() );
      }

      // Kill vertices.
      for ( TopExp_Explorer exp(face, TopAbs_VERTEX); exp.More(); exp.Next() )
      {
        m_reShape->Remove( exp.Current() );
      }
    }
  }
  m_result = m_reShape->Apply(m_master);

  // For some reason, Apply() may not be effective and return null shape out
  // of the non-null input. Let's do this final judgement over here to prevent
  // returning 'true' in such situations...
  return !m_result.IsNull();
}
