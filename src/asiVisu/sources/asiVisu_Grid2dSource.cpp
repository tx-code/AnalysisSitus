//-----------------------------------------------------------------------------
// Created on: 31 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiVisu_Grid2dSource.h>

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_ProjectPointOnMesh.h>

// asiVisu includes
#include <asiVisu_MeshUtils.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_DistanceField.h>
  #include <mobius/poly_SVO.h>
#endif

//-----------------------------------------------------------------------------

namespace
{
  bool IsIn(const double sc[4])
  {
    for ( size_t k = 0; k < 4; ++k )
      if ( sc[k] > 0 )
        return false;

    return true;
  }

  bool IsOut(const double sc[4])
  {
    for ( size_t k = 0; k < 4; ++k )
      if ( sc[k] < 0 )
        return false;

    return true;
  }

  bool IsZeroCrossing(const double sc[4])
  {
    return !IsIn(sc) && !IsOut(sc);
  }
}

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_Grid2dSource)

//-----------------------------------------------------------------------------

asiVisu_Grid2dSource::asiVisu_Grid2dSource()
: vtkUnstructuredGridAlgorithm ( ),
  m_fMinScalar                 ( DBL_MAX    ),
  m_fMaxScalar                 (-DBL_MAX    ),
  m_fMinVoxelSize              ( DBL_MAX    ),
  m_strategy                   ( SS_OnInOut ) // all
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_Grid2dSource::~asiVisu_Grid2dSource()
{}

//-----------------------------------------------------------------------------

void asiVisu_Grid2dSource::SetInputGrid(const Handle(asiAlgo_UniformGrid<float>)& grid)
{
  m_grid = grid;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo_UniformGrid<float>)&
  asiVisu_Grid2dSource::GetInputGrid() const
{
  return m_grid;
}

//-----------------------------------------------------------------------------

void asiVisu_Grid2dSource::SetSamplingStrategy(const int strategy)
{
  m_strategy = strategy;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

int asiVisu_Grid2dSource::GetSamplingStrategy() const
{
  return m_strategy;
}

//-----------------------------------------------------------------------------

int asiVisu_Grid2dSource::RequestData(vtkInformation*        asiVisu_NotUsed(request),
                                      vtkInformationVector** asiVisu_NotUsed(inputVector),
                                      vtkInformationVector*  outputVector)
{
  if ( m_grid.IsNull() )
  {
    vtkErrorMacro( << "Invalid input: null grid." );
    return 0;
  }

  m_fMinScalar =  DBL_MAX;
  m_fMaxScalar = -DBL_MAX;

  /* ================
   *  Prepare output.
   * ================ */

  // Get the output unstructured grid data from the information vector.
  vtkUnstructuredGrid* pOutputGrid = vtkUnstructuredGrid::GetData(outputVector);
  pOutputGrid->Allocate();
  pOutputGrid->SetPoints( vtkSmartPointer<vtkPoints>::New() );

  /* ===========
   *  Add cells.
   * =========== */

  // Prepare array for nodal scalars.
  vtkPointData*                   pPointData = pOutputGrid->GetPointData();
  vtkSmartPointer<vtkDoubleArray> scalarsArr = asiVisu_Utils::InitDoubleArray(ARRNAME_VOXEL_N_SCALARS);
  //
  pPointData->SetScalars(scalarsArr);

  // Add cells.
  this->addUniformVoxels(pOutputGrid);

  std::cout << "Min scalar is "     << m_fMinScalar    << std::endl;
  std::cout << "Max scalar is "     << m_fMaxScalar    << std::endl;
  std::cout << "Min voxel size is " << m_fMinVoxelSize << std::endl;

  return 1;
}

//-----------------------------------------------------------------------------

void asiVisu_Grid2dSource::addUniformVoxels(vtkUnstructuredGrid* pData)
{
  if ( m_grid.IsNull() )
  {
    vtkErrorMacro( << "The uniform grid data structure is null." );
    return;
  }

  const double z    = m_grid->ZMin;
  const double step = (double) m_grid->CellSize;

  for ( int i = 0; i <= m_grid->Nx; ++i )
  {
    const double x = m_grid->XMin + step*i;
    //
    for ( int j = 0; j <= m_grid->Ny; ++j )
    {
      const double y = m_grid->YMin + step*j;

      if ( (i < m_grid->Nx) && (j < m_grid->Ny) )
      {
        const double
          sc[4] = { m_grid->pArray[i]    [j]    [0],
                    m_grid->pArray[i + 1][j]    [0],
                    m_grid->pArray[i + 1][j + 1][0],
                    m_grid->pArray[i]    [j + 1][0] };

        const bool isOn  = ::IsZeroCrossing (sc);
        const bool isIn  = ::IsIn           (sc);
        const bool isOut = ::IsOut          (sc);

        if ( ( isOn  && (m_strategy & SS_On)  ) ||
             ( isIn  && (m_strategy & SS_In)  ) ||
             ( isOut && (m_strategy & SS_Out) ) )
        {
          // Cell corners.
          gp_Pnt P0(x,        y,        z);
          gp_Pnt P1(x + step, y,        z);
          gp_Pnt P2(x + step, y + step, z);
          gp_Pnt P3(x,        y + step, z);

          // Extremities over the scalar values.
          m_fMinScalar = Min(m_fMinScalar, sc[0]);
          m_fMinScalar = Min(m_fMinScalar, sc[1]);
          m_fMinScalar = Min(m_fMinScalar, sc[2]);
          m_fMinScalar = Min(m_fMinScalar, sc[3]);
          //
          m_fMaxScalar = Max(m_fMaxScalar, sc[0]);
          m_fMaxScalar = Max(m_fMaxScalar, sc[1]);
          m_fMaxScalar = Max(m_fMaxScalar, sc[2]);
          m_fMaxScalar = Max(m_fMaxScalar, sc[3]);

          // Add voxel to the data set.
          this->registerCell(P0, P1, P2, P3,
                             sc[0], sc[1], sc[2], sc[3],
                             pData);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_Grid2dSource::registerCell(const gp_Pnt&        node0,
                                     const gp_Pnt&        node1,
                                     const gp_Pnt&        node2,
                                     const gp_Pnt&        node3,
                                     const double         sc0,
                                     const double         sc1,
                                     const double         sc2,
                                     const double         sc3,
                                     vtkUnstructuredGrid* pData)
{
  std::vector<vtkIdType> pids =
  {
    this->addPoint(node0, pData),
    this->addPoint(node1, pData),
    this->addPoint(node2, pData),
    this->addPoint(node3, pData)
  };

  // Set scalars.
  vtkDoubleArray*
    pScalarsArr = vtkDoubleArray::SafeDownCast( pData->GetPointData()->GetArray(ARRNAME_VOXEL_N_SCALARS) );
  //
  pScalarsArr->InsertTypedTuple(pids[0], &sc0);
  pScalarsArr->InsertTypedTuple(pids[1], &sc1);
  pScalarsArr->InsertTypedTuple(pids[2], &sc2);
  pScalarsArr->InsertTypedTuple(pids[3], &sc3);

  // Register grid cell.
  vtkIdType cellID = pData->InsertNextCell(VTK_QUAD, 4, &pids[0]);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType asiVisu_Grid2dSource::registerVertex(const gp_Pnt&        point,
                                               vtkUnstructuredGrid* pData)
{
  std::vector<vtkIdType> pids =
  {
    this->addPoint(point, pData)
  };

  // Register vertex cell.
  vtkIdType cellID = pData->InsertNextCell(VTK_VERTEX, 1, &pids[0]);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_Grid2dSource::addPoint(const gp_Pnt&        coords,
                                 vtkUnstructuredGrid* pData)
{
  // Access points array.
  vtkPoints*
    points = pData->GetPoints();

  // Add the point to the VTK data set.
  vtkIdType
    resPid = points->InsertNextPoint( coords.X(), coords.Y(), coords.Z() );

  return resPid;
}
