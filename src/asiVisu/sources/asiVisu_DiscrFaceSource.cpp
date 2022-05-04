//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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
#include <asiVisu_DiscrFaceSource.h>

// VTK includes
#include <vtkPointData.h>

// asiAlgo includes
#include <asiAlgo_DiscrFace.h>
#include <asiAlgo_DiscrSequenceOfPointer.h>
#include <asiAlgo_DiscrWire.h>

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_DiscrFaceSource)

//-----------------------------------------------------------------------------

asiVisu_DiscrFaceSource::asiVisu_DiscrFaceSource()
: vtkPolyDataAlgorithm( ),
  m_faceID(-1)
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_DiscrFaceSource::~asiVisu_DiscrFaceSource()
{}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFaceSource::SetDiscrModel(const Handle(asiAlgo::discr::Model)& model)
{
  m_discrModel = model;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo::discr::Model)&
  asiVisu_DiscrFaceSource::GetDiscrModel() const
{
  return m_discrModel;
}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFaceSource::SetFaceIDs(const TColStd_PackedMapOfInteger& faceID)
{
  m_faceID = faceID;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

const TColStd_PackedMapOfInteger& asiVisu_DiscrFaceSource::GetFaceIDs() const
{
  return m_faceID;
}

//-----------------------------------------------------------------------------

int asiVisu_DiscrFaceSource::RequestData(vtkInformation*        asiVisu_NotUsed(request),
                                         vtkInformationVector** asiVisu_NotUsed(inputVector),
                                         vtkInformationVector*  outputVector)
{
  if ( m_discrModel.IsNull() )
  {
    vtkErrorMacro(<< "Invalid input: null discrete model.");
    return 0;
  }

  /* ================
   *  Prepare output.
   * ================ */

  vtkPolyData* pOutputPolyData = vtkPolyData::GetData(outputVector);
  pOutputPolyData->Allocate();
  //
  vtkSmartPointer<vtkPoints> pOutputPts = vtkSmartPointer<vtkPoints>::New();
  pOutputPolyData->SetPoints(pOutputPts);

  /* =============
   *  Get polylines
   * ============= */

  const Handle(asiVisu_ShapeData)& shapeData = generatePolyLine();
  if ( shapeData.IsNull() )
  {
    vtkErrorMacro(<< "Invalid input: null data.");
    return 0;
  }

  const vtkSmartPointer<vtkPolyData>& resultPolyData = shapeData->GetPolyData();

  /* ==========
   *  Finalize
   * ========== */

  pOutputPolyData->CopyStructure(resultPolyData);  // Copy points and cells
  pOutputPolyData->CopyAttributes(resultPolyData); // Copy data arrays

  return 1;
}

//-----------------------------------------------------------------------------

Handle(asiVisu_ShapeData) asiVisu_DiscrFaceSource::generatePolyLine() const
{
  Handle(asiVisu_ShapeData) data = new asiVisu_ShapeData();

  int shapeId = -1;

  int nbFaces = m_discrModel->GetNbFaces();
  for ( int iFace = 1; iFace <= nbFaces; ++iFace )
  {
    const asiAlgo::discr::Face& face = m_discrModel->GetFace(iFace);
    int nbWires = face.GetNbWires();
    for ( int iWire = 1; iWire <= nbWires; iWire++ )
    {
      const asiAlgo::discr::Wire& wire = face.GetWire(iWire);
      const int nbEdges = wire.GetNbEdges();
      for ( int iEdge = 1; iEdge <= nbEdges; iEdge++ )
      {
        vtkSmartPointer<vtkIdList> pids = vtkSmartPointer<vtkIdList>::New();

        const asiAlgo::discr::PairOfPEdgeBoolean& edgeData = wire.GetEdgeData(iEdge);
        const asiAlgo::discr::Edge& edge = *edgeData.first;
        const asiAlgo::discr::Curve& curve = edge.GetCurve();
        const int& nbPoints = curve.NbPoints();
        for ( int iPoint = 1; iPoint <= nbPoints; iPoint++ )
        {
          const gp_Pnt& point = curve.Point(iPoint);
          vtkIdType pid = data->InsertCoordinate(point.X(), point.Y(), point.Z());
          pids->InsertNextId(pid);
        }

        data->InsertPolyline((vtkIdType)(++shapeId), pids, ShapePrimitive_BorderEdge);
      }
    }
  }

  return data;
}
