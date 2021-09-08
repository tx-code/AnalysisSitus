//-----------------------------------------------------------------------------
// Created on: April 2012
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
#include <ActData_MeshAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>
#include <Standard_ImmutableObject.hxx>
#include <Standard_ProgramError.hxx>
#include <TDF_Data.hxx>

// Mesh includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

#undef COUT_DEBUG

//-----------------------------------------------------------------------------
// Macro for managing Modification Deltas
//-----------------------------------------------------------------------------

// ------------------------------------------------------------------------- //
#define BACKUP this->Backup()
// ------------------------------------------------------------------------- //
#define MDELTA m_delta
// ------------------------------------------------------------------------- //
#define MDELTA_ACCESS \
  if ( m_delta.IsNull() ) \
    m_delta = new ActData_MeshMDelta(this);
// ------------------------------------------------------------------------- //
#define MDELTA_REPLACED_MESH(OldM, NewM) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->ReplacedMesh(OldM, NewM); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_ADDED_NODE(ID, X, Y, Z) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->AddedNode(ID, X, Y, Z); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_REMOVED_NODE(ID) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->RemovedNode(ID); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_ADDED_TRI(ID, NODES) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->AddedTriangle(ID, NODES); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_REMOVED_TRI(ID) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->RemovedTriangle(ID); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_ADDED_QUAD(ID, NODES) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->AddedQuadrangle(ID, NODES); \
  }
// ------------------------------------------------------------------------- //
#define MDELTA_REMOVED_QUAD(ID) \
  if ( m_bDeltaEnabled ) \
  { \
    MDELTA_ACCESS \
    m_delta->RemovedQuadrangle(ID); \
  }
// ------------------------------------------------------------------------- //

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_MeshAttr::ActData_MeshAttr() : TDF_Attribute()
{
  MDELTA = new ActData_MeshMDelta(this);
  m_bDeltaEnabled = Standard_True;
}

//! Settles down new Mesh Attribute to the given CAF Label.
//! \param Label [in] TDF Label to settle down the new Mesh Attribute onto.
//! \return newly created Mesh Attribute settled down onto the target Label.
Handle(ActData_MeshAttr) ActData_MeshAttr::Set(const TDF_Label& Label)
{
  Handle(ActData_MeshAttr) aMeshAttr;
  if ( !Label.FindAttribute(GUID(), aMeshAttr) )
  {
    aMeshAttr = new ActData_MeshAttr();
    Label.AddAttribute(aMeshAttr);

    aMeshAttr->NewEmptyMesh(); // Initialize with empty Mesh DS
  }
  return aMeshAttr;
}

//-----------------------------------------------------------------------------
// Accessors for Attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for Mesh Attribute.
//! \return statically defined GUID.
const Standard_GUID& ActData_MeshAttr::GUID()
{
  static Standard_GUID AttrGUID("6554F48B-EA68-4DBC-A8EC-D379D2BD7A54");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of CAF Attribute.
//! \return GUID of the CAF Attribute.
const Standard_GUID& ActData_MeshAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of Mesh Attribute which is not initially populated
//! with any mesh structures. This method is mainly used by OCAF Undo/Redo
//! kernel as a part of Backup functionality. The preparation of Backup copy
//! of any CAF Attribute starts with NewEmpty invocation.
//! \return new instance of Mesh Attribute.
Handle(TDF_Attribute) ActData_MeshAttr::NewEmpty() const
{
  return new ActData_MeshAttr();
}

//! Performs data transferring from the given CAF Attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! Backup functionality. For our Mesh Attribute we keep the implementation
//! of this method empty as we are not going to compare our Main Attribute
//! with its Backup copy somehow. Such a comparing is actually the most
//! obvious and common way to retrieve Modification Delta for any Attribute.
//! However, this is not really acceptable for meshes due to optimality
//! reasons. Instead of straightforward comparing we cumulate a Modification
//! Delta in the Main Attribute and push it to the "external world" in
//! DeltaOnModification method.
//! \param MainAttr [in] CAF Attribute to copy data from.
void ActData_MeshAttr::Restore(const Handle(TDF_Attribute)& ActData_NotUsed(MainAttr))
{
  // Nothing is here
}

//! Passing the Modification Delta from the top of the bi-directional stack
//! (either from its positive or negative tails), this method inverts that
//! Delta (as required for Undo/Redo operations) and pushes it to the Main
//! Attribute in order to make the Modification Delta accessible from
//! DeltaOnModification method.
//! \param Delta [in] Modification Delta coming from OCAF kernel stack.
//! \param doForce [in] not used (see OCAF reference for details).
//! \return true always. False value is not used (see OCAF reference for details).
Standard_Boolean ActData_MeshAttr::BeforeUndo(const Handle(TDF_AttributeDelta)& Delta,
                                              const Standard_Boolean            ActData_NotUsed(doForce))
{
  // Prepare Modification Delta for UNDO
  Handle(ActData_MeshMDelta) aMeshDelta = Handle(ActData_MeshMDelta)::DownCast(Delta);
  if ( !aMeshDelta.IsNull() ) // NULL if nothing exists in the stack
    aMeshDelta->Invert();

#if defined ACT_DEBUG && defined COUT_DEBUG
  std::cout << "\nBeforeUndo:" << std::endl;
  if ( !aMeshDelta.IsNull() )
    aMeshDelta->Dump(cout);
#endif

  // Set shallow copy of Modification Delta to the Main Attribute. This
  // trick allows us to have Modification Delta already "charged" with
  // those Modification Queue coming from the Undo/Redo stack
  MDELTA = aMeshDelta;

  return Standard_True;
}

//! Not currently used.
//! \param Delta [in] not used (see OCAF reference for details).
//! \param doForce [in] not used (see OCAF reference for details).
//! \return true always. False value is not used (see OCAF reference for details).
Standard_Boolean ActData_MeshAttr::AfterUndo(const Handle(TDF_AttributeDelta)& ActData_NotUsed(Delta),
                                             const Standard_Boolean            ActData_NotUsed(doForce))
{
  return Standard_True;
}

//! Performs Backup of the Main Attribute as required by OCAF mechanism.
//! Actually, here, in Mesh Attribute, we need only an empty Backup copy
//! just to charge the internal OCAF Undo/Redo workflows. The created
//! Backup instance will not be used for comparison with the Main Attribute
//! as we have Modification Delta for playing that role.
void ActData_MeshAttr::BeforeCommitTransaction()
{
  BACKUP;
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying mesh.
void ActData_MeshAttr::Paste(const Handle(TDF_Attribute)& Into,
                             const Handle(TDF_RelocationTable)&) const
{
  Handle(ActData_MeshAttr) IntoMesh = Handle(ActData_MeshAttr)::DownCast(Into);
  Handle(ActData_Mesh) IntoMeshDS = new ActData_Mesh();

  /* ==================
   *  Paste mesh nodes
   * ================== */

  ActData_Mesh_ElementsIterator aMeshNodesIt(m_mesh, ActData_Mesh_ET_Node);
  for ( ; aMeshNodesIt.More(); aMeshNodesIt.Next() )
  {
    // Access next node
    Handle(ActData_Mesh_Node)
      aNode = Handle(ActData_Mesh_Node)::DownCast( aMeshNodesIt.GetValue() );
    
    // Paste node
    IntoMeshDS->AddNodeWithID( aNode->X(), aNode->Y(), aNode->Z(), aNode->GetID() );
  }

  /* =====================
   *  Paste mesh elements
   * ===================== */

  ActData_Mesh_ElementsIterator aMeshElemsIt(m_mesh, ActData_Mesh_ET_Face);
  for ( ; aMeshElemsIt.More(); aMeshElemsIt.Next() )
  {
    const Handle(ActData_Mesh_Element)& anElem = aMeshElemsIt.GetValue();

    // Proceed with TRIANGLE elements
    if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
    {
      Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(anElem);
      IntoMeshDS->AddElementWithID( aTriElem, aTriElem->GetID() );
    }
    // Proceed with QUADRANGLE elements
    else if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
    {
      Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(anElem);
      IntoMeshDS->AddElementWithID( aQuadElem, aQuadElem->GetID() );
    }
  }

  // Notice that we disable MDELTA recording here, in Paste method, as this
  // method is paired with DeltaOnAddition standard functionality rather
  // than with DeltaOnModification one. Ergo, if we bind a transient MDelta
  // to "IntoMesh" Attribute here, this Delta will bypass current transaction
  // and return into play in the next one. Obviously, this is not correct as
  // we normally want our Deltas to be kept alive during a single transaction
  // only. Moreover, there is no sense to have any MDelta for the Attribute
  // being pasted, as this Attribute is new one and does not require any
  // Modification Delta so.
  IntoMesh->SetMesh(IntoMeshDS, Standard_False);
}

//! Returns Modification Delta to be pushed to the bi-directional stack (to
//! its negative side in case of normal modification transactions and Redo,
//! and to its positive side in case of Undo).
//! \param Backup [in] not used Backup copy of the Main Attribute.
//! \return Modification Delta.
Handle(TDF_DeltaOnModification)
  ActData_MeshAttr::DeltaOnModification(const Handle(TDF_Attribute)& ActData_NotUsed(Backup)) const
{
  // We push a copy to the bi-directional stack as our own transient instance
  // will be cleaned up soon
  Handle(ActData_MeshMDelta) aResult = ( MDELTA.IsNull() ? NULL : MDELTA->DeepCopy() );

#if defined ACT_DEBUG && defined COUT_DEBUG
  std::cout << "\nDeltaOnModification [FROM]:" << std::endl;
  if ( !MDELTA.IsNull() )
    MDELTA->Dump(cout);

  std::cout << "\nDeltaOnModification [RESULT]:" << std::endl;
  if ( !aResult.IsNull() )
    aResult->Dump(cout);
#endif

  // Now clean up the owning Delta for the next transaction (if any)
  if ( !MDELTA.IsNull() )
    MDELTA->Clean();

  return aResult;
}

//! Returns Addition Delta.
//! \return Addition Delta.
Handle(TDF_DeltaOnAddition) ActData_MeshAttr::DeltaOnAddition() const
{
  return TDF_Attribute::DeltaOnAddition();
}

//! Enables Delta recording mode. This mode is turned ON by default, so
//! the only case you're supposed to use this method is when you have called
//! DeltaModeOff previously.
void ActData_MeshAttr::DeltaModeOn()
{
  m_bDeltaEnabled = Standard_True;
}

//! Disables Delta recording mode. This method is useful when huge amount
//! of data is being transferred to the Mesh Attribute. Normally you do not
//! want any Undo/Redo working for such cases.
void ActData_MeshAttr::DeltaModeOff()
{
  m_bDeltaEnabled = Standard_False;
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

//! Prepares new empty Mesh DS.
void ActData_MeshAttr::NewEmptyMesh()
{
  m_mesh = new ActData_Mesh();
}

//! Sets Mesh DS to store.
//! \param Mesh [in] mesh to store.
//! \param doDelta [in] indicates whether to apply Delta.
void ActData_MeshAttr::SetMesh(const Handle(ActData_Mesh)& Mesh,
                               const Standard_Boolean doDelta)
{
  if ( doDelta )
  {
    MDELTA_REPLACED_MESH(m_mesh, Mesh); // Deltalize replacement
  }
  m_mesh = Mesh;
}

//! Returns the stored Mesh DS.
//! \return stored mesh.
Handle(ActData_Mesh)& ActData_MeshAttr::GetMesh()
{
  return m_mesh;
}

//-----------------------------------------------------------------------------
// Manipulations with mesh
//-----------------------------------------------------------------------------

//! Creates new mesh node with the passed co-ordinates.
//! \param X [in] nodal X co-ordinate.
//! \param Y [in] nodal Y co-ordinate.
//! \param Z [in] nodal Z co-ordinate.
//! \return node ID.
Standard_Integer ActData_MeshAttr::AddNode(const Standard_Real X,
                                           const Standard_Real Y,
                                           const Standard_Real Z)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add node to Mesh DS
  Standard_Integer aResID = m_mesh->AddNode(X, Y, Z);

  MDELTA_ADDED_NODE(aResID, X, Y, Z); // Deltalize modification

  return aResID;
}

//! Adds the passed mesh node to the Mesh DS. This method is useful in cases
//! when you already have an ID for your node.
//! \param X [in] nodal X co-ordinate.
//! \param Y [in] nodal Y co-ordinate.
//! \param Z [in] nodal Z co-ordinate.
//! \param ID [in] node ID.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::AddNodeWithID(const Standard_Real X,
                                                 const Standard_Real Y,
                                                 const Standard_Real Z,
                                                 const Standard_Integer ID)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add node to Mesh DS
  Standard_Boolean aRes = m_mesh->AddNodeWithID(X, Y, Z, ID);

  MDELTA_ADDED_NODE(ID, X, Y, Z); // Deltalize modification

  return aRes;
}

//! Removes mesh node with the given ID.
//! \param ID [in] ID of the mesh node to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::RemoveNode(const Standard_Integer ID)
{
  // Attempt to remove the mesh node
  Standard_Boolean isOk = m_mesh->RemoveNode(ID);

  // Deltalize removal if it has been done successfully
  if ( isOk )
  {
    MDELTA_REMOVED_NODE(ID);
  }

  return isOk;
}

//! Creates new mesh element by the given mesh nodes.
//! \param Nodes [in] collection of nodal IDs comprising the mesh element.
//! \param NbNodes [in] number of Nodes.
//! \return element ID.
Standard_Integer ActData_MeshAttr::AddElement(Standard_Address Nodes,
                                              const Standard_Integer NbNodes)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add element to the underlying Mesh DS
  Standard_Integer aResID = m_mesh->AddFace(Nodes, NbNodes);
  
  // Deltalize modification
  if ( NbNodes == 3 )
  {
    MDELTA_ADDED_TRI(aResID, Nodes);
  }
  else if ( NbNodes == 4 )
  {
    MDELTA_ADDED_QUAD(aResID, Nodes);
  }
  else
    Standard_ProgramError::Raise("Unexpected number of nodes for delta");

  return aResID;
}

//! Creates new mesh element by the given mesh nodes. This method is useful
//! when you already have an ID for your element.
//! \param Nodes [in] collection of nodal IDs comprising the mesh element.
//! \param NbNodes [in] number of Nodes.
//! \param ID [in] ID of the element.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_MeshAttr::AddElementWithID(Standard_Address Nodes,
                                     const Standard_Integer NbNodes,
                                     const Standard_Integer ID)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add element to the underlying Mesh DS
  Standard_Boolean aRes = m_mesh->AddFaceWithID(Nodes, NbNodes, ID);
  
  // Deltalize modification
  if ( NbNodes == 3 )
  {
    MDELTA_ADDED_TRI(ID, Nodes);
  }
  else if ( NbNodes == 4 )
  {
    MDELTA_ADDED_QUAD(ID, Nodes);
  }
  else
    Standard_ProgramError::Raise("Unexpected number of nodes for delta");

  return aRes;
}

//! Removes mesh element with the given ID.
//! \param ID [in] ID of the mesh element to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::RemoveElement(const Standard_Integer ID)
{
  // Check if the requested element exists in Mesh DS
  Handle(ActData_Mesh_Element) anElem = m_mesh->FindElement(ID);
  if ( anElem.IsNull() )
    return Standard_False;

  // Remove element
  m_mesh->RemoveElement(anElem);

  // Deltalize removal
  if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
  {
    MDELTA_REMOVED_TRI(ID);
  }
  else if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
  {
    MDELTA_REMOVED_QUAD(ID);
  }
  else
    Standard_ProgramError::Raise("Unexpected type of element for delta");

  return Standard_True;
}

//-----------------------------------------------------------------------------
// Internal kernel methods
//-----------------------------------------------------------------------------

//! Checks whether modification is allowed on the Mesh Attribute. If not,
//! throws an appropriate exception.
void ActData_MeshAttr::assertModificationAllowed()
{
  if ( !Label().Data()->IsModificationAllowed() )
    Standard_ImmutableObject::Raise("ActData_MeshAttr changed outside transaction");

  if ( m_mesh.IsNull() )
    Standard_ProgramError::Raise("Mesh DS is NULL");
}
