//-----------------------------------------------------------------------------
// Created on: 03 September 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiVisu_HatchingPipeline.h>

// asiVisu includes
#include <asiVisu_CurveSource.h>
#include <asiVisu_HatchingDataProvider.h>
#include <asiVisu_Utils.h>

// OpenCascade includes
#include <Adaptor3d_HCurve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <DBRep_IsoBuilder.hxx>
#include <DBRep_ListOfFace.hxx>
#include <Draw_Color.hxx>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

namespace
{
  static double IsoRatio     = 1.001;
  static int    MaxPlotCount = 5; // To avoid huge recursive calls in
  static int    PlotCount    = 0; // PlotIso for cases of "bad" curves and surfaces
                                  // Set PlotCount = 0 before first call of PlotIso

  void PlotIso(Handle(DBRep_Face)&  F,
               BRepAdaptor_Surface& S,
               GeomAbs_IsoType      T,
               double&              U,
               double&              V,
               double               Step,
               std::vector<gp_XYZ>& polyline)
  {
    ++PlotCount;
    gp_Pnt Pl, Pr, Pm;

    if (T == GeomAbs_IsoU)
    {
      S.D0(U, V, Pl);
      S.D0(U, V + Step/2., Pm);
      S.D0(U, V + Step, Pr);
    }
    else
    {
      S.D0(U, V, Pl);
      S.D0(U + Step/2., V, Pm);
      S.D0(U + Step, V, Pr);
    }

    if (PlotCount > MaxPlotCount) {
      polyline.push_back( Pr.XYZ() );
      return;
    }

    if (Pm.Distance(Pl) + Pm.Distance(Pr) <= IsoRatio*Pl.Distance(Pr)) {
      polyline.push_back( Pr.XYZ() );
    } else 
       if (T == GeomAbs_IsoU) {
         PlotIso (F, S, T, U, V, Step/2, polyline);
         Standard_Real aLocalV = V + Step/2 ;
         PlotIso (F, S, T, U, aLocalV, Step/2, polyline);
       } else {
         PlotIso (F, S, T, U, V, Step/2, polyline);
         Standard_Real aLocalU = U + Step/2 ;
         PlotIso (F, S, T, aLocalU, V, Step/2, polyline);
       }
  }

  void BuildIsos(const TopoDS_Face&                  ff,
                 std::vector< std::vector<gp_XYZ> >& isos)
  {
    TopoDS_Face face = TopoDS::Face( ff.Oriented(TopAbs_FORWARD) );

    GeomAbs_IsoType T;
    double Par, T1, T2, U1, U2, V1, V2, stepU = 0., stepV = 0.;
    gp_Pnt P;
    int i, j;

    DBRep_IsoBuilder IsoBuild(face, 100, 10);

    Draw_Color color;
    DBRep_ListOfFace faces;
    Handle(DBRep_Face) F = new DBRep_Face(face, IsoBuild.NbDomains(), color);
    IsoBuild.LoadIsos(F);
    //
    BRepAdaptor_Surface S(F->Face(), false);
    GeomAbs_SurfaceType SurfType = S.GetType();
    GeomAbs_CurveType   CurvType = GeomAbs_OtherCurve;
    //
    const int N = F->NbIsos();
    //
    int myDiscret = 100;
    //
    int Intrv, nbIntv;
    int nbUIntv = S.NbUIntervals(GeomAbs_CN);
    int nbVIntv = S.NbVIntervals(GeomAbs_CN);
    TColStd_Array1OfReal TI(1, Max(nbUIntv, nbVIntv) + 1);
    //
    for ( i = 1; i <= N; ++i )
    {
      std::vector<gp_XYZ> polyline;

      F->GetIso(i,T,Par,T1,T2);
      if (T == GeomAbs_IsoU) {
        S.VIntervals(TI, GeomAbs_CN);
        V1 = Max(T1, TI(1));
        V2 = Min(T2, TI(2));
        U1 = Par;
        U2 = Par;
        stepU = 0;
        nbIntv = nbVIntv;
      }
      else {
        S.UIntervals(TI, GeomAbs_CN);
        U1 = Max(T1, TI(1));
        U2 = Min(T2, TI(2));
        V1 = Par;
        V2 = Par;
        stepV = 0;
        nbIntv = nbUIntv;
      }  
  
      S.D0(U1,V1,P);
      polyline.push_back( P.XYZ() );

       for (Intrv = 1; Intrv <= nbIntv; Intrv++) {

        if (TI(Intrv) <= T1 && TI(Intrv + 1) <= T1)
          continue;
        if (TI(Intrv) >= T2 && TI(Intrv + 1) >= T2)
           continue;
        if (T == GeomAbs_IsoU) {
          V1 = Max(T1, TI(Intrv));
          V2 = Min(T2, TI(Intrv + 1));
          stepV = (V2 - V1) / myDiscret;
        }
        else {
          U1 = Max(T1, TI(Intrv));
          U2 = Min(T2, TI(Intrv + 1));
          stepU = (U2 - U1) / myDiscret;
        }

        switch (SurfType) {
    //-------------GeomAbs_Plane---------------
        case GeomAbs_Plane :
          break;
    //----GeomAbs_Cylinder   GeomAbs_Cone------
        case GeomAbs_Cylinder :
        case GeomAbs_Cone :
          if (T == GeomAbs_IsoV) {
            for (j = 1; j < myDiscret; j++) {
        U1 += stepU;
        V1 += stepV;
        S.D0(U1,V1,P);
        polyline.push_back( P.XYZ() );
            }
          }
          break;
    //---GeomAbs_Sphere   GeomAbs_Torus--------
    //GeomAbs_BezierSurface GeomAbs_BezierSurface
        case GeomAbs_Sphere :
        case GeomAbs_Torus :
        case GeomAbs_OffsetSurface :
        case GeomAbs_OtherSurface :
          for (j = 1; j < myDiscret; j++) {
            U1 += stepU;
            V1 += stepV;
            S.D0(U1,V1,P);
            polyline.push_back( P.XYZ() );
          }
          break;
    //-------------GeomAbs_BSplineSurface------
        case GeomAbs_BezierSurface :
        case GeomAbs_BSplineSurface :
          for (j = 1; j <= myDiscret/2; j++) {
            Handle(DBRep_Face) aLocalFace = F;

            PlotCount = 0;

            PlotIso (aLocalFace , S, T, U1, V1, (T == GeomAbs_IsoV) ? stepU*2. : stepV*2., polyline);
            U1 += stepU*2.;
            V1 += stepV*2.;
          }
          break;
    //-------------GeomAbs_SurfaceOfExtrusion--
    //-------------GeomAbs_SurfaceOfRevolution-
        case GeomAbs_SurfaceOfExtrusion :
        case GeomAbs_SurfaceOfRevolution :
          if ((T == GeomAbs_IsoV && SurfType == GeomAbs_SurfaceOfRevolution) ||
        (T == GeomAbs_IsoU && SurfType == GeomAbs_SurfaceOfExtrusion)) {
            if (SurfType == GeomAbs_SurfaceOfExtrusion) break;
            for (j = 1; j < myDiscret; j++) {
        U1 += stepU;
        V1 += stepV;
        S.D0(U1,V1,P);
        polyline.push_back( P.XYZ() );
            }
          } else {
            CurvType = (S.BasisCurve())->GetType();
            switch (CurvType) {
            case GeomAbs_Line :
        break;
            case GeomAbs_Circle :
            case GeomAbs_Ellipse :
        for (j = 1; j < myDiscret; j++) {
          U1 += stepU;
          V1 += stepV;
          S.D0(U1,V1,P);
          polyline.push_back( P.XYZ() );
        }
        break;
            case GeomAbs_Parabola :
            case GeomAbs_Hyperbola :
            case GeomAbs_BezierCurve :
            case GeomAbs_BSplineCurve :
            case GeomAbs_OffsetCurve :
            case GeomAbs_OtherCurve :
        for (j = 1; j <= myDiscret/2; j++) {
          Handle(DBRep_Face) aLocalFace = F;

          PlotCount = 0;

          PlotIso (aLocalFace, S, T, U1, V1,
             (T == GeomAbs_IsoV) ? stepU*2. : stepV*2., polyline);
          U1 += stepU*2.;
          V1 += stepV*2.;
        }
        break;
            }
          }
        }
      }
      S.D0(U2,V2,P);
      polyline.push_back( P.XYZ() );

      isos.push_back(polyline);
    }
  }
}

//-----------------------------------------------------------------------------

//! Creates new Hatching Pipeline initialized by default VTK mapper and actor.
asiVisu_HatchingPipeline::asiVisu_HatchingPipeline()
//
: asiVisu_Pipeline( vtkSmartPointer<vtkPolyDataMapper>::New(),
                    vtkSmartPointer<vtkActor>::New() )
{
}

//-----------------------------------------------------------------------------

//! Sets input data for the pipeline.
//! \param[in] DP Data Provider.
void asiVisu_HatchingPipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_HatchingDataProvider)
    faceProvider = Handle(asiVisu_HatchingDataProvider)::DownCast(DP);

  /* =================
   *  Validate inputs
   * ================= */

  TopoDS_Face face = faceProvider->GetFace();
  if ( face.IsNull() )
  {
    // Pass empty data set in order to have valid pipeline
    vtkSmartPointer<vtkPolyData> dummyDS = vtkSmartPointer<vtkPolyData>::New();
    this->SetInputData(dummyDS);
    this->Modified(); // Update modification timestamp
    return; // Do nothing
  }

  /* ============================
   *  Prepare polygonal data set
   * ============================ */

  if ( faceProvider->MustExecute( this->GetMTime() ) )
  {
    std::vector< std::vector<gp_XYZ> > isos;
    ::BuildIsos( faceProvider->GetFace(), isos );

    // Append filter
    vtkSmartPointer<vtkAppendPolyData>
      appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    for ( const auto& iso : isos )
    {
      // Allocate Data Source
      vtkSmartPointer<asiVisu_CurveSource>
        curveSource = vtkSmartPointer<asiVisu_CurveSource>::New();

      // Curve representation
      Handle(Geom_Curve) isoCurve = asiAlgo_Utils::PolylineAsSpline(iso);

      // Set geometry to be converted to VTK polygonal DS
      if ( !curveSource->SetInputCurve( isoCurve, isoCurve->FirstParameter(), isoCurve->LastParameter() ) )
        continue; // No poly data produced

      // Append poly data
      appendFilter->AddInputConnection( curveSource->GetOutputPort() );
    }

    // Chain pipeline
    this->SetInputConnection( appendFilter->GetOutputPort() );
  }

  // Update modification timestamp
  this->Modified();
}

//-----------------------------------------------------------------------------

//! Callback for AddToRenderer() routine. Good place to adjust visualization
//! properties of the pipeline's actor.
void asiVisu_HatchingPipeline::callback_add_to_renderer(vtkRenderer*)
{}

//! Callback for RemoveFromRenderer() routine.
void asiVisu_HatchingPipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//! Callback for Update() routine.
void asiVisu_HatchingPipeline::callback_update()
{}
