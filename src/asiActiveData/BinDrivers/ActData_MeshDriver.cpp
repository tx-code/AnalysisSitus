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

// Own include
#include <ActData_MeshDriver.h>

// OCCT includes
#include <BinObjMgt_Persistent.hxx>

#undef COUT_DEBUG

//! Constructor accepting Message Driver for the parent class.
//! \param theMsgDriver [in] Message Driver for parent.
ActData_MeshDriver::ActData_MeshDriver(const Handle(Message_Messenger)& theMsgDriver)
: BinMDF_ADriver(theMsgDriver)
{
}

//! Creates an empty instance of Mesh Attribute for data transferring.
//! \return empty instance of Mesh Attribute.
Handle(TDF_Attribute) ActData_MeshDriver::NewEmpty() const
{
  return new ActData_MeshAttr();
}

//! Transfers data from PERSISTENT source of Mesh Attribute into its TRANSIENT
//! form. This method is a secondary one comparing to the dual Paste method
//! performing converse operation and defining the format rules (order and the
//! membership of the items being recorded) so.
//! \param FromPersistent [in] persistence buffer to transfer data into
//!                            transient instance of Mesh Attribute from.
//! \param ToTransient    [in] transient instance of Mesh Attribute being
//!                            assembled.
//! \param RelocTable     [in] not used (see OCAF reference manual).
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_MeshDriver::Paste(const BinObjMgt_Persistent&  FromPersistent,
                            const Handle(TDF_Attribute)& ToTransient,
                            BinObjMgt_RRelocationTable&  ActData_NotUsed(RelocTable)) const
{
  Handle(ActData_MeshAttr) aMeshAttr = Handle(ActData_MeshAttr)::DownCast(ToTransient);
  if ( aMeshAttr.IsNull() )
  {
    myMessageDriver->Send("ERROR: NULL Mesh Attribute", Message_Fail);
    return Standard_False;
  }

  // Disable accumulation of deltas
  aMeshAttr->DeltaModeOff();

  // Create new Mesh DS
  aMeshAttr->NewEmptyMesh();

  // Read from the input stream
  Read<BinObjMgt_Persistent>(FromPersistent, aMeshAttr);

  // Enable accumulation of deltas
  aMeshAttr->DeltaModeOn();

  return Standard_True;
}

//! Transfers data from transient instance of Mesh Attribute into the
//! persistence buffer for further binary storing.
//! \param FromTransient [in] transient Mesh Attribute source.
//! \param ToPersistent [in] persistence buffer to transfer data to.
//! \param RelocTable [in] not used (see OCAF reference manual).
void ActData_MeshDriver::Paste(const Handle(TDF_Attribute)& FromTransient,
                               BinObjMgt_Persistent&        ToPersistent,
                               BinObjMgt_SRelocationTable&  ActData_NotUsed(RelocTable)) const
{
  /* ====================================================
   *  Access Mesh Attribute along with contained Mesh DS
   * ==================================================== */

  Handle(ActData_MeshAttr) aMeshAttr = Handle(ActData_MeshAttr)::DownCast(FromTransient);
  if ( aMeshAttr.IsNull() )
  {
    myMessageDriver->Send("ERROR: NULL Mesh Attribute", Message_Fail);
    return;
  }

  // Write to the output stream
  Write<BinObjMgt_Persistent>(aMeshAttr, ToPersistent);
}
