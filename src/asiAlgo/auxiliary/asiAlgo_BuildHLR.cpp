//-----------------------------------------------------------------------------
// Created on: 24 December 2020
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
#include <asiAlgo_BuildHLR.h>

// asiAlgo includes
#include <asiAlgo_Timer.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepLib.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <OSD_Thread.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

// Initialize static thread data for HLR algorithms.
asiAlgo_BuildHLR::t_threadData
  asiAlgo_BuildHLR::ThreadData[2] = { asiAlgo_BuildHLR::t_threadData(),
                                      asiAlgo_BuildHLR::t_threadData() };

//-----------------------------------------------------------------------------

namespace hlr
{
  //! Builds 3D curves out of the 2D curves constructed by HLR.
  const TopoDS_Shape& Build3dCurves(const TopoDS_Shape& shape)
  {
    for ( TopExp_Explorer it(shape, TopAbs_EDGE); it.More(); it.Next() )
      BRepLib::BuildCurve3d( TopoDS::Edge( it.Current() ) );

    return shape;
  }

  //! Runs precise HLR.
  TopoDS_Shape
    HLR(const TopoDS_Shape&                    shape,
        const gp_Dir&                          direction,
        const asiAlgo_BuildHLR::t_outputEdges& visibility)
  {
    Handle(HLRBRep_Algo) brep_hlr = new HLRBRep_Algo;
    brep_hlr->Add(shape);

    gp_Ax2 transform(gp::Origin(), direction);
    HLRAlgo_Projector projector(transform);
    brep_hlr->Projector(projector);
    brep_hlr->Update();
    brep_hlr->Hide();

    // Extract the result sets.
    HLRBRep_HLRToShape shapes(brep_hlr);

    // V -- visible
    // H -- hidden
    TopoDS_Shape V  = Build3dCurves( shapes.VCompound       () ); // "hard edges" visible
    TopoDS_Shape V1 = Build3dCurves( shapes.Rg1LineVCompound() ); // "smooth edges" visible
    TopoDS_Shape VN = Build3dCurves( shapes.RgNLineVCompound() ); // "contour edges" visible
    TopoDS_Shape VO = Build3dCurves( shapes.OutLineVCompound() ); // "outline" visible
    TopoDS_Shape VI = Build3dCurves( shapes.IsoLineVCompound() ); // "isolines" visible (precise HLR only)
    TopoDS_Shape H  = Build3dCurves( shapes.HCompound       () ); // "hard edges" hidden
    TopoDS_Shape H1 = Build3dCurves( shapes.Rg1LineHCompound() ); // "smooth edges" hidden
    TopoDS_Shape HN = Build3dCurves( shapes.RgNLineHCompound() ); // "contour edges" hidden
    TopoDS_Shape HO = Build3dCurves( shapes.OutLineHCompound() ); // "outline" hidden
    TopoDS_Shape HI = Build3dCurves( shapes.IsoLineHCompound() ); // "isolines" hidden (precise HLR only)

    TopoDS_Compound C;
    BRep_Builder().MakeCompound(C);
    //
    if ( !V.IsNull() && visibility.OutputVisibleSharpEdges)
      BRep_Builder().Add(C, V);
    //
    if ( !V1.IsNull() && visibility.OutputVisibleSmoothEdges)
      BRep_Builder().Add(C, V1);
    //
    if ( !VN.IsNull() && visibility.OutputVisibleOutlineEdges)
      BRep_Builder().Add(C, VN);
    //
    if ( !VO.IsNull() && visibility.OutputVisibleSewnEdges)
      BRep_Builder().Add(C, VO);
    //
    if ( !VI.IsNull() && visibility.OutputVisibleIsoLines)
      BRep_Builder().Add(C, VI);
    //
    if ( !H.IsNull() && visibility.OutputHiddenSharpEdges)
      BRep_Builder().Add(C, H);
    //
    if ( !H1.IsNull() && visibility.OutputHiddenSmoothEdges)
      BRep_Builder().Add(C, H1);
    //
    if ( !HN.IsNull() && visibility.OutputHiddenOutlineEdges)
      BRep_Builder().Add(C, HN);
    //
    if ( !HO.IsNull() && visibility.OutputHiddenSewnEdges)
      BRep_Builder().Add(C, HO);
    //
    if ( !HI.IsNull() && visibility.OutputHiddenIsoLines)
      BRep_Builder().Add(C, HI);

    gp_Trsf T;
    T.SetTransformation( gp_Ax3(transform) );
    T.Invert();

    return C.Moved(T);
  }

  //! Runs discrete HLR.
  TopoDS_Shape
    DHLR(const TopoDS_Shape&                    shape,
         const gp_Dir&                          direction,
         const asiAlgo_BuildHLR::t_outputEdges& visibility)
  {
    gp_Ax2 transform(gp::Origin(), direction);

    // Prepare projector.
    HLRAlgo_Projector projector(transform);

    // Prepare polygonal HLR algorithm which is known to be more reliable than
    // the "curved" version of HLR.
    Handle(HLRBRep_PolyAlgo) polyAlgo = new HLRBRep_PolyAlgo;
    //
    polyAlgo->Projector(projector);
    polyAlgo->Load(shape);
    polyAlgo->Update();

    // Create topological entities.
    HLRBRep_PolyHLRToShape shapes;
    shapes.Update(polyAlgo);

    // V -- visible
    // H -- hidden
    TopoDS_Shape V  = shapes.VCompound       (); // "hard edges" visible
    TopoDS_Shape V1 = shapes.Rg1LineVCompound(); // "smooth edges" visible
    TopoDS_Shape VN = shapes.RgNLineVCompound(); // "contour edges" visible
    TopoDS_Shape VO = shapes.OutLineVCompound(); // "outline" visible
    TopoDS_Shape H  = shapes.HCompound       (); // "hard edges" hidden
    TopoDS_Shape H1 = shapes.Rg1LineHCompound(); // "smooth edges" hidden
    TopoDS_Shape HN = shapes.RgNLineHCompound(); // "contour edges" hidden
    TopoDS_Shape HO = shapes.OutLineHCompound(); // "outline" hidden

    TopoDS_Compound C;
    BRep_Builder().MakeCompound(C);
    //
    if ( !V.IsNull() && visibility.OutputVisibleSharpEdges)
      BRep_Builder().Add(C, V);
    //
    if ( !V1.IsNull() && visibility.OutputVisibleSmoothEdges)
      BRep_Builder().Add(C, V1);
    //
    if ( !VN.IsNull() && visibility.OutputVisibleOutlineEdges)
      BRep_Builder().Add(C, VN);
    //
    if ( !VO.IsNull() && visibility.OutputVisibleSewnEdges)
      BRep_Builder().Add(C, VO);
    //
    if ( !H.IsNull() && visibility.OutputHiddenSharpEdges)
      BRep_Builder().Add(C, H);
    //
    if ( !H1.IsNull() && visibility.OutputHiddenSmoothEdges)
      BRep_Builder().Add(C, H1);
    //
    if ( !HN.IsNull() && visibility.OutputHiddenOutlineEdges)
      BRep_Builder().Add(C, HN);
    //
    if ( !HO.IsNull() && visibility.OutputHiddenSewnEdges)
      BRep_Builder().Add(C, HO);

    gp_Trsf T;
    T.SetTransformation( gp_Ax3(transform) );
    T.Invert();

    return C.Moved(T);
  }

  //! Thread function for precise HLR.
  void* ThreadHLR(void* pData)
{
  std::cout << "Running HLR in worker thread id: " << OSD_Thread::Current() << std::endl;

  TIMER_NEW
  TIMER_GO

  asiAlgo_BuildHLR::t_threadData*
    pThreadData = reinterpret_cast<asiAlgo_BuildHLR::t_threadData*>(pData);

  pThreadData->output = HLR(pThreadData->input,
                            pThreadData->dir,
                            pThreadData->style);

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("HLR finished")

  return NULL;
}

  //! Thread function for discrete HLR.
  void* ThreadDHLR(void* pData)
  {
    std::cout << "Running DHLR in worker thread id: " << OSD_Thread::Current() << std::endl;

    TIMER_NEW
    TIMER_GO

    asiAlgo_BuildHLR::t_threadData*
      pThreadData = reinterpret_cast<asiAlgo_BuildHLR::t_threadData*>(pData);

    pThreadData->output = DHLR(pThreadData->input,
                               pThreadData->dir,
                               pThreadData->style);

    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("DHLR finished")

    return NULL;
  }
}

//-----------------------------------------------------------------------------

asiAlgo_BuildHLR::asiAlgo_BuildHLR(const TopoDS_Shape&  shape,
                                   ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_input           (shape)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildHLR::Perform(const gp_Dir&        projectionDir,
                               const Mode           mode,
                               const t_outputEdges& visibility)
{
  switch ( mode )
  {
    case Mode_Precise:
    {
      m_result = hlr::HLR(m_input, projectionDir, visibility);
      break;
    }
    case Mode_Discrete:
    {
      m_result = hlr::DHLR(m_input, projectionDir, visibility);
      break;
    }
    default:
      break;
  }
  return !m_result.IsNull();
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildHLR::PerformParallel(const gp_Dir&        projectionDir,
                                       const int            timeout_ms,
                                       const t_outputEdges& visibility)
{
  std::cout << "Running in master thread id: " << OSD_Thread::Current() << std::endl;

  // Prepare threads.
  OSD_Thread threads[2];
  //
  threads[0].SetFunction(hlr::ThreadHLR);
  threads[1].SetFunction(hlr::ThreadDHLR);

  /*
   * Prepare data. Shape is passed as a shallow pointer to be deep-copied
   * in the ctor of `t_threadData`. The copy of the `shape` should stay alive
   * as long as the assigned thread is running.
   */

  ThreadData[0].input  = BRepBuilderAPI_Copy(m_input, true, true);
  ThreadData[0].dir    = projectionDir;
  ThreadData[0].style  = visibility;
  ThreadData[0].output = TopoDS_Shape();
  //
  ThreadData[1].input  = BRepBuilderAPI_Copy(m_input, true, true); // copy mesh for DHLR
  ThreadData[1].dir    = projectionDir;
  ThreadData[1].style  = visibility;
  ThreadData[1].output = TopoDS_Shape();

  // Run threads.
  for ( int i = 0; i < 2; ++i )
  {
    if ( !threads[i].Run(&ThreadData[i]) )
      std::cerr << "Error: cannot start thread " << i << std::endl;
  }

  for ( int i = 0; i < 2; ++i )
  {
    Standard_Address res;

    // In posix, The pthread_join() function waits for the thread to terminate.
    // If that thread has already terminated, then pthread_join() returns
    // immediately.
    if ( !threads[i].Wait(timeout_ms, res) )
      std::cerr << "Error: cannot get result of thread " << threads[i].GetId() << std::endl;
  }

  m_result = ThreadData[0].output.IsNull() ? ThreadData[1].output : ThreadData[0].output;
  return !m_result.IsNull();
}

//-----------------------------------------------------------------------------

const TopoDS_Shape& asiAlgo_BuildHLR::GetResult() const
{
  return m_result;
}
