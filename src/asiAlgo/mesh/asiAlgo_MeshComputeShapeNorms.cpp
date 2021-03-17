//-----------------------------------------------------------------------------
// Created on: 17 March 2021
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
#include <asiAlgo_MeshComputeShapeNorms.h>

// OpenCascade includes
#include <BRepBndLib.hxx>
#include <BRepMesh_DiscretFactory.hxx>
#include <BRepMesh_DiscretRoot.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <GeomLib.hxx>
#include <gp_XYZ.hxx>
#include <Poly.hxx>
#include <Poly_Connect.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Prs3d.hxx>
#include <Prs3d_Drawer.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopLoc_Location.hxx>
#include <TShort_HArray1OfShortReal.hxx>
#include <TShort_Array1OfShortReal.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

//=======================================================================
//function : IsTriangulated
//purpose  :
//=======================================================================
Standard_Boolean asiAlgo_MeshComputeShapeNorms::IsTriangulated (const TopoDS_Shape& theShape)
{
  TopLoc_Location aLocDummy;
  for (TopExp_Explorer aFaceIter (theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoDS_Face&                aFace = TopoDS::Face (aFaceIter.Current());
    const Handle(Poly_Triangulation)& aTri  = BRep_Tool::Triangulation (aFace, aLocDummy);
    if (aTri.IsNull())
    {
      return Standard_False;
    }
  }
  return Standard_True;
}

//=======================================================================
//function : IsClosed
//purpose  :
//=======================================================================
Standard_Boolean asiAlgo_MeshComputeShapeNorms::IsClosed (const TopoDS_Shape& theShape)
{
  if (theShape.IsNull())
  {
    return Standard_True;
  }

  switch (theShape.ShapeType())
  {
    case TopAbs_COMPOUND:
    case TopAbs_COMPSOLID:
    default:
    {
      // check that compound consists of closed solids
      for (TopoDS_Iterator anIter (theShape); anIter.More(); anIter.Next())
      {
        const TopoDS_Shape& aShape = anIter.Value();
        if (!IsClosed (aShape))
        {
          return Standard_False;
        }
      }
      return Standard_True;
    }
    case TopAbs_SOLID:
    {
      // Check for non-manifold topology first of all:
      // have to use BRep_Tool::IsClosed() because it checks the face connectivity
      // inside the shape
      if (!BRep_Tool::IsClosed (theShape))
        return Standard_False;

      for (TopoDS_Iterator anIter (theShape); anIter.More(); anIter.Next())
      {
        const TopoDS_Shape& aShape = anIter.Value();
        if (aShape.IsNull())
        {
          continue;
        }

        if (aShape.ShapeType() == TopAbs_FACE)
        {
          // invalid solid
          return Standard_False;
        }
        else if (!IsTriangulated (aShape))
        {
          // mesh contains holes
          return Standard_False;
        }
      }
      return Standard_True;
    }
    case TopAbs_SHELL:
    case TopAbs_FACE:
    {
      // free faces / shell are not allowed
      return Standard_False;
    }
    case TopAbs_WIRE:
    case TopAbs_EDGE:
    case TopAbs_VERTEX:
    {
      // ignore
      return Standard_True;
    }
  }
}

//=======================================================================
//function : ComputeNormals
//purpose  :
//=======================================================================
void asiAlgo_MeshComputeShapeNorms::ComputeNormals (const TopoDS_Face& theFace,
                                                   const Handle(Poly_Triangulation)& theTris,
                                                   Poly_Connect& thePolyConnect)
{
  if (theTris.IsNull()
   || theTris->HasNormals())
  {
    return;
  }

  // take in face the surface location
  const TopoDS_Face    aZeroFace = TopoDS::Face (theFace.Located (TopLoc_Location()));
  Handle(Geom_Surface) aSurf     = BRep_Tool::Surface (aZeroFace);
  const Poly_Array1OfTriangle& aTriangles = theTris->Triangles();
  if (!theTris->HasUVNodes() || aSurf.IsNull())
  {
    // compute normals by averaging triangulation normals sharing the same vertex
    Poly::ComputeNormals (theTris);
    return;
  }

  const Standard_Real aTol = Precision::Confusion();
  Handle(TShort_HArray1OfShortReal) aNormals = new TShort_HArray1OfShortReal (1, theTris->NbNodes() * 3);
  const TColgp_Array1OfPnt2d& aNodesUV = theTris->UVNodes();
  Standard_Integer aTri[3];
  const TColgp_Array1OfPnt& aNodes = theTris->Nodes();
  gp_Dir aNorm;
  for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
  {
    // try to retrieve normal from real surface first, when UV coordinates are available
    if (GeomLib::NormEstim (aSurf, aNodesUV.Value (aNodeIter), aTol, aNorm) > 1)
    {
      if (thePolyConnect.Triangulation() != theTris)
      {
        thePolyConnect.Load (theTris);
      }

      // compute flat normals
      gp_XYZ eqPlan (0.0, 0.0, 0.0);
      for (thePolyConnect.Initialize (aNodeIter); thePolyConnect.More(); thePolyConnect.Next())
      {
        aTriangles (thePolyConnect.Value()).Get (aTri[0], aTri[1], aTri[2]);
        const gp_XYZ v1 (aNodes (aTri[1]).Coord() - aNodes (aTri[0]).Coord());
        const gp_XYZ v2 (aNodes (aTri[2]).Coord() - aNodes (aTri[1]).Coord());
        const gp_XYZ vv = v1 ^ v2;
        const Standard_Real aMod = vv.Modulus();
        if (aMod >= aTol)
        {
          eqPlan += vv / aMod;
        }
      }
      const Standard_Real aModMax = eqPlan.Modulus();
      aNorm = (aModMax > aTol) ? gp_Dir (eqPlan) : gp::DZ();
    }

    const Standard_Integer anId = (aNodeIter - aNodes.Lower()) * 3;
    aNormals->SetValue (anId + 1, (Standard_ShortReal )aNorm.X());
    aNormals->SetValue (anId + 2, (Standard_ShortReal )aNorm.Y());
    aNormals->SetValue (anId + 3, (Standard_ShortReal )aNorm.Z());
  }
  theTris->SetNormals (aNormals);
}

//=======================================================================
//function : Normal
//purpose  :
//=======================================================================
void asiAlgo_MeshComputeShapeNorms::Normal (const TopoDS_Face&  theFace,
                                           Poly_Connect&       thePolyConnect,
                                           TColgp_Array1OfDir& theNormals)
{
  const Handle(Poly_Triangulation)& aPolyTri = thePolyConnect.Triangulation();
  if (!aPolyTri->HasNormals())
  {
    ComputeNormals (theFace, aPolyTri, thePolyConnect);
  }

  const TColgp_Array1OfPnt&       aNodes   = aPolyTri->Nodes();
  const TShort_Array1OfShortReal& aNormals = aPolyTri->Normals();
  const Standard_ShortReal*       aNormArr = &aNormals.First();
  for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
  {
    const Standard_Integer anId = 3 * (aNodeIter - aNodes.Lower());
    const gp_Dir aNorm (aNormArr[anId + 0],
                        aNormArr[anId + 1],
                        aNormArr[anId + 2]);
    theNormals (aNodeIter) = aNorm;
  }

  if (theFace.Orientation() == TopAbs_REVERSED)
  {
    for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
    {
      theNormals.ChangeValue (aNodeIter).Reverse();
    }
  }
}
