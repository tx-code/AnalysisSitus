//-----------------------------------------------------------------------------
// Created on: March 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_GraphToDot.h>

// ACT Data includes
#include <ActData_BaseModel.h>
#include <ActData_DependencyGraph.h>
#include <ActData_RealVarNode.h>

//! Converts Execution Graph to DOT format.
//! \param M [in] Data Model to convert Execution Graph for.
//! \return string representation of the Execution Graph.
TCollection_AsciiString ActData_GraphToDot::Convert(const Handle(ActAPI_IModel)& M)
{
  TCollection_AsciiString result = TCollection_AsciiString("digraph G {") + ActAux_Macro_NL;

  /* ================================================
   *  Populate graph data from Model Execution Graph
   * ================================================ */

  Handle(ActData_BaseModel)       MBase = Handle(ActData_BaseModel)::DownCast(M);
  Handle(ActData_DependencyGraph) G     = new ActData_DependencyGraph(MBase);

  // Get graph data
  const ActData_DependencyGraph::VertexDataMap& Vertices = G->Vertices();
  const ActData_DependencyGraph::EdgeMap&       Links    = G->Edges();

  // Populate graph data with specification of links
  for ( ActData_DependencyGraph::EdgeMap::Iterator eIt(Links); eIt.More(); eIt.Next() )
  {
    const ActData_DependencyGraph::OriEdge& E = eIt.Value();
    Handle(ActAPI_INode) V1_Node = Vertices.Find(E.V1).Parameter->GetNode();
    Handle(ActAPI_INode) V2_Node = Vertices.Find(E.V2).Parameter->GetNode();

    // Name of the first vertex
    TCollection_AsciiString V1_name;
    if ( V1_Node->IsKind( STANDARD_TYPE(ActData_BaseVarNode) ) )
      V1_name = Handle(ActData_BaseVarNode)::DownCast(V1_Node)->GetVariableName();
    else
      V1_name = Vertices.Find(E.V1).Parameter->GetNode()->GetId();

    // Name of the last vertex
    TCollection_AsciiString V2_name;
    if ( V2_Node->IsKind( STANDARD_TYPE(ActData_BaseVarNode) ) )
      V2_name = Handle(ActData_BaseVarNode)::DownCast(V2_Node)->GetVariableName();
    else
      V2_name = Vertices.Find(E.V2).Parameter->GetNode()->GetId();

    // Prepare string descriptor for the link
    result  = result + "\"" + Vertices.Find(E.V1).TreeFunction->GetName() + " (" + V1_name + ")\"";
    result += " -> ";
    result  = result + "\"" + Vertices.Find(E.V2).TreeFunction->GetName() + " (" + V2_name + ")\"";
    result += ";";
    result += ActAux_Macro_NL;
  }

  /* ==========
   *  Finalize
   * ========== */

  result = result + "}" + ActAux_Macro_NL;

  return result;
}
