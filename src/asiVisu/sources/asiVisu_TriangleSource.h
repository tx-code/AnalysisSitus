//-----------------------------------------------------------------------------
// Created on: 20 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Julia Slyadneva
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

#pragma once

// asiVisu includes
#include <asiVisu.h>

// VTK includes
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_Mesh.h>
#endif

//! Source of polygonal data representing surface triangulation.
class asiVisu_TriangleSource : public vtkPolyDataAlgorithm
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_TriangleSource, vtkPolyDataAlgorithm);

  asiVisu_EXPORT static asiVisu_TriangleSource*
    New();

public:

  //! Sets triangulation to visualize.
  //! \param[in] triangulation the triangulation to visualize.
  asiVisu_EXPORT void
    SetPoints(const vtkSmartPointer<vtkPoints>& points);

protected:

  //! This method (called by superclass) performs conversion of OCCT
  //! data structures to VTK polygonal representation.
  //!
  //! \param request      [in]  describes "what" algorithm should do. This is
  //!                           typically just one key such as REQUEST_INFORMATION.
  //! \param inputVector  [in]  inputs of the algorithm.
  //! \param outputVector [out] outputs of the algorithm.
  //! \return status.
  asiVisu_EXPORT virtual int
    RequestData(vtkInformation*        request,
                vtkInformationVector** inputVector,
                vtkInformationVector*  outputVector);

private:

  //! Creates or takes an existing mesh node by its ID.
  //!
  //! \param nodeID   [in]     node ID.
  //! \param polyData [in,out] output polygonal data.
  //! \return internal VTK ID for the corresponding point.
  vtkIdType
    findMeshNode(const int    nodeID,
                 vtkPolyData* polyData);

  //! Adds the mesh element with to VTK polygonal data.
  //!
  //! \param elemId   [in]     1-based element ID in the original Poly_Triangulation.
  //! \param nodeID1  [in]     ID of the 1-st node.
  //! \param nodeID2  [in]     ID of the 2-nd node.
  //! \param nodeID3  [in]     ID of the 3-rd node.
  //! \param polyData [in,out] polygonal data being populated.
  //! \return ID of the just added VTK cell.
  vtkIdType
    registerFacet(const int    elemId,
                  const int    nodeID1,
                  const int    nodeID2,
                  const int    nodeID3,
                  vtkPolyData* polyData);

private:

  //! Default constructor.
  asiVisu_TriangleSource();

  //! Destructor.
  ~asiVisu_TriangleSource();

private:

  asiVisu_TriangleSource(const asiVisu_TriangleSource&);
  asiVisu_TriangleSource& operator=(const asiVisu_TriangleSource&);

private:

  //! Registered VTK points.
  vtkSmartPointer<vtkPoints> m_points;
};

