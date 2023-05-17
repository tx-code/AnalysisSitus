//-----------------------------------------------------------------------------
// Created on: 14 May 2022 (*)
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

// cmdMisc includes
#include <cmdMisc.h>

// OpenCascade includes
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <TopExp_Explorer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <IntTools_EdgeFace.hxx>

#include <asiEngine_Part.h>

#include <Geom2d_Circle.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <GeomAPI.hxx>
#include <gp_Pln.hxx>
#include <gp_Quaternion.hxx>
#include <BRepTools_WireExplorer.hxx>

//-----------------------------------------------------------------------------

#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <asiAlgo_DivideByContinuity.h>

#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_QuickHull2d.h>

#if defined USE_MOBIUS
  #include <mobius/poly_Mesh.h>
  using namespace mobius;
#endif

#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <GeomConvert.hxx>
#include <ShapeFix_Wire.hxx>

#include <IGESControl_Reader.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeAnalysis_Curve.hxx>

//-----------------------------------------------------------------------------

bool LoadIGES(const TCollection_AsciiString& filename,
              TopoDS_Shape&                  result)
{
  IGESControl_Reader reader;
  IFSelect_ReturnStatus outcome = reader.ReadFile( filename.ToCString() );
  //
  if ( outcome != IFSelect_RetDone )
    return false;

  reader.TransferRoots();

  result = reader.OneShape();
  return true;
}

//-----------------------------------------------------------------------------

TopoDS_Shape BooleanGeneralFuse(const TopTools_ListOfShape& objects,
                                const double                fuzz,
                                BOPAlgo_Builder&            API,
                                const bool                  glue)
{
  const bool bRunParallel = false;

  BOPAlgo_PaveFiller DSFiller;
  DSFiller.SetArguments(objects);
  DSFiller.SetRunParallel(bRunParallel);
  DSFiller.SetFuzzyValue(fuzz);
  DSFiller.Perform();
  bool hasErr = DSFiller.HasErrors();
  //
  if ( hasErr )
  {
    return TopoDS_Shape();
  }

  if ( glue )
    API.SetGlue(BOPAlgo_GlueFull);

  API.SetArguments(objects);
  API.SetRunParallel(bRunParallel);
  API.PerformWithFiller(DSFiller);
  hasErr = API.HasErrors();
  //
  if ( hasErr )
  {
    return TopoDS_Shape();
  }

  return API.Shape();
}

#include <asiAlgo_PSO.h>
#include <asiAlgo_GradientDescent.h>

//! Geometric primitive for 2D point.
class Prim_UV
{
public:

  static int num_coordinates()
  {
    return 2;
  }

// Construction & destruction:
public:

  Prim_UV();
  Prim_UV(const double u, const double v);
  Prim_UV(const Prim_UV& UV);
  virtual ~Prim_UV();

public:

  //! Returns U coordinate of the 2D point.
  //! \return U coordinate.
  inline double U() const
  {
    return m_fU;
  }

  //! Sets U coordinate.
  //! \param u [in] value to set.
  inline void SetU(const double u)
  {
    m_fU = u;
  }

  //! Returns V coordinate of the 2D point.
  //! \return V coordinate.
  inline double V() const
  {
    return m_fV;
  }

  //! Sets V coordinate.
  //! \param v [in] value to set.
  inline void SetV(const double v)
  {
    m_fV = v;
  }

  //! Returns coordinate by its 0-based index.
  //! \param idx [in] 0 for U, 1 for V.
  //! \return requested coordinate.
  inline double Coord(const int idx) const
  {
    if ( idx == 0 )
      return this->U();

    if ( idx == 1 )
      return this->V();

    return DBL_MAX;
  }

  //! Updates coordinate having the specified 0-based index with the
  //! passed value.
  //! \param idx [in] 0 for U, 1 for V.
  //! \param val [in] value to set.
  inline double SetCoord(const int idx,
                         const double val)
  {
    double* coord = NULL;

    if ( idx == 0 )
      coord = &m_fU;
    if ( idx == 1 )
      coord = &m_fV;

    if ( coord )
      *coord = val;

    return DBL_MAX;
  }

public:

  double Modulus       ()                  const;
  double SquaredModulus()                  const;
  double Dot           (const Prim_UV& UV) const;

public:

  Prim_UV& operator=  (const Prim_UV& UV);
  Prim_UV  operator*  (const double coeff) const;
  Prim_UV  operator*= (const double coeff);
  Prim_UV  operator/  (const double coeff) const;
  Prim_UV  operator/= (const double coeff);
  Prim_UV  operator+  (const Prim_UV& UV)  const;
  Prim_UV& operator+= (const Prim_UV& UV);
  Prim_UV  Invert()                        const;
  Prim_UV  operator-  (const Prim_UV& UV)  const;
  Prim_UV& operator-= (const Prim_UV& UV);

private:

  double m_fU; //!< U coordinate.
  double m_fV; //!< V coordinate.

};

//! Default constructor. Initializes point coordinates with origin values:
//! (0, 0).
Prim_UV::Prim_UV()
{
  m_fU = m_fV = 0.0;
}

//! Complete constructor.
//! \param u [in] first coordinate.
//! \param v [in] second coordinate.
Prim_UV::Prim_UV(const double u, const double v)
{
  m_fU = u;
  m_fV = v;
}

//! Copy constructor.
//! \param UV [in] point to copy.
Prim_UV::Prim_UV(const Prim_UV& UV)
{
  this->operator=(UV);
}

//! Destructor.
Prim_UV::~Prim_UV()
{
}

//! Returns modulus of the point's radius vector.
//! \return modulus.
double Prim_UV::Modulus() const
{
  return sqrt( this->SquaredModulus() );
}

//! Returns squared modulus of the point's radius vector.
//! \return modulus.
double Prim_UV::SquaredModulus() const
{
  return m_fU*m_fU + m_fV*m_fV;
}

//! Calculates dot product between this and another vector.
//! \param UV [in] another vector.
//! \return dot product.
double Prim_UV::Dot(const Prim_UV& UV) const
{
  return m_fU*UV.m_fU + m_fV*UV.m_fV;
}

//! Assignment operator.
//! \param UV [in] point to copy into this one.
//! \return this one.
Prim_UV& Prim_UV::operator=(const Prim_UV& UV)
{
  m_fU = UV.m_fU;
  m_fV = UV.m_fV;
  return *this;
}

//! Multiplies copy of point by the passed scalar value.
//! \param coeff [in] scalar value to multiply point by.
//! \return resulting point.
Prim_UV Prim_UV::operator*(const double coeff) const
{
  Prim_UV UV_Copy(*this);
  UV_Copy *= coeff;
  return UV_Copy;
}

//! Multiplies this point by the passed scalar value.
//! \param coeff [in] scalar value to multiply point by.
//! \return this point multiplied by the passed scalar.
Prim_UV Prim_UV::operator*=(const double coeff)
{
  this->m_fU *= coeff;
  this->m_fV *= coeff;
  return *this;
}

//! Divides copy of point by the passed scalar value.
//! \param coeff [in] scalar value to divide point by.
//! \return resulting point.
Prim_UV Prim_UV::operator/(const double coeff) const
{
  Prim_UV UV_Copy(*this);
  UV_Copy /= coeff;
  return UV_Copy;
}

//! Divides this point by the passed scalar value.
//! \param coeff [in] scalar value to divide point by.
//! \return this point multiplied by the passed scalar.
Prim_UV Prim_UV::operator/=(const double coeff)
{
  this->m_fU /= coeff;
  this->m_fV /= coeff;
  return *this;
}

//! Adds the passed point to the copy of this one.
//! \param UV [in] point to add.
//! \return result of addition.
Prim_UV Prim_UV::operator+(const Prim_UV& UV) const
{
  Prim_UV UV_Copy(*this);
  UV_Copy += UV;
  return UV_Copy;
}

//! Adds the passed point to this one.
//! \param UV [in] point to add.
//! \return result of addition.
Prim_UV& Prim_UV::operator+=(const Prim_UV& UV)
{
  this->m_fU += UV.m_fU;
  this->m_fV += UV.m_fV;
  return *this;
}

//! Inverts the copy of this point.
//! \return result of inversion.
Prim_UV Prim_UV::Invert() const
{
  Prim_UV UV_Copy(*this);
  UV_Copy.m_fU = -this->m_fU;
  UV_Copy.m_fV = -this->m_fV;
  return UV_Copy;
}

//! Subtracts the passed point from the copy of this one.
//! \param UV [in] point to subtract.
//! \return result of subtraction.
Prim_UV Prim_UV::operator-(const Prim_UV& UV) const
{
  return this->operator+( UV.Invert() );
}

//! Adds the passed point to this one.
//! \param UV [in] point to add.
//! \return result of subtraction.
Prim_UV& Prim_UV::operator-=(const Prim_UV& UV)
{
  return this->operator+=( UV.Invert() );
}

//! Objective function.
class LowestZ : public asiAlgo_Function<Prim_UV>
{
public:

  //! Ctor.
  LowestZ(const Handle(Geom_Surface)& surf) : m_surf(surf) {}

public:

  inline virtual double Value(const Prim_UV& uv)
  {
    gp_Pnt P = m_surf->Value( uv.U(), uv.V() );

    return P.Z();
  }

protected:

  Handle(Geom_Surface) m_surf;

};

//! Objective function with gradient.
class LowestZ_Grad : public asiAlgo_FunctionWithGradient<Prim_UV>
{
public:

  LowestZ_Grad(const Handle(Geom_Surface)& surf) : m_surf(surf)
  {}

public:

  inline virtual double Value(const Prim_UV& uv)
  {
    gp_Pnt P = m_surf->Value( uv.U(), uv.V() );

    return P.Z();
  }

  inline virtual t_coord Gradient(const Prim_UV& uv)
  {
    const double d = 0.01;
    const double z1 = Value(uv);
    const double z2x = Value(uv + Prim_UV(d,0));
    const double z2y = Value(uv + Prim_UV(0,d));
    const Prim_UV grad( (z2x - z1)/d, (z2y - z1)/d );
    return grad;
  }

protected:

  Handle(Geom_Surface) m_surf;

};

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          argc,
              const char**                 argv)
{
  asiEngine_Part partApi( cmdMisc::cf->Model,
                          cmdMisc::cf->ViewerPart->PrsMgr() );

  asiAlgo_Feature fids;
  partApi.GetHighlightedFaces(fids);

  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  const TopoDS_Face& F = aag->GetFace( fids.GetMinimalMapped() );

  Handle(Geom_Surface) surf = BRep_Tool::Surface(F);

  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(F, uMin, uMax, vMin, vMax);

  // Objective function.
  LowestZ func(surf);

  asiAlgo_PSO<Prim_UV>::t_search_params pso_params;
  //
  pso_params.num_particles   = 10;                  // In PSO the number of agents is typically small
  pso_params.num_iterations  = 5000;                 // More than enough in typical situations
  pso_params.area.corner_min = Prim_UV(uMin, vMin); // Min corner of the search area
  pso_params.area.corner_max = Prim_UV(uMax, vMax); // Max corner of the search area
  pso_params.precision       = 1.0e-6;               // Less values are of no practical sense
  pso_params.pFunc           = &func;                // Objective function
  pso_params.m               = 0.729;                // Retain weight
  pso_params.mu_cognition    = 0.4;                  // Pure social behavior
  pso_params.nu_social       = 1.49445;              // Social determinant of an agent
  //
  asiAlgo_PSO<Prim_UV> pso(pso_params);

  int pso_num_iters = 0;
  pso.Perform(pso_num_iters);
  const asiAlgo_PSO<Prim_UV>::t_measuring& pso_sol = pso.GetBestGlobal();

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Iterations done: %1." << pso_num_iters);
  interp->GetPlotter().REDRAW_POINT("sol", surf->Value(pso_sol.p.U(), pso_sol.p.V() ), Color_Red);

  double u_sol, v_sol;
  LowestZ_Grad F_Grad(surf);

  // Gradient descent parameters
  asiAlgo_GradientDescent<Prim_UV>::t_search_params grad_params;
  grad_params.num_iterations = 10000;
  grad_params.pFunc          = &F_Grad;
  grad_params.precision      = 1.0e-9;
  grad_params.start          = pso_sol.p;
  grad_params.is_adaptive    = true;
  grad_params.step           = 1.0e-1; // To be corrected by Armijo rule

  // Run local optimization
  int grad_num_iters = 0;
  asiAlgo_GradientDescent<Prim_UV> Descent(grad_params);
  if ( !Descent.Perform(grad_num_iters) )
  {
    // TODO: cout is a bad practice (some logger to be used instead)
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Gradient descent did not converge.");
    // Still, let's continue for better robustness of the method
  }
  const Prim_UV& grad_sol = Descent.Solution();

  interp->GetPlotter().REDRAW_POINT("grad_sol", surf->Value(grad_sol.U(), grad_sol.V() ), Color_Green);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test", "Test anything.", __FILE__, group, MISC_Test);
}
