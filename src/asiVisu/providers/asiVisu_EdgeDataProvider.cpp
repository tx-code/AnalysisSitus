//-----------------------------------------------------------------------------
// Created on: 18 August 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_EdgeDataProvider.h>

// A-Situs (geometry) includes
#include <asiData_PartNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>

//-----------------------------------------------------------------------------

//! Constructor.
//! \param edge_n [in] source Data Node.
asiVisu_EdgeDataProvider::asiVisu_EdgeDataProvider(const Handle(asiData_EdgeNode)& edge_n)
: asiVisu_CurveDataProvider()
{
  this->init(edge_n);
}

//-----------------------------------------------------------------------------

//! Constructor.
//! \param curve_n [in] source Data Node.
asiVisu_EdgeDataProvider::asiVisu_EdgeDataProvider(const Handle(asiData_CurveNode)& curve_n)
: asiVisu_CurveDataProvider()
{
  this->init(curve_n);
}

//-----------------------------------------------------------------------------

//! Returns ID of the Data Node which is being sourced by the visualization
//! pipeline. This ID is bound to the pipeline's actor in order to have a
//! back-reference from Presentation to Data Object.
//! \return Node ID.
ActAPI_DataObjectId asiVisu_EdgeDataProvider::GetNodeID() const
{
  return m_node->GetId();
}

//-----------------------------------------------------------------------------

//! \return global index of the OCCT edge to be visualized.
int asiVisu_EdgeDataProvider::GetEdgeIndexAmongSubshapes() const
{
  int globalId = 0;

  if ( m_node->IsInstance( STANDARD_TYPE(asiData_EdgeNode) ) )
    globalId = Handle(asiData_EdgeNode)::DownCast(m_node)->GetSelectedEdge();
  else if ( m_node->IsInstance( STANDARD_TYPE(asiData_CurveNode) ) )
    globalId = Handle(asiData_CurveNode)::DownCast(m_node)->GetSelectedEdge();

  return globalId;
}

//-----------------------------------------------------------------------------

//! \return local index of the OCCT edges to be visualized.
int asiVisu_EdgeDataProvider::GetEdgeIndexAmongEdges() const
{
  const int globalId = this->GetEdgeIndexAmongSubshapes();

  if ( globalId )
    return m_edges.FindIndex( m_subShapes.FindKey(globalId) );

  return 0;
}

//-----------------------------------------------------------------------------

//! \return topological edge extracted from the part by its stored ID.
TopoDS_Edge asiVisu_EdgeDataProvider::ExtractEdge() const
{
  const int eIdx = this->GetEdgeIndexAmongSubshapes();
  if ( !eIdx )
    return TopoDS_Edge();

  const TopoDS_Shape& shape = m_subShapes.FindKey(eIdx);
  if ( shape.ShapeType() != TopAbs_EDGE )
    return TopoDS_Edge();

  // Access edge by the stored index
  const TopoDS_Edge& E = TopoDS::Edge(shape);
  return E;
}

//-----------------------------------------------------------------------------

//! \return type of the host curve.
Handle(Standard_Type) asiVisu_EdgeDataProvider::GetCurveType() const
{
  double f, l;
  Handle(Geom_Curve) c3d = this->GetCurve(f, l);
  //
  if ( c3d.IsNull() )
    return NULL;

  return c3d->DynamicType();
}

//-----------------------------------------------------------------------------

//! Not used.
Handle(Geom2d_Curve) asiVisu_EdgeDataProvider::GetCurve2d(double&, double&) const
{
  return NULL;
}

//-----------------------------------------------------------------------------

//! Returns host curve.
//! \param[out] f first parameter on the curve.
//! \param[out] l last parameter on the curve.
//! \return host curve.
Handle(Geom_Curve) asiVisu_EdgeDataProvider::GetCurve(double& f, double& l) const
{
  TopoDS_Edge E = this->ExtractEdge();
  //
  if ( E.IsNull() )
    return NULL;

  return BRep_Tool::Curve(E, f, l);
}

//-----------------------------------------------------------------------------

//! Creates a copy of the Data Provider.
//! \return copy.
Handle(asiVisu_EdgeDataProvider) asiVisu_EdgeDataProvider::Clone() const
{
  if ( m_node->IsInstance( STANDARD_TYPE(asiData_EdgeNode) ) )
  {
    Handle(asiData_EdgeNode)
      edgeNode = Handle(asiData_EdgeNode)::DownCast(m_node);
    //
    return new asiVisu_EdgeDataProvider(edgeNode);
  }

  if ( m_node->IsInstance( STANDARD_TYPE(asiData_CurveNode) ) )
  {
    Handle(asiData_CurveNode)
      curveNode = Handle(asiData_CurveNode)::DownCast(m_node);
    //
    return new asiVisu_EdgeDataProvider(curveNode);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Initializes data provider with the source data.
//! \param subNode [in] Data Node representing the edge.
void asiVisu_EdgeDataProvider::init(const Handle(ActAPI_INode)& subNode)
{
  m_node = subNode;

  // Access owning geometry
  Handle(asiData_PartNode)
    geom_n = Handle(asiData_PartNode)::DownCast( subNode->GetParentNode() );

  // Build maps
  if ( !geom_n->GetShape().IsNull() )
  {
    TopExp::MapShapes(geom_n->GetShape(), m_subShapes);
    TopExp::MapShapes(geom_n->GetShape(), TopAbs_EDGE, m_edges);
  }
}

//-----------------------------------------------------------------------------

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_EdgeDataProvider::translationSources() const
{
  int PID = -1;
  if ( m_node->IsInstance( STANDARD_TYPE(asiData_EdgeNode) ) )
    PID = asiData_EdgeNode::PID_SelectedEdge;
  else if ( m_node->IsInstance( STANDARD_TYPE(asiData_CurveNode) ) )
    PID = asiData_CurveNode::PID_SelectedEdge;

  if ( PID == -1 )
    return NULL;

  return ActParamStream() << m_node->Parameter(PID); // Parameter for edge index
}
