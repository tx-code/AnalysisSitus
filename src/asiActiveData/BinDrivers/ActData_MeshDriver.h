//-----------------------------------------------------------------------------
// Created on: May 2012
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

#ifndef ActData_MeshDriver_HeaderFile
#define ActData_MeshDriver_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_MeshAttr.h>

// Mesh includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

// OCCT includes
#include <BinMDF_ADriver.hxx>
#include <Message_Messenger.hxx>

DEFINE_STANDARD_HANDLE(ActData_MeshDriver, BinMDF_ADriver)

//! \ingroup AD_DF
//!
//! Storage/Retrieval Driver for Mesh Attribute.
class ActData_MeshDriver : public BinMDF_ADriver
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MeshDriver, BinMDF_ADriver)

public:

  //---------------------------------------------------------------------------

  template <typename TStream>
  static bool
    Write(const Handle(ActData_MeshAttr)& meshAttr,
          TStream&                        out)
  {
    /* ==================================
     *  Push mesh nodes to binary buffer
     * ================================== */

    const Handle(ActData_Mesh)& aMeshDS = meshAttr->GetMesh();
    if ( aMeshDS.IsNull() )
    {
      return Standard_False;
    }

    // Access data
    Standard_Integer aNbNodes = aMeshDS->NbNodes();
    Standard_Integer aNbFaces = aMeshDS->NbFaces();

    // Push data
    out << aNbNodes << aNbFaces;

    ActData_Mesh_ElementsIterator aMeshNodesIt(aMeshDS, ActData_Mesh_ET_Node);
    for ( ; aMeshNodesIt.More(); aMeshNodesIt.Next() )
    {
      // Access next node
      Handle(ActData_Mesh_Node)
        aNode = Handle(ActData_Mesh_Node)::DownCast( aMeshNodesIt.GetValue() );

      // Push data
      out << aNode->GetID();
      out << aNode->Pnt().X();
      out << aNode->Pnt().Y();
      out << aNode->Pnt().Z();
    }

    /* =====================================
     *  Push mesh elements to binary buffer
     * ===================================== */

    ActData_Mesh_ElementsIterator aMeshElemsIt(aMeshDS, ActData_Mesh_ET_Face);
    for ( ; aMeshElemsIt.More(); aMeshElemsIt.Next() )
    {
      const Handle(ActData_Mesh_Element)& anElem = aMeshElemsIt.GetValue();

      // Proceed with TRIANGLE elements
      if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
      {
        // Access element data
        Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(anElem);
        Standard_Integer aTriNodeIds[3];
        Standard_Integer aNbFaceNodes;
        aTriElem->GetFaceDefinedByNodes(3, aTriNodeIds, aNbFaceNodes);

        // Push data
        out << aTriElem->GetID();
        out << aNbFaceNodes;
        for ( Standard_Integer i = 0; i < aNbFaceNodes; i++ )
          out << aTriNodeIds[i];
      }
      // Proceed with QUADRANGLE elements
      else if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
      {
        // Access element data
        Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(anElem);
        Standard_Integer aQuadNodeIds[4];
        Standard_Integer aNbFaceNodes;
        aQuadElem->GetFaceDefinedByNodes(4, aQuadNodeIds, aNbFaceNodes);

        // Push data
        out << aQuadElem->GetID();
        out << aNbFaceNodes;
        for ( Standard_Integer i = 0; i < aNbFaceNodes; i++ )
          out << aQuadNodeIds[i];
      }
    }
    return Standard_True;
  }

  //---------------------------------------------------------------------------

  template <typename TStream>
  static Standard_Boolean
    Read(const TStream&            in,
         Handle(ActData_MeshAttr)& meshAttr)
  {
    /* ==========================================================
     *  Read number of nodes and elements from the binary buffer
     * ========================================================== */

    // Read data
    Standard_Integer aNbNodes, aNbFaces;
    in >> aNbNodes >> aNbFaces;

    /* ====================
     *  Restore mesh nodes
     * ==================== */

    for ( Standard_Integer i = 1; i <= aNbNodes; i++ )
    {
      Standard_Integer aNodeID;
      Standard_Real aCoordX, aCoordY, aCoordZ;

      // Read data
      in >> aNodeID;
      in >> aCoordX;
      in >> aCoordY;
      in >> aCoordZ;

      // Create a transient node
      meshAttr->GetMesh()->AddNodeWithID(aCoordX, aCoordY, aCoordZ, aNodeID);
    }

    /* =======================
     *  Restore mesh elements
     * ======================= */

    for ( Standard_Integer i = 1; i <= aNbFaces; i++ )
    {
      Standard_Integer aFaceID, aNbFaceNodes;
      in >> aFaceID >> aNbFaceNodes;

      if ( aNbFaceNodes == 3 )
      {
        Standard_Integer aTriNodeIDs[3];
        in >> aTriNodeIDs[0] >> aTriNodeIDs[1] >> aTriNodeIDs[2];

        // Create a transient element
        meshAttr->GetMesh()->AddFaceWithID(aTriNodeIDs, aNbFaceNodes, aFaceID);
      }
      else if ( aNbFaceNodes == 4 )
      {
        Standard_Integer aQuadNodeIDs[4];
        in >> aQuadNodeIDs[0] >> aQuadNodeIDs[1]
           >> aQuadNodeIDs[2] >> aQuadNodeIDs[3];

        // Create a transient element
        meshAttr->GetMesh()->AddFaceWithID(aQuadNodeIDs, aNbFaceNodes, aFaceID);
      }
      else
      {
        return Standard_False;
      }
    }
    return Standard_True;
  }

  //---------------------------------------------------------------------------

// Construction:
public:

  ActData_EXPORT
    ActData_MeshDriver(const Handle(Message_Messenger)& theMsgDriver);

// Kernel:
public:

  ActData_EXPORT virtual Handle(TDF_Attribute)
    NewEmpty() const;

  ActData_EXPORT virtual Standard_Boolean
    Paste(const BinObjMgt_Persistent&  FromPersistent,
          const Handle(TDF_Attribute)& ToTransient,
          BinObjMgt_RRelocationTable&  RelocTable) const;
  
  ActData_EXPORT virtual void
    Paste(const Handle(TDF_Attribute)& FromTransient,
          BinObjMgt_Persistent&        ToPersistent,
          BinObjMgt_SRelocationTable&  RelocTable) const;

};

#endif
