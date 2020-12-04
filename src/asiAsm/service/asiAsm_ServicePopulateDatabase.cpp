//-----------------------------------------------------------------------------
// Created on: 22 September 2020 (*)
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <service_PopulateDatabase.h>

// asiAsm includes
#include <service_Model.h>
#include <service_Prototype.h>
#include <xde_AssemblyDoc.h>
#include <xde_AssemblyGraph.h>

// asiUI includes
#include <asiUI_Common.h>

using namespace asiAsm;

//-----------------------------------------------------------------------------

bool service::PopulateDatabase(const Handle(service::Model)&   asiAsmModel,
                               const Handle(xde::AssemblyDoc)& xdeModel,
                               ActAPI_ProgressEntry            progress)
{
  /* =================
   *  Construct parts.
   * ================= */

  // Get part IDs from XDE.
  xde::PartIds pids;
  xdeModel->GetParts(pids);
  //
  progress.SendLogMessage( LogInfo(Normal) << "XDE contents: %1 part(s)."
                                           << pids.Length() );

  // Service for parts.
  Prototype apiProto(asiAsmModel->GetDbName(), progress);

  // Create parts.
  for ( xde::PartIds::Iterator pit(pids); pit.More(); pit.Next() )
  {
    const xde::PartId& pid = pit.Value();

    std::string name = ExtStr2StdStr( xdeModel->GetPartName(pid) );

    const int asiAsmPid = apiProto.Create( entity::Prototype(name) );

    progress.SendLogMessage(LogInfo(Normal) << "Created part '%1' with new id %2."
                                            << name << asiAsmPid);
  }

  /* ==========================
   *  Construct assembly graph.
   * ========================== */

  Handle(xde::AssemblyGraph) asmGraph = new xde::AssemblyGraph(xdeModel);

  for ( xde::AssemblyGraph::Iterator asmIt(asmGraph); asmIt.More(); asmIt.Next() )
  {
    const int nid = asmIt.GetCurrentNode();

    //asmGraph->
  }

  //asiAsmModel->CreatePart()

  //// Prepare DAO for parts.
  //Handle(mgc_PartDAO)
  //  partDao = new mgc_PartDAO(Mongo_Default_URI, m_progress);

  //// Get all parts of the model.
  //core_PartIds partIds;
  //xdeModel->GetParts(partIds);

  //// Transfer each part individually.
  //for ( core_PartIds::Iterator pit(partIds); pit.More(); pit.Next() )
  //{
  //  const core_PartId& pid = pit.Value();

  //  // Prepare DTO.
  //  mgc_PartDTO partDto;
  //  partDto.name = TCollection_AsciiString( xdeModel->GetPartName(pid) ).ToCString();

  //  // Create db entity (document).
  //  mgc_ObjectId id;
  //  //
  //  if ( !partDao->apiCreate(partDto, id) )
  //  {
  //    m_progress.SendLogMessage(LogErr(Normal) << "Cannot create part with ID %1 in the database."
  //                                             << pid);
  //    continue;
  //  }
  //}

  return true;
}
