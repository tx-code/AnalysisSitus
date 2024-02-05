//-----------------------------------------------------------------------------
// Created on: 20 November 2015
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

#ifndef asiVisu_Utils_h
#define asiVisu_Utils_h

// asiVisu includes
#include <asiVisu_Prs.h>

// asiAlgo includes
#include <asiAlgo_FeatureAngleType.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkAxesActor.h>
#include <vtkCubeAxesActor.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkInteractorObserver.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTextWidget.h>
#pragma warning(pop)

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <NCollection_Sequence.hxx>
#include <Precision.hxx>
#include <Standard_TypeDef.hxx>
#include <TCollection_AsciiString.hxx>

//-----------------------------------------------------------------------------

#define INF_LIMIT 100
#define MAX_COLOR_SCALE 255.

//-----------------------------------------------------------------------------

//! Pointer to Node allocation routine.
typedef Handle(asiVisu_Prs) (*asiVisu_PrsAllocator)(const Handle(ActAPI_INode)&);

#define DEFINE_PRESENTATION_FACTORY(CNode, AllocFunction) \
  static void RegisterPrs() \
  { \
    asiVisu_Utils::RegisterPrsType(STANDARD_TYPE(CNode)->Name(), AllocFunction); \
  }

#define REGISTER_PRESENTATION(C) \
  C::RegisterPrs();

//-----------------------------------------------------------------------------

//! Orientations.
enum asiVisu_Orientation
{
  VisuOri_Undefined = -1,
  VisuOri_Forward   =  0,
  VisuOri_Reversed  =  1,
  VisuOri_Internal  =  2,
  VisuOri_External  =  3,
  VisuOri_Last
};

//-----------------------------------------------------------------------------

//! Curvature combs.
enum asiVisu_CurvatureCombElem
{
  VisuCurvComb_Undefined    = -1,
  VisuCurvComb_PointOk      =  0,
  VisuCurvComb_PointFailure =  1,
  VisuCurvComb_Comb         =  2,
  VisuCurvComb_Envelope     =  3,
  VisuCurvComb_Last
};

//-----------------------------------------------------------------------------

//! Isos continuity.
enum asiVisu_IsosCont
{
  VisuIsosCont_Undefined = -1,
  VisuIsosCont_C0        =  0,
  VisuIsosCont_C1        =  1,
  VisuIsosCont_C2        =  2,
  VisuIsosCont_CN        =  3,
  VisuIsosCont_Last
};

//-----------------------------------------------------------------------------

//! 3D axes frame.
enum asiVisu_Axis
{
  VisuAxis_Undefined = -1,
  VisuAxis_X         =  0,
  VisuAxis_Y         =  1,
  VisuAxis_Z         =  2,
  VisuAxis_Last
};

//-----------------------------------------------------------------------------

//! Curvilinear axes.
enum asiVisu_CurviAxis
{
  VisuCurviAxis_Undefined = -1,
  VisuCurviAxis_U         =  0,
  VisuCurviAxis_V         =  1,
  VisuCurviAxis_Last
};

//-----------------------------------------------------------------------------

//! Common visualization utilities.
class asiVisu_Utils
{
// Presentation factory:
public:

  //! Mapping between Node types and allocation routines.
  typedef NCollection_DataMap<TCollection_AsciiString, asiVisu_PrsAllocator> TPrsAllocMap;

  asiVisu_EXPORT static TPrsAllocMap
    RegisterPrsType(const TCollection_AsciiString& theType,
                    const asiVisu_PrsAllocator     theAllocFunc);

  asiVisu_EXPORT static const TPrsAllocMap&
    GetAllocMap();

// Scene:
public:

  asiVisu_EXPORT static int
    ComputeVisiblePropBounds(vtkRenderer*       theRenderer,
                             double             theBounds[6],
                             vtkPropCollection* thePropsToSkip);

  asiVisu_EXPORT static void
    ResetCamera(vtkRenderer* renderer);

  asiVisu_EXPORT static bool
    AdjustCamera(vtkRenderer*       theRenderer,
                 vtkPropCollection* thePropsToSkip,
                 bool               isDefaultNorm   = false,
                 bool               doScaling       = true);

  asiVisu_EXPORT static void
    AdjustCameraClippingRange(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnTop(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnBottom(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnFront(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnBack(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnLeft(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    CameraOnRight(vtkRenderer* theRenderer);

  asiVisu_EXPORT static void
    ApplyLightingRules(vtkActor* theActor);

  asiVisu_EXPORT static void
    ApplyLightingRulesDark(vtkActor* theActor);

  asiVisu_EXPORT static void
    TranslateView(vtkRenderer* theRenderer,
                  const int    theOldX,
                  const int    theOldY,
                  const int    theNewX,
                  const int    theNewY);

  asiVisu_EXPORT static void
    AdjustTrihedron(vtkRenderer*       theRenderer,
                    vtkAxesActor*      theActor,
                    vtkPropCollection* thePropsToSkip);

  asiVisu_EXPORT static vtkSmartPointer<vtkImageData>
    GetImage(vtkRenderWindow* pRenderWindow);

  asiVisu_EXPORT static void
    WritePNG(const vtkSmartPointer<vtkImageData>& data,
             const char*                          filename);

// Other commons:
public:

  asiVisu_EXPORT static vtkSmartPointer<vtkIntArray>
    InitIntArray(const char* theArrName);

  asiVisu_EXPORT static vtkSmartPointer<vtkDoubleArray>
    InitDoubleArray(const char* theArrName);

  asiVisu_EXPORT static vtkSmartPointer<vtkDoubleArray>
    InitDoubleVectorArray(const char* theArrName);

  asiVisu_EXPORT static vtkSmartPointer<vtkStringArray>
    InitStringArray(const char* theArrName);

  asiVisu_EXPORT static vtkSmartPointer<vtkProperty>
    DefaultBackfaceProp();

  asiVisu_EXPORT static void
    DefaultPickingColor(double& fR,
                        double& fG,
                        double& fB);

  asiVisu_EXPORT static void
    DefaultDetectionColor(double& fR,
                          double& fG,
                          double& fB);

  asiVisu_EXPORT static double
    DefaultPickLineWidth();

  asiVisu_EXPORT static double
    DefaultDetectionLineWidth();

  asiVisu_EXPORT static double
    DefaultHilightPointSize();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitLookupTable();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitLookupTable(const NCollection_DataMap<int, int>& customScalarMap,
                    const int                            lastUnusedScalar);

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitLookupTable(const NCollection_DataMap<int, int>& customScalarMap,
                    const int                            lastUnusedScalar,
                    const double                         ref_r,
                    const double                         ref_g,
                    const double                         ref_b,
                    const double                         eref_r,
                    const double                         eref_g,
                    const double                         eref_b);

  asiVisu_EXPORT static void
    InitShapeMapper(vtkMapper* mapper);

  asiVisu_EXPORT static void
    InitShapeMapper(vtkMapper* mapper, vtkLookupTable* colorTable);

  asiVisu_EXPORT static void
    InitShapeMapper(vtkMapper*                           mapper,
                    const NCollection_DataMap<int, int>& customScalarMap,
                    const int                            lastUnusedScalar);

  asiVisu_EXPORT static void
    InitShapeMapper(vtkMapper*                           mapper,
                    vtkActor*                            actor,
                    const NCollection_DataMap<int, int>& customScalarMap,
                    const int                            lastUnusedScalar);

  asiVisu_EXPORT static void
    InitShapeMapper(vtkMapper*                           mapper,
                    const double                         ref_r,
                    const double                         ref_g,
                    const double                         ref_b,
                    const double                         eref_r,
                    const double                         eref_g,
                    const double                         eref_b,
                    const NCollection_DataMap<int, int>& customScalarMap,
                    const int                            lastUnusedScalar);

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitDomainLookupTable();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitCurvatureCombsLookupTable();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitKnotsIsosLookupTable();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitAxesLookupTable();

  asiVisu_EXPORT static vtkSmartPointer<vtkLookupTable>
    InitCurviAxesLookupTable();

  asiVisu_EXPORT static void
    InitMapper(vtkMapper*      theMapper,
               vtkLookupTable* theLookup,
               const char*     theScalarsArrName);

  asiVisu_EXPORT static void
    InitTextWidget(vtkTextWidget* theTextWidget);

public:

  static void ColorForFeatureAngle(const asiAlgo_FeatureAngleType angle,
                                   double&                        redF,
                                   double&                        greenF,
                                   double&                        blueF)
  {
    switch ( angle )
    {
      case FeatureAngleType_SmoothConcave:
      case FeatureAngleType_Concave:
        redF   = 1.0;
        greenF = 0.0;
        blueF  = 0.0;
        break;
      case FeatureAngleType_SmoothConvex:
      case FeatureAngleType_Convex:
        redF   = 0.0;
        greenF = 1.0;
        blueF  = 0.0;
        break;
      case FeatureAngleType_Smooth:
        redF   = 0.0;
        greenF = 0.0;
        blueF  = 1.0;
        break;
      case FeatureAngleType_NonManifold:
        redF   = 0.0;
        greenF = 1.0;
        blueF  = 1.0;
        break;
      case FeatureAngleType_Undefined:
      default:
        redF   = 1.0;
        greenF = 1.0;
        blueF  = 1.0;
    }
  }

  double static TrimInf(const double val,
                        const double limit = INF_LIMIT,
                        const double begin = 0.0)
  {
    double ret_val = val;
    if ( Precision::IsPositiveInfinite(val) )
      ret_val = limit + begin;
    else if ( Precision::IsNegativeInfinite(val) )
      ret_val = -limit + begin;

    return ret_val;
  }

  double static Trim(const double val,
                     const double limit,
                     const double begin = 0.0)
  {
    double ret_val = val;
    if ( val > limit + begin )
      ret_val = limit + begin;
    else if ( val < -limit + begin )
      ret_val = -limit + begin;

    return ret_val;
  }

  //! Converts string to color.
  //! \param[in] string string to convert.
  //! \return color.
  asiVisu_EXPORT static ActAPI_Color
    StringToColor(const std::string& string);

  //! Converts RGB color to integer.
  //! \param[in] r red component.
  //! \param[in] g green component.
  //! \param[in] b blue component.
  //! \return converted value.
  static int ColorToInt(unsigned int r, unsigned int g, unsigned int b)
  {
    return r << 16 | g << 8 | b;
  }

  //! Converts RGB color to integer.
  //! \param[in] rgb color.
  //! \return converted value.
  static int ColorToInt(unsigned int rgb[3])
  {
    return ColorToInt(rgb[0], rgb[1], rgb[2]);
  }

  //! Converts RGB color to integer.
  //! \param[in] r red component of the color.
  //! \param[in] g green component of the color.
  //! \param[in] b blue component of the color.
  //! \return converted value.
  static int ColorToInt(const double r, const double g, const double b)
  {
    unsigned char red   = (unsigned char) ( floor(r >= 1.0 ? 255 : r * 256.0) );
    unsigned char green = (unsigned char) ( floor(g >= 1.0 ? 255 : g * 256.0) );
    unsigned char blue  = (unsigned char) ( floor(b >= 1.0 ? 255 : b * 256.0) );
    //
    return red << 16 | green << 8 | blue;
  }

  //! Converts color value to an integer representation.
  //! \param[in] color Qt color.
  //! \return converted value
  static int ColorToInt(const ActAPI_Color& color)
  {
    return ColorToInt( color.Red(), color.Green(), color.Blue() );
  }

  //! Converts integer value to a color.
  //! \param[in] icolor integer color code.
  //! \return converted value
  static ActAPI_Color IntToColor(const int icolor)
  {
    unsigned char uRed   = ( icolor >> 16 ) & 0xFF;
    unsigned char uGreen = ( icolor >>  8 ) & 0xFF;
    unsigned char uBlue  =   icolor         & 0xFF;
    return ActAPI_Color(uRed/MAX_COLOR_SCALE, uGreen/MAX_COLOR_SCALE, uBlue/MAX_COLOR_SCALE, Quantity_TOC_RGB);
  }

private:

  static TPrsAllocMap m_allocMap; //!< Presentation factory.

};

#endif
