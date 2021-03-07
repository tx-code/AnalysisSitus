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
#include <gltf_FaceIterator.h>

// OpenCascade includes
#include <BRepAdaptor_Surface.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs.hxx>

//-----------------------------------------------------------------------------

asiAsm::gltf_FaceIterator::gltf_FaceIterator(const TDF_Label&           label,
                                             const TopLoc_Location&     location,
                                             const bool                 toMapColors,
                                             const gltf_XdeVisualStyle& style)
: m_defStyle      (style),
  m_bMapColors    (toMapColors),
  m_SLTool        (1, 1e-12),
  m_pNodes        (nullptr),
  m_pNormals      (nullptr),
  m_pNodeUVs      (nullptr),
  m_bHasNormals   (false),
  m_bMirrored     (false),
  m_bHasFaceColor (false)
{
  TopoDS_Shape shape;

  // If there's no shape in the XDE for the host label, we do nothing.
  if ( !XCAFDoc_ShapeTool::GetShape(label, shape) || shape.IsNull() )
  {
    return;
  }

  shape.Location(location);
  m_faceExp.Init(shape, TopAbs_FACE);

  if ( toMapColors )
  {
    this->readStyles(label, location, style);
  }

  this->Next();
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_FaceIterator::More() const
{
  return !m_polyTriang.IsNull();
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_FaceIterator::Next()
{
  for ( ; m_faceExp.More(); m_faceExp.Next() )
  {
    m_face       = TopoDS::Face( m_faceExp.Current() );
    m_polyTriang = BRep_Tool::Triangulation(m_face, m_faceLocation);
    m_trsf       = m_faceLocation.Transformation();

    if ( m_polyTriang.IsNull() || !m_polyTriang->Triangles().Length() )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Face without triangulation skipped.");

      this->resetFace();
      continue;
    }

    this->initFace();
    m_faceExp.Next();
    return;
  }

  this->resetFace();
}

//-----------------------------------------------------------------------------

const TopoDS_Face& asiAsm::gltf_FaceIterator::Face() const
{
  return m_face;
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_FaceIterator::IsEmptyMesh() const
{
  return m_polyTriang.IsNull()
     || (m_polyTriang->NbNodes() < 1 && m_polyTriang->NbTriangles() < 1);
}

//-----------------------------------------------------------------------------

const asiAsm::gltf_XdeVisualStyle& asiAsm::gltf_FaceIterator::FaceStyle() const
{
  return m_faceStyle;
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_FaceIterator::HasFaceColor() const
{
  return m_bHasFaceColor;
}

//-----------------------------------------------------------------------------

const Quantity_ColorRGBA& asiAsm::gltf_FaceIterator::FaceColor() const
{
  return m_faceColor;
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::NbTriangles() const
{
  return m_polyTriang->NbTriangles();
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::ElemLower() const
{
  return m_polyTriang->Triangles().Lower();
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::ElemUpper() const
{
  return m_polyTriang->Triangles().Upper();
}

//-----------------------------------------------------------------------------

Poly_Triangle asiAsm::gltf_FaceIterator::TriangleOriented(int elemIndex) const
{
  Poly_Triangle tri = triangle(elemIndex);

  if ( (m_face.Orientation() == TopAbs_REVERSED) ^ m_bMirrored )
  {
    return Poly_Triangle( tri.Value(1), tri.Value(3), tri.Value(2) );
  }
  return tri;
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_FaceIterator::HasNormals() const
{
  return m_bHasNormals;
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_FaceIterator::HasTexCoords() const
{
  return m_pNodeUVs != NULL;
}

//-----------------------------------------------------------------------------

gp_Dir asiAsm::gltf_FaceIterator::NormalTransformed(int theNode)
{
  gp_Dir aNorm = normal (theNode);
  if ( m_trsf.Form() != gp_Identity )
  {
    aNorm.Transform (m_trsf);
  }
  if ( m_face.Orientation() == TopAbs_REVERSED )
  {
    aNorm.Reverse();
  }
  return aNorm;
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::NbNodes() const
{
  return !m_polyTriang.IsNull()
        ? m_polyTriang->Nodes().Length()
        : 0;
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::NodeLower() const
{
  return m_polyTriang->Nodes().Lower();
}

//-----------------------------------------------------------------------------

int asiAsm::gltf_FaceIterator::NodeUpper() const
{
  return m_polyTriang->Nodes().Upper();
}

//-----------------------------------------------------------------------------

gp_Pnt asiAsm::gltf_FaceIterator::NodeTransformed(const int N) const
{
  gp_Pnt NP = node(N);
  NP.Transform(m_trsf);
  return NP;
}

//-----------------------------------------------------------------------------

gp_Pnt2d asiAsm::gltf_FaceIterator::NodeTexCoord(const int N) const
{
  return m_pNodeUVs != NULL ? m_pNodeUVs->Value(N) : gp_Pnt2d();
}

//-----------------------------------------------------------------------------

gp_Pnt asiAsm::gltf_FaceIterator::node(const int N) const
{
  return m_polyTriang->Nodes().Value(N);
}

//-----------------------------------------------------------------------------

gp_Dir asiAsm::gltf_FaceIterator::normal(int N)
{
  gp_Dir norm( gp::DZ() );
  if (m_pNormals != NULL)
  {
    const int nodeIdx = N - m_pNodes->Lower();
    const Graphic3d_Vec3 normVec3( m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3),
                                   m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3 + 1),
                                   m_pNormals->Value(m_pNormals->Lower() + nodeIdx * 3 + 2) );
    if ( normVec3.Modulus() != 0.0f )
    {
      norm.SetCoord (normVec3.x(), normVec3.y(), normVec3.z());
    }
  }
  else if ( m_bHasNormals && m_pNodeUVs != NULL )
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

Poly_Triangle asiAsm::gltf_FaceIterator::triangle(int elemIndex) const
{
  return m_polyTriang->Triangles().Value(elemIndex);
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_FaceIterator::readStyles(const TDF_Label&           label,
                                           const TopLoc_Location&     location,
                                           const gltf_XdeVisualStyle& style)
{
  // Get styles out of OCAF.
  TopLoc_Location dummyLoc;
  XCAFPrs_IndexedDataMapOfShapeStyle styles;
  XCAFPrs::CollectStyleSettings(label, dummyLoc, styles);

  int nbTypes[TopAbs_SHAPE] = {};
  for ( int tit = TopAbs_FACE; tit >= TopAbs_COMPOUND; --tit )
  {
    if ( (tit != TopAbs_FACE) && (nbTypes[tit] == 0) )
    {
      continue;
    }

    for ( XCAFPrs_IndexedDataMapOfShapeStyle::Iterator sit(styles); sit.More(); sit.Next())
    {
      const TopoDS_Shape&    keyShape     = sit.Key();
      const TopAbs_ShapeEnum keyShapeType = keyShape.ShapeType();

      if ( tit == TopAbs_FACE )
      {
        ++nbTypes[keyShapeType];
      }
      if ( tit != keyShapeType )
      {
        continue;
      }

      gltf_XdeVisualStyle cafStyle = sit.Value();
      if ( !cafStyle.IsSetColorCurve() && style.IsSetColorCurve() )
      {
        cafStyle.SetColorCurve( style.GetColorCurve() );
      }
      if ( !cafStyle.IsSetColorSurf() && style.IsSetColorSurf() )
      {
        cafStyle.SetColorSurf( style.GetColorSurfRGBA() );
      }
      if ( cafStyle.GetMaterial().IsNull() && !style.GetMaterial().IsNull() )
      {
        cafStyle.SetMaterial( style.GetMaterial() );
      }

      TopoDS_Shape keyShapeLocated = keyShape.Located(location);
      //
      if ( keyShapeType == TopAbs_FACE )
      {
        m_styles.Bind(keyShapeLocated, cafStyle);
      }
      else
      {
        for ( TopExp_Explorer fit(keyShapeLocated, TopAbs_FACE); fit.More(); fit.Next() )
        {
          if ( !m_styles.IsBound( fit.Current() ) )
          {
            m_styles.Bind(fit.Current(), cafStyle);
          }
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_FaceIterator::resetFace()
{
  m_polyTriang.Nullify();
  m_face.Nullify();
  //
  m_pNodes        = nullptr;
  m_pNormals      = nullptr;
  m_pNodeUVs      = nullptr;
  m_bHasNormals   = false;
  m_bHasFaceColor = false;
  m_faceColor     = Quantity_ColorRGBA();
  m_faceStyle     = gltf_XdeVisualStyle();
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_FaceIterator::initFace()
{
  m_bHasNormals   = false;
  m_bHasFaceColor = false;
  m_bMirrored     = m_trsf.VectorialPart().Determinant() < 0.0;
  m_pNormals      = nullptr;
  m_pNodeUVs      = nullptr;

  m_pNodes = &m_polyTriang->Nodes();
  if ( m_polyTriang->HasNormals() )
  {
    m_pNormals    = &m_polyTriang->Normals();
    m_bHasNormals = true;
  }
  if ( m_polyTriang->HasUVNodes() )
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
  if ( !m_bMapColors )
  {
    return;
  }

  if ( !m_styles.Find(m_face, m_faceStyle) )
  {
    m_faceStyle = m_defStyle;
  }

  if ( !m_faceStyle.GetMaterial().IsNull() )
  {
    m_bHasFaceColor = true;
    m_faceColor     = m_faceStyle.GetMaterial()->BaseColor();
  }
  else if ( m_faceStyle.IsSetColorSurf() )
  {
    m_bHasFaceColor = true;
    m_faceColor     = m_faceStyle.GetColorSurfRGBA();
  }
}
