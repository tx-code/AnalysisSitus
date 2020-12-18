//-----------------------------------------------------------------------------
// Created on: 17 September 2020
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

// cmdMobius includes
#include <cmdMobius.h>

// asiEngine includes
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_Tessellation.h>
#include <asiEngine_Triangulation.h>

// asiAlgo includes
#include <asiAlgo_MeshComputeNorms.h>
#include <asiAlgo_Timer.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

#ifdef USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/cascade_Triangulation.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

Handle(asiEngine_Model)        cmdMobius::model = nullptr;
Handle(asiUI_CommonFacilities) cmdMobius::cf    = nullptr;

//-----------------------------------------------------------------------------

void cmdMobius::ClearViewers(const bool repaint)
{
  if ( cf.IsNull() )
    return;

  // Get all presentation managers
  const vtkSmartPointer<asiVisu_PrsManager>& partPM   = cf->ViewerPart->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& hostPM   = cf->ViewerHost->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& domainPM = cf->ViewerDomain->PrsMgr();

  // Update viewers
  partPM  ->DeleteAllPresentations();
  hostPM  ->DeleteAllPresentations();
  domainPM->DeleteAllPresentations();

  if ( repaint )
  {
    cf->ViewerPart->Repaint();
    cf->ViewerHost->Repaint();
    cf->ViewerDomain->Repaint();
  }
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_ComputeNorms(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  // Convert to Mobius.
  t_ptr<t_mesh> mesh = cascade::GetMobiusMesh(tris);

  // Compute norm of each triangle.
  Handle(HIntArray)                elemIds;
  Handle(HRealArray)               elemNorms;
  NCollection_DataMap<int, gp_Vec> norms;
  int                              tidx = 0;
  //
  for ( t_mesh::TriangleIterator tit(mesh); tit.More(); tit.Next(), ++tidx )
  {
    poly_TriangleHandle ht = tit.Current();

    // Compute norm.
    t_xyz N;
    if ( !mesh->ComputeNormal(ht, N) )
      continue;

    norms.Bind( tidx, cascade::GetOpenCascadeVec(N) );
  }

  // Convert to plain arrays.
  asiAlgo_MeshComputeNorms::GetResultArrays(norms, elemIds, elemNorms);

  // Create Data Node for the norms.
  Handle(asiData_MeshNormsNode) norms_n;
  //
  cmdMobius::model->OpenCommand();
  {
    norms_n = asiEngine_Tessellation(cmdMobius::model).CreateNorms(tris_n,
                                                                   "Normal field",
                                                                   true); // Elemental.
    //
    norms_n->SetIDs(elemIds);
    norms_n->SetVectors(elemNorms);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(norms_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  cmdMobius_NotUsed(argc);
  cmdMobius_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

void cmdMobius::Factory(const Handle(asiTcl_Interp)&      interp,
                        const Handle(Standard_Transient)& data)
{
  static const char* group = "cmdMobius";

  /* ==========================
   *  Initialize UI facilities.
   * ========================== */

  // Get common facilities.
  Handle(asiUI_CommonFacilities)
    passedCF = Handle(asiUI_CommonFacilities)::DownCast(data);
  //
  if ( passedCF.IsNull() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "[cmdMobius] UI facilities are not available. GUI may not be updated.");
  else
    cf = passedCF;

  /* ================================
   *  Initialize Data Model instance.
   * ================================ */

  model = Handle(asiEngine_Model)::DownCast( interp->GetModel() );
  //
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdMobius] Data Model instance is null or not of asiEngine_Model kind.");
    return;
  }

  /* ==================
   *  Add Tcl commands.
   * ================== */

  //-------------------------------------------------------------------------//
  interp->AddCommand("mobius-poly-compute-norms",
    //
    "mobius-poly-compute-norms\n"
    "\n"
    "\t Computes normal field.",
    //
    __FILE__, group, MOBIUS_POLY_ComputeNorms);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdMobius)
