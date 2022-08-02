/***************************************************************************
 *   Copyright (c) OPEN CASCADE SAS                                        *
 *                                                                         *
 *   This file is part of Open CASCADE Technology software library.        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 ***************************************************************************/

// Own include
#include <asiAsm_GLTFFacePropertyExtractor.h>

// OpenCascade includes
#include <BRepAdaptor_Surface.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

glTFFacePropertyExtractor::glTFFacePropertyExtractor(const TopoDS_Face& face)
//
: m_face        (face),
  m_SLTool      (1, 1e-12),
  m_pNodes      (nullptr),
  m_pNormals    (nullptr),
  m_pNodeUVs    (nullptr),
  m_bHasNormals (false),
  m_bMirrored   (false)
{
  m_polyTriang = BRep_Tool::Triangulation(m_face, m_faceLocation);
  m_trsf = m_faceLocation.Transformation();

  this->initFace();
}

//-----------------------------------------------------------------------------

const TopoDS_Face& glTFFacePropertyExtractor::Face() const
{
  return m_face;
}

//-----------------------------------------------------------------------------

bool glTFFacePropertyExtractor::IsEmptyMesh() const
{
  return m_polyTriang.IsNull()
     || (m_polyTriang->NbNodes() < 1 && m_polyTriang->NbTriangles() < 1);
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::NbTriangles() const
{
  return m_polyTriang->NbTriangles();
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::ElemLower() const
{
  return m_polyTriang->Triangles().Lower();
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::ElemUpper() const
{
  return m_polyTriang->Triangles().Upper();
}

//-----------------------------------------------------------------------------

Poly_Triangle glTFFacePropertyExtractor::TriangleOriented(const int elemIndex) const
{
  Poly_Triangle tri = triangle(elemIndex);

  if ( (m_face.Orientation() == TopAbs_REVERSED) ^ m_bMirrored )
  {
    return Poly_Triangle( tri.Value(1), tri.Value(3), tri.Value(2) );
  }
  return tri;
}

//-----------------------------------------------------------------------------

bool glTFFacePropertyExtractor::HasNormals() const
{
  return m_bHasNormals;
}

//-----------------------------------------------------------------------------

bool glTFFacePropertyExtractor::HasTexCoords() const
{
  return m_pNodeUVs != nullptr;
}

//-----------------------------------------------------------------------------

gp_Dir glTFFacePropertyExtractor::NormalTransformed(const int N)
{
  gp_Dir norm = normal(N);

  if ( m_trsf.Form() != gp_Identity )
  {
    norm.Transform(m_trsf);
  }
  if ( m_face.Orientation() == TopAbs_REVERSED )
  {
    norm.Reverse();
  }
  return norm;
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::NbNodes() const
{
  return !m_polyTriang.IsNull()
        ? m_polyTriang->Nodes().Length()
        : 0;
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::NodeLower() const
{
  return m_polyTriang->Nodes().Lower();
}

//-----------------------------------------------------------------------------

int glTFFacePropertyExtractor::NodeUpper() const
{
  return m_polyTriang->Nodes().Upper();
}

//-----------------------------------------------------------------------------

gp_Pnt glTFFacePropertyExtractor::NodeTransformed(const int N) const
{
  gp_Pnt NP = node(N);
  NP.Transform(m_trsf);
  return NP;
}

//-----------------------------------------------------------------------------

gp_Pnt2d glTFFacePropertyExtractor::NodeTexCoord(const int N) const
{
  return m_pNodeUVs != nullptr ? m_pNodeUVs->Value(N) : gp_Pnt2d();
}

//-----------------------------------------------------------------------------

gp_Pnt glTFFacePropertyExtractor::node(const int N) const
{
  return m_polyTriang->Nodes().Value(N);
}

//-----------------------------------------------------------------------------

gp_Dir glTFFacePropertyExtractor::normal(const int N)
{
  gp_Dir norm( gp::DZ() );

  if ( m_pNormals != nullptr )
  {
    const int nodeIdx = N - m_pNodes->Lower();
    const Graphic3d_Vec3 normVec3( m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3),
                                   m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3 + 1),
                                   m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3 + 2) );
    if ( normVec3.Modulus() != 0.0f )
    {
      norm.SetCoord( normVec3.x(), normVec3.y(), normVec3.z() );
    }
  }
  else if ( m_bHasNormals && m_pNodeUVs != nullptr )
  {
    const gp_XY& anUV = m_pNodeUVs->Value(N).XY();
    m_SLTool.SetParameters( anUV.X(), anUV.Y() );
    //
    if ( m_SLTool.IsNormalDefined() )
    {
      norm = m_SLTool.Normal();
    }
  }
  return norm;
}

//-----------------------------------------------------------------------------

const Poly_Triangle&
  glTFFacePropertyExtractor::triangle(const int elemIndex) const
{
  return m_polyTriang->Triangles().Value(elemIndex);
}

//-----------------------------------------------------------------------------

void glTFFacePropertyExtractor::initFace()
{
  m_bHasNormals = false;
  m_bMirrored   = m_trsf.VectorialPart().Determinant() < 0.0;
  m_pNormals    = nullptr;
  m_pNodeUVs    = nullptr;

  m_pNodes = &m_polyTriang->Nodes();
  //
  if ( !m_polyTriang.IsNull() && m_polyTriang->HasNormals() )
  {
    m_pNormals    = &m_polyTriang->Normals();
    m_bHasNormals = true;
  }
  if ( !m_polyTriang.IsNull() && m_polyTriang->HasUVNodes() )
  {
    m_pNodeUVs = &m_polyTriang->UVNodes();
    if ( !m_bHasNormals )
    {
      TopoDS_Face faceFwd = TopoDS::Face( m_face.Oriented(TopAbs_FORWARD) );
      faceFwd.Location( TopLoc_Location() );
      TopLoc_Location aLoc;
      if ( !BRep_Tool::Surface(faceFwd, aLoc).IsNull() )
      {
        m_faceAdaptor.Initialize(faceFwd, false);
        m_SLTool.SetSurface(m_faceAdaptor);
        m_bHasNormals = true;
      }
    }
  }
}
