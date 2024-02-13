//-----------------------------------------------------------------------------
// Created on: 17 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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

#ifndef asiAlgo_DiscrBuilder_HeaderFile
#define asiAlgo_DiscrBuilder_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrEdge.h>
#include <asiAlgo_DiscrFace.h>
#include <asiAlgo_DiscrModel.h>
#include <asiAlgo_DiscrWire.h>

// OpenCascade includes
#include <NCollection_Map.hxx>
#include <Standard_Mutex.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>
#include <TColStd_DataMapOfIntegerReal.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

//! Constructs discrete primitives.
class Builder
{
public:

  //! Computation status.
  enum CompStatus
  {
    FailureNullShape          = 1<<0,  //!< Initial shape is null
    FailureShapeWithoutFaces  = 1<<1,  //!< Initial shape does not contain faces
    FailureShapeWithoutEdges  = 1<<2,  //!< Initial shape does not contain edges
    FailureDiscretizeCurve    = 1<<3,  //!< Failure discretisation of 3D curve
    FailureNonMonifoldWires   = 1<<4,  //!< Non-manifold wires in face (skip)
    FailureFaceWithoutWires   = 1<<5,  //!< Face does not contain wires
    FailureDiscretizeFace     = 1<<6,  //!< Failure triangulating face
    FailureOrderOfEdges       = 1<<7,  //!< Impossible to detect order of edges in wire
    FailureDiscretizeWire     = 1<<8,  //!< Failure discretization of wire
    FailureDiscretizePCurve   = 1<<9,  //!< Failure discretisation of pcurve
    FailureDegeneratedEdge    = 1<<10, //!< Degenerated edge
    FailurePCurveOnSurface    = 1<<11, //!< Edge has no pcurve on the face
    FailureEdgeWithout3DCurve = 1<<12, //!< A non-degenerated edge has no 3D curve
    FailurePCurveRedundancy   = 1<<13, //!< Edge has more than 2 pcurves on the same face
    FailureProcessEdge        = 1<<14, //!< Cannot process non-SameParameter edge
    FailureInvalidEdge        = 1<<15, //!< Invalid edge
    FailureInvalidWire        = 1<<16, //!< Invalid wire
    FailureNullTargetMesh     = 1<<17, //!< Null output mesh for non-triangle meshing
    FailureUserBreak          = 1<<18, //!< User break

    // Warnings
    WarningLargeTolerance     = 1<<19, //!< Tolerance of some edges is more then mesh parameters
    WarningNeedToReorderEdges = 1<<20, //!< Some edges in wire need to be reversed
    WarningInternalEdges      = 1<<21, //!< mixture of oriented and not oriented edges in a wire
    WarningWireWithoutEdges   = 1<<22, //!< Wire does not contain edges
    WarningHoles              = 1<<23  //!< Some areas left not meshed
  };

public:

  Builder(const TopoDS_Shape& shape)
  //
  : m_shape       (shape),
    m_iCompStatus (0),
    m_bDone       (false),
    myIsAutoSize  (true),
    myAutoSize    (0.2)
  {}

public:

  asiAlgo_EXPORT void
    Tessellate();

public:

  //! Adds computation status.
  void AddCompStatus(const int status)
  {
    m_iCompStatus |= status;
  }

  void SetParams(const Params& params)
  {
    m_meshParams = params;
  }

  const Handle(Model)& GetModel() const
  {
    return m_model;
  }

protected:

  bool CheckNullLength2d(const TopoDS_Edge& theEdge);
  double GetEdgeLength(const TopoDS_Edge& aEdge);

  //! Analises theWire s consistency, reorders edges in case of need
  //! and discretises them.
  bool DiscretiseWire
    (const TopoDS_Wire& theWire, Wire&       theDWire,
     Face&       theDFace, const TopoDS_Face& theFace=TopoDS_Face());

  bool DiscretiseEdge(const TopoDS_Edge& aEdge, Edge& aDEdge);

  bool DiscretiseEdge(const TopoDS_Edge& aEdge, const TopoDS_Face& aFace,
                      Edge& aDEdge, const Face& aDFace,
                      const bool isEForward);

  bool IsSameParameterEdge(const TopoDS_Edge& theEdge,
                           const TColStd_SequenceOfReal& theParams,
                           double& theTolerance,
                           bool& isTolUpdated) const;

  bool IsSignificant(const TopoDS_Edge&   theEdge,
                     const Edge&          theDEdge,
                     const TopoDS_Vertex& theVertex);

  TopoDS_Face FixFaceWires(const TopoDS_Face& theFace);

  double GetMinSizeForFace(const TopoDS_Face& aFace) const;

protected:

  int                                                m_iCompStatus;
  bool                                               m_bDone;
  TopoDS_Shape                                       m_shape;
  Handle(Model)                                      m_model;
  Params                                             m_meshParams;
  TopTools_IndexedMapOfShape                         m_mapF;
  TopTools_IndexedDataMapOfShapeListOfShape          m_mapEF;
  TopTools_IndexedMapOfShape                         m_mapV;
  TopTools_IndexedMapOfShape                         m_badShapes;
  NCollection_DataMap<Standard_Address, TopoDS_Edge> m_mapDEdges; //!< map of discrete edge address to TopoDS edge
  NCollection_DataMap<Standard_Address, int>         m_mapDFacesId; //!< map of discrete face address to id of TopoDS_Face

  bool                                   myIsAutoSize;
  double                                 myAutoSize;
  TColStd_DataMapOfIntegerReal           m_mapELen;
  TColStd_DataMapOfIntegerReal           m_mapEDefl;
  mutable TColStd_DataMapOfIntegerReal   m_mapFMinSize;
  TColStd_DataMapOfIntegerInteger        m_mapESameParam;
  NCollection_Map<int>                   m_mapEIgnored;
  mutable Standard_Mutex                 m_mutex;

};

}
}

#endif
