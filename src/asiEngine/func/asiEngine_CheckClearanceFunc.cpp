//-----------------------------------------------------------------------------
// Created on: 14 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
#include <asiEngine_CheckClearanceFunc.h>

// asiData includes
#include <asiData_ClearanceNode.h>
#include <asiData_MeshParameter.h>

// asiAlgo includes
#include <asiAlgo_CheckClearance.h>
#include <asiAlgo_Timer.h>

// Active Data includes
#include <ActData_ParameterFactory.h>
#include <ActData_UserExtParameter.h>

#if defined USE_MOBIUS
#include <mobius/cascade.h>
using namespace mobius;
#endif

//-----------------------------------------------------------------------------

int asiEngine_CheckClearanceFunc::execute(const Handle(ActAPI_HParameterList)& inputs,
                                          const Handle(ActAPI_HParameterList)& outputs,
                                          const Handle(Standard_Transient)&) const
{
#if defined USE_MOBIUS
  ActAPI_ProgressEntry progress = this->GetProgressNotifier();

  /* ============================
   *  Interpret input Parameters.
   * ============================ */

  // Get mesh.
  Handle(ActData_UserExtParameter)
    trisExtParam = Handle(ActData_UserExtParameter)::DownCast( inputs->Value(1) );
  //
  Handle(ActAPI_INode) ownerNode = trisExtParam->GetNode();

  // Get the custom Mesh Parameter.
  Handle(asiData_MeshParameter)
    trisParam = Handle(asiData_MeshParameter)::DownCast( ownerNode->Parameter( trisExtParam->GetParamId() ) );

  t_ptr<poly_Mesh> tris = trisParam->GetMesh();

  // Get Clearance Node.
  Handle(asiData_ClearanceNode)
    CN = Handle(asiData_ClearanceNode)::DownCast( trisParam->GetNode() );

  /* ===================
   *  Analyze clearance.
   * =================== */

  TIMER_NEW
  TIMER_GO

  // Initialize the algorithm.
  asiAlgo_CheckClearance algo(tris, m_progress, m_plotter);

  // Perform.
  if ( !algo.Perform() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Clearance analysis failed.");
    return 1; // Error.
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(progress, "Check clearance")

  /* =======================
   *  Set output Parameters.
   * ======================= */

  // Set the clearance field.
  CN->SetMeshWithScalars( algo.GetClearanceField() );

  // Set the extreme clearance values.
  ActParamTool::AsReal( outputs->Value(1) )->SetValue( *algo.GetMinClearance() );
  ActParamTool::AsReal( outputs->Value(2) )->SetValue( *algo.GetMaxClearance() );

  return 0; // Success.
#else
  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return 1;
#endif
}

//-----------------------------------------------------------------------------

ActAPI_ParameterTypeStream
  asiEngine_CheckClearanceFunc::inputSignature() const
{
  return ActAPI_ParameterTypeStream() << Parameter_PolyMesh; // Mesh of a CAD part to check.
}

//-----------------------------------------------------------------------------

ActAPI_ParameterTypeStream
  asiEngine_CheckClearanceFunc::outputSignature() const
{
  return ActAPI_ParameterTypeStream() << Parameter_Real  // Min clearance.
                                      << Parameter_Real; // Max clearance.
}
