//-----------------------------------------------------------------------------
// Created on: 26 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiVisu_MeshDataProvider.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// asiVisu includes
#include <asiVisu_MeshUtils.h>
#include <asiVisu_Utils.h>

//! Default constructor.
asiVisu_MeshDataProvider::asiVisu_MeshDataProvider() : asiVisu_DataProvider() {}

//! Constructor accepting the set of source data structures.
//! \param theNodeId    [in] ID of the target Data Node.
//! \param theParamList [in] source Parameters: Mesh, Color, EdgesColors.
asiVisu_MeshDataProvider::asiVisu_MeshDataProvider(const ActAPI_DataObjectId&           nodeId,
                                                   const Handle(ActData_MeshParameter)& meshParam,
                                                   const Handle(ActData_IntParameter)&  colorParam,
                                                   const Handle(ActData_IntParameter)&  edgesColorParam)
: asiVisu_DataProvider ( ),
  m_nodeID             ( nodeId ),
  m_meshParam          ( meshParam ),
  m_colorParam         ( colorParam ),
  m_edgeColorParam     ( edgesColorParam )
{}

//! Returns ID of the Data Node represented by VTK actor. This ID is bound to
//! the pipeline's actor in order to have a back-reference from Presentation
//! to Data Object.
//! \return Node ID.
ActAPI_DataObjectId asiVisu_MeshDataProvider::GetNodeID() const
{
  return m_nodeID;
}

//! Returns Mesh Data Structures used as the main source for pipelining.
//! \return tessellation DS.
Handle(ActData_Mesh) asiVisu_MeshDataProvider::GetMeshDS() const
{
  return m_meshParam->GetMesh();
}

//! Returns persistent color.
//! \param[out] r red component.
//! \param[out] g green component.
//! \param[out] b blue component.
void asiVisu_MeshDataProvider::GetColor(double& r, double& g, double& b) const
{
  asiVisu_MeshUtils::DefaultElemColor(r, g, b);

  if ( m_colorParam.IsNull() )
  {
    return;
  }

  const int icolor = m_colorParam->GetValue();

  ActAPI_Color color = asiVisu_Utils::IntToColor(icolor);

  r = color.Red();
  g = color.Green();
  b = color.Blue();
}

//! Returns persistent edges color.
//! \param[out] r red component.
//! \param[out] g green component.
//! \param[out] b blue component.
void asiVisu_MeshDataProvider::GetEdgesColor(double& r, double& g, double& b) const
{
  asiVisu_MeshUtils::DefaultContourColor(r, g, b);

  if ( m_edgeColorParam.IsNull() )
  {
    return;
  }

  const int icolor = m_edgeColorParam->GetValue();

  ActAPI_Color color = asiVisu_Utils::IntToColor(icolor);

  r = color.Red();
  g = color.Green();
  b = color.Blue();
}

//! Accessor for the source Data Parameters.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_MeshDataProvider::SourceParameters() const
{
  return this->translationSources();
}

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_MeshDataProvider::translationSources() const
{
  ActAPI_ParameterStream aResStream;
  if ( !m_meshParam.IsNull() )
    aResStream << m_meshParam;       // Mesh Parameter [entire mesh]
  if ( !m_colorParam.IsNull() )
    aResStream << m_colorParam;      // Color.
  if ( !m_edgeColorParam.IsNull() )
    aResStream << m_edgeColorParam;  // Color of edges.
  return aResStream;
}
