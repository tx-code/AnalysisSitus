// Created on: 1993-03-09
// Created by: JCV
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

// 14-Mar-96 : xab  portage hp
// pmn : 28-Jun-96 Distinction entre la continuite en U et V (bug PRO4625)
// pmn : 07-Jan-97 Centralisation des verif rational (PRO6834)
//       et ajout des InvalideCache() dans les SetWeight*(PRO6833)
// RBD : 15-10-98 ; Le cache est maintenant calcule sur [-1,1] (pro15537).
// jct : 19-01-99 ; permutation de urational et vrational dans Rational.
#define No_Standard_OutOfRange

#include <asiAlgo_BSplineSurface.h>

#include <BSplCLib.hxx>
#include <BSplSLib.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <Geom_UndefinedDerivative.hxx>
#include <gp.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_DimensionError.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>
#include <BSplCLib.hxx>
#include <BSplSLib.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Standard_RangeError.hxx>
#include <TColgp_Array1OfXYZ.hxx>

#define  POLES    (poles->Array2())
#define  WEIGHTS  (weights->Array2())
#define  UKNOTS   (uknots->Array1())
#define  VKNOTS   (vknots->Array1())
#define  UFKNOTS  (ufknots->Array1())
#define  VFKNOTS  (vfknots->Array1())
#define  FMULTS   (BSplCLib::NoMults())

IMPLEMENT_STANDARD_RTTIEXT(asiAlgo_BSplineSurface,Geom_BoundedSurface)

//=======================================================================
//function : IsCNu
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsCNu
(const Standard_Integer N) const
{
  Standard_RangeError_Raise_if (N < 0, " ");
  switch (Usmooth) {
  case GeomAbs_CN : return Standard_True;
  case GeomAbs_C0 : return N <= 0;
  case GeomAbs_G1 : return N <= 0;
  case GeomAbs_C1 : return N <= 1;
  case GeomAbs_G2 : return N <= 1;
  case GeomAbs_C2 : return N <= 2;
  case GeomAbs_C3 :
    return N <= 3 ? Standard_True :
      N <= udeg - BSplCLib::MaxKnotMult (umults->Array1(), umults->Lower() + 1, umults->Upper() - 1);
  default:
    return Standard_False;
  }
}

//=======================================================================
//function : IsCNv
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsCNv
(const Standard_Integer N) const
{
  Standard_RangeError_Raise_if (N < 0, " ");

  switch (Vsmooth) {
  case GeomAbs_CN : return Standard_True;
  case GeomAbs_C0 : return N <= 0;
  case GeomAbs_G1 : return N <= 0;
  case GeomAbs_C1 : return N <= 1;
  case GeomAbs_G2 : return N <= 1;
  case GeomAbs_C2 : return N <= 2;
  case GeomAbs_C3 :
    return N <= 3 ? Standard_True :
      N <= vdeg - BSplCLib::MaxKnotMult (vmults->Array1(), vmults->Lower() + 1, vmults->Upper() - 1);
  default:
    return Standard_False;
  }
}

//=======================================================================
//function : D0
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::D0(const Standard_Real U,
  const Standard_Real V,
  gp_Pnt&       P) const 
{
  Standard_Real aNewU = U;
  Standard_Real aNewV = V;
  PeriodicNormalization(aNewU, aNewV);

  BSplSLib::D0(aNewU,aNewV,0,0,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P);
}

//=======================================================================
//function : D1
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::D1(const Standard_Real U,
  const Standard_Real V,
  gp_Pnt&       P,
  gp_Vec&       D1U,
  gp_Vec&       D1V) const
{
  Standard_Real aNewU = U;
  Standard_Real aNewV = V;
  PeriodicNormalization(aNewU, aNewV);

  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, uknots->Array1(), &umults->Array1(), U, uperiodic, uindex, aNewU);
  uindex = BSplCLib::FlatIndex(udeg, uindex, umults->Array1(), uperiodic);

  BSplCLib::LocateParameter(vdeg, vknots->Array1(), &vmults->Array1(), V, vperiodic, vindex, aNewV);
  vindex = BSplCLib::FlatIndex(vdeg, vindex, vmults->Array1(), vperiodic);

  BSplSLib::D1(aNewU,aNewV,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P, D1U, D1V);
}

//=======================================================================
//function : D2
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::D2 (const Standard_Real U, 
  const Standard_Real V,
  gp_Pnt&       P,
  gp_Vec&       D1U,
  gp_Vec&       D1V,
  gp_Vec&       D2U,
  gp_Vec&       D2V,
  gp_Vec&       D2UV) const
{
  Standard_Real aNewU = U;
  Standard_Real aNewV = V;
  PeriodicNormalization(aNewU, aNewV);

  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, uknots->Array1(), &umults->Array1(), U, uperiodic, uindex, aNewU);
  uindex = BSplCLib::FlatIndex(udeg, uindex, umults->Array1(), uperiodic);

  BSplCLib::LocateParameter(vdeg, vknots->Array1(), &vmults->Array1(), V, vperiodic, vindex, aNewV);
  vindex = BSplCLib::FlatIndex(vdeg, vindex, vmults->Array1(), vperiodic);

  BSplSLib::D2(aNewU,aNewV,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P, D1U, D1V, D2U, D2V, D2UV);
}

//=======================================================================
//function : D3
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::D3 (const Standard_Real U, 
  const Standard_Real V,
  gp_Pnt&       P,
  gp_Vec&       D1U,
  gp_Vec&       D1V, 
  gp_Vec&       D2U,
  gp_Vec&       D2V,
  gp_Vec&       D2UV,
  gp_Vec&       D3U,
  gp_Vec&       D3V,
  gp_Vec&       D3UUV,
  gp_Vec&       D3UVV) const
{
  BSplSLib::D3(U,V,0,0,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P,D1U,D1V,D2U,D2V,D2UV,D3U,D3V,D3UUV,D3UVV);
}

//=======================================================================
//function : DN
//purpose  : 
//=======================================================================

gp_Vec asiAlgo_BSplineSurface::DN (const Standard_Real    U,
  const Standard_Real    V,
  const Standard_Integer Nu,
  const Standard_Integer Nv ) const
{
  gp_Vec Vn;
  BSplSLib::DN(U,V,Nu,Nv,0,0,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    Vn);
  return Vn;
}

//=======================================================================
//function : LocalValue
//purpose  : 
//=======================================================================

gp_Pnt asiAlgo_BSplineSurface::LocalValue (const Standard_Real    U,
  const Standard_Real    V, 
  const Standard_Integer FromUK1,
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1, 
  const Standard_Integer ToVK2)  const
{
  gp_Pnt P;
  LocalD0(U,V,FromUK1,ToUK2,FromVK1,ToVK2,P);
  return P;
}

//=======================================================================
//function : LocalD0
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocalD0 (const Standard_Real    U,
  const Standard_Real    V, 
  const Standard_Integer FromUK1,
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1, 
  const Standard_Integer ToVK2,
  gp_Pnt&          P     )  const
{
  Standard_DomainError_Raise_if (FromUK1 == ToUK2 || FromVK1 == ToVK2, 
    "asiAlgo_BSplineSurface::LocalD0");

  Standard_Real u = U, v = V;
  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, UFKNOTS, U, uperiodic,FromUK1,ToUK2,
    uindex,u);
  uindex = BSplCLib::FlatIndex(udeg,uindex,umults->Array1(),uperiodic);

  BSplCLib::LocateParameter(vdeg, VFKNOTS, V, vperiodic,FromVK1,ToVK2,
    vindex,v);
  vindex = BSplCLib::FlatIndex(vdeg,vindex,vmults->Array1(),vperiodic);

  //  BSplSLib::D0(U,V,uindex,vindex,POLES,WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
  BSplSLib::D0(u,v,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P);
}

//=======================================================================
//function : LocalD1
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocalD1 (const Standard_Real    U, 
  const Standard_Real    V,
  const Standard_Integer FromUK1, 
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1, 
  const Standard_Integer ToVK2,
  gp_Pnt&          P,
  gp_Vec&          D1U, 
  gp_Vec&          D1V)     const
{
  Standard_DomainError_Raise_if (FromUK1 == ToUK2 || FromVK1 == ToVK2, 
    "asiAlgo_BSplineSurface::LocalD1");

  Standard_Real u = U, v = V;
  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, UFKNOTS, U, uperiodic,FromUK1,ToUK2,
    uindex,u);
  uindex = BSplCLib::FlatIndex(udeg,uindex,umults->Array1(),uperiodic);

  BSplCLib::LocateParameter(vdeg, VFKNOTS, V, vperiodic,FromVK1,ToVK2,
    vindex,v);
  vindex = BSplCLib::FlatIndex(vdeg,vindex,vmults->Array1(),vperiodic);

  BSplSLib::D1(u,v,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P,D1U,D1V);
}

//=======================================================================
//function : LocalD2
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocalD2 (const Standard_Real    U,
  const Standard_Real    V,
  const Standard_Integer FromUK1,
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1, 
  const Standard_Integer ToVK2,
  gp_Pnt&          P,
  gp_Vec&          D1U,
  gp_Vec&          D1V,
  gp_Vec&          D2U,
  gp_Vec&          D2V,
  gp_Vec&          D2UV) const
{
  Standard_DomainError_Raise_if (FromUK1 == ToUK2 || FromVK1 == ToVK2, 
    "asiAlgo_BSplineSurface::LocalD2");

  Standard_Real u = U, v = V;
  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, UFKNOTS, U, uperiodic,FromUK1,ToUK2,
    uindex,u);
  uindex = BSplCLib::FlatIndex(udeg,uindex,umults->Array1(),uperiodic);

  BSplCLib::LocateParameter(vdeg, VFKNOTS, V, vperiodic,FromVK1,ToVK2,
    vindex,v);
  vindex = BSplCLib::FlatIndex(vdeg,vindex,vmults->Array1(),vperiodic);

  BSplSLib::D2(u,v,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P,D1U,D1V,D2U,D2V,D2UV);
}

//=======================================================================
//function : LocalD3
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocalD3 (const Standard_Real    U, 
  const Standard_Real    V,
  const Standard_Integer FromUK1, 
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1, 
  const Standard_Integer ToVK2,
  gp_Pnt&          P,
  gp_Vec&          D1U,
  gp_Vec&          D1V, 
  gp_Vec&          D2U, 
  gp_Vec&          D2V, 
  gp_Vec&          D2UV, 
  gp_Vec&          D3U,
  gp_Vec&          D3V,
  gp_Vec&          D3UUV,
  gp_Vec&          D3UVV) const
{
  Standard_DomainError_Raise_if (FromUK1 == ToUK2 || FromVK1 == ToVK2, 
    "asiAlgo_BSplineSurface::LocalD3");

  Standard_Real u = U, v = V;
  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, UFKNOTS, U, uperiodic,FromUK1,ToUK2,
    uindex,u);
  uindex = BSplCLib::FlatIndex(udeg,uindex,umults->Array1(),uperiodic);

  BSplCLib::LocateParameter(vdeg, VFKNOTS, V, vperiodic,FromVK1,ToVK2,
    vindex,v);
  vindex = BSplCLib::FlatIndex(vdeg,vindex,vmults->Array1(),vperiodic);

  BSplSLib::D3(u,v,uindex,vindex,POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    P,D1U,D1V,D2U,D2V,D2UV,D3U,D3V,D3UUV,D3UVV);
}

//=======================================================================
//function : LocalDN
//purpose  : 
//=======================================================================

gp_Vec asiAlgo_BSplineSurface::LocalDN  (const Standard_Real    U, 
  const Standard_Real    V,
  const Standard_Integer FromUK1,
  const Standard_Integer ToUK2,
  const Standard_Integer FromVK1,
  const Standard_Integer ToVK2,
  const Standard_Integer Nu,
  const Standard_Integer Nv) const
{
  Standard_DomainError_Raise_if (FromUK1 == ToUK2 || FromVK1 == ToVK2, 
    "asiAlgo_BSplineSurface::LocalDN");

  Standard_Real u = U, v = V;
  Standard_Integer uindex = 0, vindex = 0;

  BSplCLib::LocateParameter(udeg, UFKNOTS, U, uperiodic,FromUK1,ToUK2,
    uindex,u);
  uindex = BSplCLib::FlatIndex(udeg,uindex,umults->Array1(),uperiodic);

  BSplCLib::LocateParameter(vdeg, VFKNOTS, V, vperiodic,FromVK1,ToVK2,
    vindex,v);
  vindex = BSplCLib::FlatIndex(vdeg,vindex,vmults->Array1(),vperiodic);

  gp_Vec Vn;
  BSplSLib::DN(u,v,Nu,Nv,uindex,vindex,
    POLES,&WEIGHTS,UFKNOTS,VFKNOTS,FMULTS,FMULTS,
    udeg,vdeg,urational,vrational,uperiodic,vperiodic,
    Vn);
  return Vn;
}

//=======================================================================
//function : Pole
//purpose  : 
//=======================================================================

const gp_Pnt& asiAlgo_BSplineSurface::Pole(const Standard_Integer UIndex,
  const Standard_Integer VIndex) const
{
  Standard_OutOfRange_Raise_if
  (UIndex < 1 || UIndex > poles->ColLength() ||
    VIndex < 1 || VIndex > poles->RowLength(), " ");
  return poles->Value (UIndex, VIndex);
}

//=======================================================================
//function : Poles
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::Poles (TColgp_Array2OfPnt& P) const
{
  Standard_DimensionError_Raise_if
  (P.ColLength() != poles->ColLength() ||
    P.RowLength() != poles->RowLength(), " ");
  P = poles->Array2();
}

const TColgp_Array2OfPnt& asiAlgo_BSplineSurface::Poles() const
{
  return poles->Array2();
}

//=======================================================================
//function : UIso
//purpose  : 
//=======================================================================

Handle(Geom_Curve) asiAlgo_BSplineSurface::UIso (const Standard_Real U) const
{
  TColgp_Array1OfPnt   cpoles(1,poles->RowLength());
  TColStd_Array1OfReal cweights(1,poles->RowLength());

  Handle(Geom_BSplineCurve) C;

  if ( urational || vrational) {
    BSplSLib::Iso(U,Standard_True,POLES,&WEIGHTS,UFKNOTS,FMULTS,udeg,uperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,cweights,
      vknots->Array1(),
      vmults->Array1(),
      vdeg,vperiodic);
  }
  else {
    BSplSLib::Iso(U,Standard_True,POLES,
      BSplSLib::NoWeights(),
      UFKNOTS,FMULTS,udeg,uperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,
      vknots->Array1(),
      vmults->Array1(),
      vdeg,vperiodic);
  }

  return C;
}

//=======================================================================
//function : UIso
//purpose  : If CheckRational=False, no try to make it non-rational
//=======================================================================

Handle(Geom_Curve) asiAlgo_BSplineSurface::UIso (const Standard_Real U,
  const Standard_Boolean CheckRational) const
{
  TColgp_Array1OfPnt   cpoles(1,poles->RowLength());
  TColStd_Array1OfReal cweights(1,poles->RowLength());

  Handle(Geom_BSplineCurve) C;

  if ( urational || vrational) {
    BSplSLib::Iso(U,Standard_True,POLES,&WEIGHTS,UFKNOTS,FMULTS,udeg,uperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,cweights,
      vknots->Array1(),
      vmults->Array1(),
      vdeg,vperiodic,
      CheckRational);
  }
  else {
    BSplSLib::Iso(U,Standard_True,POLES,
      BSplSLib::NoWeights(),
      UFKNOTS,FMULTS,udeg,uperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,
      vknots->Array1(),
      vmults->Array1(),
      vdeg,vperiodic);
  }

  return C;
}

//=======================================================================
//function : UKnot
//purpose  : 
//=======================================================================

Standard_Real asiAlgo_BSplineSurface::UKnot(const Standard_Integer UIndex) const
{
  Standard_OutOfRange_Raise_if (UIndex < 1 || UIndex > uknots->Length(), " ");
  return uknots->Value (UIndex);
}

//=======================================================================
//function : VKnot
//purpose  : 
//=======================================================================

Standard_Real asiAlgo_BSplineSurface::VKnot(const Standard_Integer VIndex) const
{
  Standard_OutOfRange_Raise_if (VIndex < 1 || VIndex > vknots->Length(), " ");
  return vknots->Value (VIndex);
}

//=======================================================================
//function : UKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UKnots (TColStd_Array1OfReal& Ku) const
{
  Standard_DimensionError_Raise_if (Ku.Length() != uknots->Length(), " ");
  Ku = uknots->Array1();
}

const TColStd_Array1OfReal& asiAlgo_BSplineSurface::UKnots() const
{
  return uknots->Array1();
}

//=======================================================================
//function : VKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::VKnots (TColStd_Array1OfReal& Kv) const
{
  Standard_DimensionError_Raise_if (Kv.Length() != vknots->Length(), " ");
  Kv = vknots->Array1();
}

const TColStd_Array1OfReal& asiAlgo_BSplineSurface::VKnots() const
{
  return vknots->Array1();
}

//=======================================================================
//function : UKnotSequence
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UKnotSequence (TColStd_Array1OfReal& Ku) const
{
  Standard_DimensionError_Raise_if (Ku.Length() != ufknots->Length(), " ");
  Ku = ufknots->Array1();
}

const TColStd_Array1OfReal& asiAlgo_BSplineSurface::UKnotSequence() const
{
  return ufknots->Array1();
}

//=======================================================================
//function : VKnotSequence
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::VKnotSequence (TColStd_Array1OfReal& Kv) const
{
  Standard_DimensionError_Raise_if (Kv.Length() != vfknots->Length(), " ");
  Kv = vfknots->Array1();
}

const TColStd_Array1OfReal& asiAlgo_BSplineSurface::VKnotSequence() const
{
  return vfknots->Array1();
}

//=======================================================================
//function : UMultiplicity
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::UMultiplicity 
(const Standard_Integer UIndex) const
{
  Standard_OutOfRange_Raise_if (UIndex < 1 || UIndex > umults->Length()," ");
  return umults->Value (UIndex);
}

//=======================================================================
//function : UMultiplicities
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UMultiplicities (TColStd_Array1OfInteger& Mu) const
{
  Standard_DimensionError_Raise_if (Mu.Length() != umults->Length(), " ");
  Mu = umults->Array1();
}

const TColStd_Array1OfInteger& asiAlgo_BSplineSurface::UMultiplicities() const
{
  return umults->Array1();
}

//=======================================================================
//function : VIso
//purpose  : 
//=======================================================================

Handle(Geom_Curve) asiAlgo_BSplineSurface::VIso (const Standard_Real V) const
{
  TColgp_Array1OfPnt   cpoles(1,poles->ColLength());
  TColStd_Array1OfReal cweights(1,poles->ColLength());

  Handle(Geom_BSplineCurve) C;

  if ( urational || vrational) {
    BSplSLib::Iso(V,Standard_False,POLES,
      &WEIGHTS,
      VFKNOTS,FMULTS,vdeg,vperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,cweights,
      uknots->Array1(),
      umults->Array1(),
      udeg,uperiodic);
  }
  else {
    BSplSLib::Iso(V,Standard_False,POLES,
      BSplSLib::NoWeights(),
      VFKNOTS,FMULTS,vdeg,vperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,
      uknots->Array1(),
      umults->Array1(),
      udeg,uperiodic);
  }

  return C;
}

//=======================================================================
//function : VIso
//purpose  : If CheckRational=False, no try to make it non-rational
//=======================================================================

Handle(Geom_Curve) asiAlgo_BSplineSurface::VIso (const Standard_Real V,
  const Standard_Boolean CheckRational) const
{
  TColgp_Array1OfPnt   cpoles(1,poles->ColLength());
  TColStd_Array1OfReal cweights(1,poles->ColLength());

  Handle(Geom_BSplineCurve) C;

  if ( urational || vrational) {
    BSplSLib::Iso(V,Standard_False,POLES,
      &WEIGHTS,
      VFKNOTS,FMULTS,vdeg,vperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,cweights,
      uknots->Array1(),
      umults->Array1(),
      udeg,uperiodic,
      CheckRational);
  }
  else {
    BSplSLib::Iso(V,Standard_False,POLES,
      BSplSLib::NoWeights(),
      VFKNOTS,FMULTS,vdeg,vperiodic,
      cpoles,&cweights);
    C = new Geom_BSplineCurve(cpoles,
      uknots->Array1(),
      umults->Array1(),
      udeg,uperiodic);
  }

  return C;
}

//=======================================================================
//function : VMultiplicity
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::VMultiplicity 
(const Standard_Integer VIndex) const
{
  Standard_OutOfRange_Raise_if (VIndex < 1 || VIndex > vmults->Length()," ");
  return vmults->Value (VIndex);
}

//=======================================================================
//function : VMultiplicities
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::VMultiplicities (TColStd_Array1OfInteger& Mv) const
{
  Standard_DimensionError_Raise_if (Mv.Length() != vmults->Length(), " ");
  Mv = vmults->Array1();
}

const TColStd_Array1OfInteger& asiAlgo_BSplineSurface::VMultiplicities() const
{
  return vmults->Array1();
}

//=======================================================================
//function : Weight
//purpose  : 
//=======================================================================

Standard_Real asiAlgo_BSplineSurface::Weight 
(const Standard_Integer UIndex,
  const Standard_Integer VIndex ) const
{
  Standard_OutOfRange_Raise_if
  (UIndex < 1 || UIndex > weights->ColLength() ||
    VIndex < 1 || VIndex > weights->RowLength(), " ");
  return weights->Value (UIndex, VIndex);
}

//=======================================================================
//function : Weights
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::Weights (TColStd_Array2OfReal& W) const
{
  Standard_DimensionError_Raise_if
  (W.ColLength() != weights->ColLength() ||
    W.RowLength() != weights->RowLength(), " ");
  W = weights->Array2();
}

const TColStd_Array2OfReal* asiAlgo_BSplineSurface::Weights() const
{
  if (urational || vrational)
    return &weights->Array2();
  return BSplSLib::NoWeights();
}

//=======================================================================
//function : Transform
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::Transform (const gp_Trsf& T)
{
  TColgp_Array2OfPnt & VPoles = poles->ChangeArray2();
  for (Standard_Integer j = VPoles.LowerCol(); j <= VPoles.UpperCol(); j++) {
    for (Standard_Integer i = VPoles.LowerRow(); i <= VPoles.UpperRow(); i++) {
      VPoles (i, j).Transform (T);
    }
  }
}

//=======================================================================
//function : SetUPeriodic
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUPeriodic ()
{
  Standard_Integer i,j;

  Standard_Integer first = FirstUKnotIndex();
  Standard_Integer last  = LastUKnotIndex();

  Handle(TColStd_HArray1OfReal) tk = uknots;
  TColStd_Array1OfReal cknots((uknots->Array1())(first),first,last);
  uknots = new TColStd_HArray1OfReal(1,cknots.Length());
  uknots->ChangeArray1() = cknots;

  Handle(TColStd_HArray1OfInteger) tm = umults;
  TColStd_Array1OfInteger cmults((umults->Array1())(first),first,last);
  //  Modified by Sergey KHROMOV - Mon Feb 10 10:59:00 2003 Begin
  //   cmults(first) = cmults(last) = Max( cmults(first), cmults(last));
  cmults(first) = cmults(last) = Min(udeg, Max( cmults(first), cmults(last)));
  //  Modified by Sergey KHROMOV - Mon Feb 10 10:59:00 2003 End
  umults = new TColStd_HArray1OfInteger(1,cmults.Length());
  umults->ChangeArray1() = cmults;

  // compute new number of poles;
  Standard_Integer nbp = BSplCLib::NbPoles(udeg,Standard_True,cmults);

  TColgp_Array2OfPnt cpoles(1,nbp,poles->LowerCol(),poles->UpperCol());
  for (i = 1; i <= nbp; i++) {
    for (j = poles->LowerCol(); j <= poles->UpperCol(); j++) {
      cpoles(i,j) = poles->Value(i,j);
    }
  }
  poles = 
    new TColgp_HArray2OfPnt(1,nbp,cpoles.LowerCol(),cpoles.UpperCol());
  poles->ChangeArray2() = cpoles;

  TColStd_Array2OfReal 
    cweights(1,nbp,weights->LowerCol(),weights->UpperCol());
  if (urational || vrational) {
    for (i = 1; i <= nbp; i++) {
      for (j = weights->LowerCol(); j <= weights->UpperCol(); j++) {
        cweights(i,j) = weights->Value(i,j);
      }
    }
  }
  else { 
    for (i = 1; i <= nbp; i++) {
      for (j = weights->LowerCol(); j <= weights->UpperCol(); j++) {
        cweights(i,j) = 1;
      }
    }
  }
  weights = new 
    TColStd_HArray2OfReal(1,nbp,cweights.LowerCol(),cweights.UpperCol());
  weights->ChangeArray2() = cweights;


  uperiodic = Standard_True;

  maxderivinvok = 0;
  UpdateUKnots();
}

//=======================================================================
//function : SetVPeriodic
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVPeriodic ()
{
  Standard_Integer i,j;

  Standard_Integer first = FirstVKnotIndex();
  Standard_Integer last  = LastVKnotIndex();

  Handle(TColStd_HArray1OfReal) tk = vknots;
  TColStd_Array1OfReal cknots((vknots->Array1())(first),first,last);
  vknots = new TColStd_HArray1OfReal(1,cknots.Length());
  vknots->ChangeArray1() = cknots;


  Handle(TColStd_HArray1OfInteger) tm = vmults;
  TColStd_Array1OfInteger cmults((vmults->Array1())(first),first,last);
  //  Modified by Sergey KHROMOV - Mon Feb 10 11:00:33 2003 Begin
  //   cmults(first) = cmults(last) = Max( cmults(first), cmults(last));
  cmults(first) = cmults(last) = Min(vdeg, Max( cmults(first), cmults(last)));
  //  Modified by Sergey KHROMOV - Mon Feb 10 11:00:34 2003 End
  vmults = new TColStd_HArray1OfInteger(1,cmults.Length());
  vmults->ChangeArray1() = cmults;

  // compute new number of poles;
  Standard_Integer nbp = BSplCLib::NbPoles(vdeg,Standard_True,cmults);

  TColgp_Array2OfPnt cpoles(poles->LowerRow(),poles->UpperRow(),1,nbp);
  for (i = poles->LowerRow(); i <= poles->UpperRow(); i++) {
    for (j = 1; j <= nbp; j++) {
      cpoles(i,j) = poles->Value(i,j);
    }
  }
  poles =
    new TColgp_HArray2OfPnt(cpoles.LowerRow(),cpoles.UpperRow(),1,nbp);
  poles->ChangeArray2() = cpoles;

  if (urational || vrational) {
    TColStd_Array2OfReal 
      cweights(weights->LowerRow(),weights->UpperRow(),1,nbp);
    for (i = weights->LowerRow(); i <= weights->UpperRow(); i++) {
      for (j = 1; j <= nbp; j++) {
        cweights(i,j) = weights->Value(i,j);
      }
    }
    weights = new 
      TColStd_HArray2OfReal(cweights.LowerRow(),cweights.UpperRow(),1,nbp);
    weights->ChangeArray2() = cweights;
  }

  vperiodic = Standard_True;

  maxderivinvok = 0;
  UpdateVKnots();
}

//=======================================================================
//function : SetUOrigin
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUOrigin(const Standard_Integer Index)
{
  if (!uperiodic)
    throw Standard_NoSuchObject("asiAlgo_BSplineSurface::SetUOrigin: surface is not U periodic");

  Standard_Integer i,j,k;
  Standard_Integer first = FirstUKnotIndex();
  Standard_Integer last  = LastUKnotIndex();

  if ((Index < first) || (Index > last))
    throw Standard_DomainError("Geom_BSplineCurve::SetUOrigin: Index out of range");

  Standard_Integer nbknots = uknots->Length();
  Standard_Integer nbpoles = poles->ColLength();

  Handle(TColStd_HArray1OfReal) nknots = 
    new TColStd_HArray1OfReal(1,nbknots);
  TColStd_Array1OfReal& newknots = nknots->ChangeArray1();

  Handle(TColStd_HArray1OfInteger) nmults =
    new TColStd_HArray1OfInteger(1,nbknots);
  TColStd_Array1OfInteger& newmults = nmults->ChangeArray1();

  // set the knots and mults
  Standard_Real period = uknots->Value(last) - uknots->Value(first);
  k = 1;
  for ( i = Index; i <= last ; i++) {
    newknots(k) = uknots->Value(i);
    newmults(k) = umults->Value(i);
    k++;
  }
  for ( i = first+1; i <= Index; i++) {
    newknots(k) = uknots->Value(i) + period;
    newmults(k) = umults->Value(i);
    k++;
  }

  Standard_Integer index = 1;
  for (i = first+1; i <= Index; i++) 
    index += umults->Value(i);

  // set the poles and weights
  Standard_Integer nbvp = poles->RowLength();
  Handle(TColgp_HArray2OfPnt) npoles =
    new TColgp_HArray2OfPnt(1,nbpoles,1,nbvp);
  Handle(TColStd_HArray2OfReal) nweights =
    new TColStd_HArray2OfReal(1,nbpoles,1,nbvp);
  TColgp_Array2OfPnt   & newpoles   = npoles->ChangeArray2();
  TColStd_Array2OfReal & newweights = nweights->ChangeArray2();
  first = poles->LowerRow();
  last  = poles->UpperRow();
  if ( urational || vrational) {
    k = 1;
    for ( i = index; i <= last; i++) {
      for ( j = 1; j <= nbvp; j++) {
        newpoles(k,j)   = poles->Value(i,j);
        newweights(k,j) = weights->Value(i,j);
      }
      k++;
    }
    for ( i = first; i < index; i++) {
      for ( j = 1; j <= nbvp; j++) {
        newpoles(k,j)   = poles->Value(i,j);
        newweights(k,j) = weights->Value(i,j);
      }
      k++;
    }
  }
  else {
    k = 1;
    for ( i = index; i <= last; i++) {
      for ( j = 1; j <= nbvp; j++) {
        newpoles(k,j) = poles->Value(i,j);
      }
      k++;
    }
    for ( i = first; i < index; i++) {
      for ( j = 1; j <= nbvp; j++) {
        newpoles(k,j) = poles->Value(i,j);
      }
      k++;
    }
  }

  poles  = npoles;
  uknots = nknots;
  umults = nmults;
  if (urational || vrational) 
    weights = nweights;
  UpdateUKnots();

}

//=======================================================================
//function : SetVOrigin
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVOrigin(const Standard_Integer Index)
{
  if (!vperiodic)
    throw Standard_NoSuchObject("asiAlgo_BSplineSurface::SetVOrigin: surface is not V periodic");

  Standard_Integer i,j,k;
  Standard_Integer first = FirstVKnotIndex();
  Standard_Integer last  = LastVKnotIndex();

  if ((Index < first) || (Index > last))
    throw Standard_DomainError("Geom_BSplineCurve::SetVOrigin: Index out of range");

  Standard_Integer nbknots = vknots->Length();
  Standard_Integer nbpoles = poles->RowLength();

  Handle(TColStd_HArray1OfReal) nknots = 
    new TColStd_HArray1OfReal(1,nbknots);
  TColStd_Array1OfReal& newknots = nknots->ChangeArray1();

  Handle(TColStd_HArray1OfInteger) nmults =
    new TColStd_HArray1OfInteger(1,nbknots);
  TColStd_Array1OfInteger& newmults = nmults->ChangeArray1();

  // set the knots and mults
  Standard_Real period = vknots->Value(last) - vknots->Value(first);
  k = 1;
  for ( i = Index; i <= last ; i++) {
    newknots(k) = vknots->Value(i);
    newmults(k) = vmults->Value(i);
    k++;
  }
  for ( i = first+1; i <= Index; i++) {
    newknots(k) = vknots->Value(i) + period;
    newmults(k) = vmults->Value(i);
    k++;
  }

  Standard_Integer index = 1;
  for (i = first+1; i <= Index; i++) 
    index += vmults->Value(i);

  // set the poles and weights
  Standard_Integer nbup = poles->ColLength();
  Handle(TColgp_HArray2OfPnt) npoles =
    new TColgp_HArray2OfPnt(1,nbup,1,nbpoles);
  Handle(TColStd_HArray2OfReal) nweights =
    new TColStd_HArray2OfReal(1,nbup,1,nbpoles);
  TColgp_Array2OfPnt   & newpoles   = npoles->ChangeArray2();
  TColStd_Array2OfReal & newweights = nweights->ChangeArray2();
  first = poles->LowerCol();
  last  = poles->UpperCol();
  if ( urational || vrational) {
    k = 1;
    for ( j = index; j <= last; j++) {
      for ( i = 1; i <= nbup; i++) {
        newpoles(i,k)   = poles->Value(i,j);
        newweights(i,k) = weights->Value(i,j);
      }
      k++;
    }
    for ( j = first; j < index; j++) {
      for ( i = 1; i <= nbup; i++) {
        newpoles(i,k)   = poles->Value(i,j);
        newweights(i,k) = weights->Value(i,j);
      }
      k++;
    }
  }
  else {
    k = 1;
    for ( j = index; j <= last; j++) {
      for ( i = 1; i <= nbup; i++) {
        newpoles(i,k)   = poles->Value(i,j);
      }
      k++;
    }
    for ( j = first; j < index; j++) {
      for ( i = 1; i <= nbup; i++) {
        newpoles(i,k)   = poles->Value(i,j);
      }
      k++;
    }
  }

  poles  = npoles;
  vknots = nknots;
  vmults = nmults;
  if (urational || vrational) 
    weights = nweights;
  UpdateVKnots();

}

//=======================================================================
//function : SetUNotPeriodic
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUNotPeriodic () 
{ 
  if ( uperiodic) {
    Standard_Integer NbKnots, NbPoles;
    BSplCLib::PrepareUnperiodize( udeg, umults->Array1(), NbKnots, NbPoles);

    Handle(TColgp_HArray2OfPnt) npoles = 
      new TColgp_HArray2OfPnt(1,NbPoles, 1, poles->RowLength());

    Handle(TColStd_HArray1OfReal) nknots 
      = new TColStd_HArray1OfReal(1,NbKnots);

    Handle(TColStd_HArray1OfInteger) nmults
      = new TColStd_HArray1OfInteger(1,NbKnots);

    Handle(TColStd_HArray2OfReal) nweights = new TColStd_HArray2OfReal(1,NbPoles, 1, poles->RowLength(), 0);

    if ( urational || vrational) {

      BSplSLib::Unperiodize(Standard_True         , udeg, 
        umults->Array1()      , uknots->Array1(),
        poles->Array2()       , &weights->Array2(),
        nmults->ChangeArray1(), nknots->ChangeArray1(),
        npoles->ChangeArray2(),
        &nweights->ChangeArray2());
    }
    else {

      BSplSLib::Unperiodize(Standard_True         , udeg, 
        umults->Array1()      , uknots->Array1(),
        poles->Array2()       , BSplSLib::NoWeights(),
        nmults->ChangeArray1(), nknots->ChangeArray1(),
        npoles->ChangeArray2(),
        BSplSLib::NoWeights());
    }
    poles     = npoles;
    weights   = nweights;
    umults    = nmults;
    uknots    = nknots;
    uperiodic = Standard_False;

    maxderivinvok = 0;
    UpdateUKnots();

  }
}

//=======================================================================
//function : SetVNotPeriodic
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVNotPeriodic ()
{
  if ( vperiodic) {
    Standard_Integer NbKnots, NbPoles;
    BSplCLib::PrepareUnperiodize( vdeg, vmults->Array1(), NbKnots, NbPoles);

    Handle(TColgp_HArray2OfPnt) npoles = 
      new TColgp_HArray2OfPnt(1, poles->ColLength(), 1, NbPoles);

    Handle(TColStd_HArray1OfReal) nknots 
      = new TColStd_HArray1OfReal(1,NbKnots);

    Handle(TColStd_HArray1OfInteger) nmults
      = new TColStd_HArray1OfInteger(1,NbKnots) ;

    Handle(TColStd_HArray2OfReal) nweights = new TColStd_HArray2OfReal(1, poles->ColLength(), 1, NbPoles, 0);

    if ( urational || vrational) {

      BSplSLib::Unperiodize(Standard_False        , vdeg, 
        vmults->Array1()      , vknots->Array1(),
        poles->Array2()       , &weights->Array2(),
        nmults->ChangeArray1(), nknots->ChangeArray1(),
        npoles->ChangeArray2(),
        &nweights->ChangeArray2());
    }
    else {

      BSplSLib::Unperiodize(Standard_False        , vdeg, 
        vmults->Array1()      , vknots->Array1(),
        poles->Array2()       , BSplSLib::NoWeights(),
        nmults->ChangeArray1(), nknots->ChangeArray1(),
        npoles->ChangeArray2(),
        BSplSLib::NoWeights());
    }
    poles     = npoles;
    weights   = nweights;
    vmults    = nmults;
    vknots    = nknots;
    vperiodic = Standard_False;

    maxderivinvok = 0;
    UpdateVKnots();

  }
}

//=======================================================================
//function : IsUClosed
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsUClosed () const
{   
  if (uperiodic)
    return Standard_True;

  Standard_Real aU1, aU2, aV1, aV2;
  Bounds( aU1, aU2, aV1, aV2 );
  Handle(Geom_Curve) aCUF = UIso( aU1 );
  Handle(Geom_Curve) aCUL = UIso( aU2 );
  if(aCUF.IsNull() || aCUL.IsNull())
    return Standard_False;
  Handle(Geom_BSplineCurve) aBsF = Handle(Geom_BSplineCurve)::DownCast(aCUF);
  Handle(Geom_BSplineCurve) aBsL = Handle(Geom_BSplineCurve)::DownCast(aCUL);
  return (!aBsF.IsNull() && !aBsL.IsNull() && aBsF->IsEqual( aBsL, Precision::Confusion()) ); 
}

//=======================================================================
//function : IsVClosed
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsVClosed () const
{
  if (vperiodic)
    return Standard_True;

  Standard_Real aU1, aU2, aV1, aV2;
  Bounds( aU1, aU2, aV1, aV2 );
  Handle(Geom_Curve) aCVF = VIso( aV1 );
  Handle(Geom_Curve) aCVL = VIso( aV2 );
  if(aCVF.IsNull() || aCVL.IsNull())
    return Standard_False;
  Handle(Geom_BSplineCurve) aBsF = Handle(Geom_BSplineCurve)::DownCast(aCVF);
  Handle(Geom_BSplineCurve) aBsL = Handle(Geom_BSplineCurve)::DownCast(aCVL);
  return (!aBsF.IsNull() && !aBsL.IsNull() && aBsF->IsEqual(aBsL, Precision::Confusion())); 
}

//=======================================================================
//function : IsUPeriodic
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsUPeriodic () const 
{
  return uperiodic; 
}

//=======================================================================
//function : IsVPeriodic
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsVPeriodic () const 
{ 
  return vperiodic; 
}

//=======================================================================
//function : FirstUKnotIndex
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::FirstUKnotIndex () const 
{ 
  if (uperiodic) return 1;
  else return BSplCLib::FirstUKnotIndex(udeg,umults->Array1()); 
}

//=======================================================================
//function : FirstVKnotIndex
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::FirstVKnotIndex () const 
{ 
  if (vperiodic) return 1;
  else return BSplCLib::FirstUKnotIndex(vdeg,vmults->Array1()); 
}

//=======================================================================
//function : LastUKnotIndex
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::LastUKnotIndex() const 
{
  if (uperiodic) return uknots->Length();
  else return BSplCLib::LastUKnotIndex(udeg,umults->Array1()); 
}

//=======================================================================
//function : LastVKnotIndex
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::LastVKnotIndex() const 
{
  if (vperiodic) return vknots->Length();
  else return BSplCLib::LastUKnotIndex(vdeg,vmults->Array1()); 
}

//=======================================================================
//function : LocateU
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocateU
(const Standard_Real     U, 
  const Standard_Real     ParametricTolerance, 
  Standard_Integer& I1,
  Standard_Integer& I2,
  const Standard_Boolean  WithKnotRepetition ) const
{
  Standard_Real NewU =U, vbid = vknots->Value(1);
  Handle(TColStd_HArray1OfReal) TheKnots;
  if (WithKnotRepetition) TheKnots = ufknots;
  else                    TheKnots = uknots;

  PeriodicNormalization(NewU, vbid); //Attention a la periode

  const TColStd_Array1OfReal & Knots = TheKnots->Array1();
  Standard_Real UFirst = Knots (1);
  Standard_Real ULast  = Knots (Knots.Length());
  Standard_Real PParametricTolerance = Abs(ParametricTolerance);
  if (Abs (NewU - UFirst) <= PParametricTolerance) { 
    I1 = I2 = 1; 
  }
  else if (Abs (NewU - ULast) <= PParametricTolerance) { 
    I1 = I2 = Knots.Length();
  }
  else if (NewU < UFirst) {
    I2 = 1;
    I1 = 0;
  }
  else if (NewU > ULast) {
    I1 = Knots.Length();
    I2 = I1 + 1;
  }
  else {
    I1 = 1;
    BSplCLib::Hunt (Knots, NewU, I1);
    I1 = Max (Min (I1, Knots.Upper()), Knots.Lower());
    while (I1 + 1 <= Knots.Upper()
      && Abs (Knots (I1 + 1) - NewU) <= PParametricTolerance)
    {
      I1++;
    }
    if ( Abs( Knots(I1) - NewU) <= PParametricTolerance) {
      I2 = I1;
    }
    else {
      I2 = I1 + 1;
    }
  }
}

//=======================================================================
//function : LocateV
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::LocateV
(const Standard_Real     V, 
  const Standard_Real     ParametricTolerance, 
  Standard_Integer& I1,
  Standard_Integer& I2,
  const Standard_Boolean  WithKnotRepetition ) const
{
  Standard_Real NewV =V, ubid = uknots->Value(1);  
  Handle(TColStd_HArray1OfReal) TheKnots;
  if (WithKnotRepetition) TheKnots = vfknots;
  else                    TheKnots = vknots;

  PeriodicNormalization(ubid, NewV); //Attention a la periode

  const TColStd_Array1OfReal & Knots = TheKnots->Array1();
  Standard_Real VFirst = Knots (1);
  Standard_Real VLast  = Knots (Knots.Length());
  Standard_Real PParametricTolerance = Abs(ParametricTolerance);
  if (Abs (NewV - VFirst) <= PParametricTolerance) { I1 = I2 = 1; }
  else if (Abs (NewV - VLast) <= PParametricTolerance) { 
    I1 = I2 = Knots.Length();
  }
  else if (NewV < VFirst - PParametricTolerance) {
    I2 = 1;
    I1 = 0;
  }
  else if (NewV > VLast + PParametricTolerance) {
    I1 = Knots.Length();
    I2 = I1 + 1;
  }
  else {
    I1 = 1;
    BSplCLib::Hunt (Knots, NewV, I1);
    I1 = Max (Min (I1, Knots.Upper()), Knots.Lower());
    while (I1 + 1 <= Knots.Upper()
      && Abs (Knots (I1 + 1) - NewV) <= PParametricTolerance)
    {
      I1++;
    }
    if ( Abs( Knots(I1) - NewV) <= PParametricTolerance) {
      I2 = I1;
    }
    else {
      I2 = I1 + 1;
    }
  }
}

//=======================================================================
//function : UReverse
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UReverse ()
{
  BSplCLib::Reverse(umults->ChangeArray1());
  BSplCLib::Reverse(uknots->ChangeArray1());
  Standard_Integer last;
  if (uperiodic)
    last = ufknots->Upper() - udeg -1;
  else
    last = poles->UpperRow();
  BSplSLib::Reverse(poles->ChangeArray2(),last,Standard_True);
  if (urational || vrational)
    BSplSLib::Reverse(weights->ChangeArray2(),last,Standard_True);
  UpdateUKnots();
}

//=======================================================================
//function : UReversedParameter
//purpose  : 
//=======================================================================

Standard_Real asiAlgo_BSplineSurface::UReversedParameter
( const Standard_Real U) const 
{
  return ( uknots->Value( 1) + uknots->Value( uknots->Length()) - U);
}

//=======================================================================
//function : VReverse
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::VReverse ()
{
  BSplCLib::Reverse(vmults->ChangeArray1());
  BSplCLib::Reverse(vknots->ChangeArray1());
  Standard_Integer last;
  if (vperiodic)
    last = vfknots->Upper() - vdeg -1;
  else
    last = poles->UpperCol();
  BSplSLib::Reverse(poles->ChangeArray2(),last,Standard_False);
  if (urational || vrational)
    BSplSLib::Reverse(weights->ChangeArray2(),last,Standard_False);
  UpdateVKnots();
}

//=======================================================================
//function : VReversedParameter
//purpose  : 
//=======================================================================

Standard_Real asiAlgo_BSplineSurface::VReversedParameter
( const Standard_Real V) const 
{
  return ( vknots->Value( 1) + vknots->Value( vknots->Length()) - V);
}

//=======================================================================
//function : SetPoleCol
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPoleCol (const Standard_Integer      VIndex,
  const TColgp_Array1OfPnt&   CPoles)
{
  if (VIndex < 1 || VIndex > poles->RowLength())
  {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetPoleCol: VIndex out of range");
  }
  if (CPoles.Lower() < 1 || CPoles.Lower() > poles->ColLength() || 
    CPoles.Upper() < 1 || CPoles.Upper() > poles->ColLength())
  {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetPoleCol: invalid array dimension");
  }

  TColgp_Array2OfPnt & Poles = poles->ChangeArray2();

  for (Standard_Integer I = CPoles.Lower(); I <= CPoles.Upper(); I++) {
    Poles (I+Poles.LowerRow()-1, VIndex+Poles.LowerCol()-1) = CPoles(I);
  }
}

//=======================================================================
//function : SetPoleCol
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPoleCol (const Standard_Integer      VIndex,
  const TColgp_Array1OfPnt&   CPoles,
  const TColStd_Array1OfReal& CPoleWeights)
{
  SetPoleCol  (VIndex, CPoles);
  SetWeightCol(VIndex, CPoleWeights); 
}

//=======================================================================
//function : SetPoleRow
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPoleRow (const Standard_Integer    UIndex,
  const TColgp_Array1OfPnt& CPoles)
{
  if (UIndex < 1 || UIndex > poles->ColLength())
  {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetPoleRow: UIndex out of range");
  }
  if (CPoles.Lower() < 1 || CPoles.Lower() > poles->RowLength() || 
    CPoles.Upper() < 1 || CPoles.Upper() > poles->RowLength())
  {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetPoleRow: invalid array dimension");
  }

  TColgp_Array2OfPnt & Poles = poles->ChangeArray2();

  for (Standard_Integer I = CPoles.Lower(); I <= CPoles.Upper(); I++) {
    Poles (UIndex+Poles.LowerRow()-1, I+Poles.LowerCol()-1) = CPoles (I);
  }
}

//=======================================================================
//function : SetPoleRow
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPoleRow(const Standard_Integer      UIndex,
  const TColgp_Array1OfPnt &  CPoles,
  const TColStd_Array1OfReal& CPoleWeights)
{
  SetPoleRow  (UIndex, CPoles);
  SetWeightRow(UIndex, CPoleWeights);  
}

//=======================================================================
//function : SetPole
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPole (const Standard_Integer UIndex,
  const Standard_Integer VIndex,
  const gp_Pnt&          P)
{
  poles->SetValue (UIndex+poles->LowerRow()-1, VIndex+poles->LowerCol()-1, P);
}

//=======================================================================
//function : SetPole
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetPole (const Standard_Integer UIndex,
  const Standard_Integer VIndex,
  const gp_Pnt&          P, 
  const Standard_Real    Weight)
{
  SetWeight(UIndex, VIndex, Weight);
  SetPole  (UIndex, VIndex, P);
}

//=======================================================================
//function : MovePoint
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::MovePoint(const Standard_Real U,
  const Standard_Real V,
  const gp_Pnt& P,
  const Standard_Integer UIndex1,
  const Standard_Integer UIndex2,
  const Standard_Integer VIndex1,
  const Standard_Integer VIndex2,
  Standard_Integer& UFirstModifiedPole,
  Standard_Integer& ULastmodifiedPole,
  Standard_Integer& VFirstModifiedPole,
  Standard_Integer& VLastmodifiedPole)
{
  if (UIndex1 < 1 || UIndex1 > poles->UpperRow() || 
    UIndex2 < 1 || UIndex2 > poles->UpperRow() || UIndex1 > UIndex2 ||
    VIndex1 < 1 || VIndex1 > poles->UpperCol() || 
    VIndex2 < 1 || VIndex2 > poles->UpperCol() || VIndex1 > VIndex2)
  {
    throw Standard_OutOfRange ("asiAlgo_BSplineSurface::MovePoint: Index and #pole mismatch");
  }

  TColgp_Array2OfPnt npoles(1, poles->UpperRow(), 1, poles->UpperCol());
  gp_Pnt P0;
  D0(U, V, P0);
  gp_Vec Displ(P0, P);
  Standard_Boolean rational = (urational || vrational);
  BSplSLib::MovePoint(U, V, Displ, UIndex1, UIndex2, VIndex1, VIndex2, udeg, vdeg,
    rational, poles->Array2(), weights->Array2(),
    ufknots->Array1(), vfknots->Array1(), 
    UFirstModifiedPole, ULastmodifiedPole,
    VFirstModifiedPole, VLastmodifiedPole,
    npoles);
  if (UFirstModifiedPole) {
    poles->ChangeArray2() = npoles;
  }
  maxderivinvok = 0;
}

//=======================================================================
//function : Bounds
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::Bounds (Standard_Real& U1,
  Standard_Real& U2,
  Standard_Real& V1,
  Standard_Real& V2) const
{
  U1 = ufknots->Value (udeg+1);
  U2 = ufknots->Value (ufknots->Upper()-udeg);
  V1 = vfknots->Value (vdeg+1);
  V2 = vfknots->Value (vfknots->Upper()-vdeg);
}

//=======================================================================
//function : MaxDegree
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::MaxDegree () 
{ 
  return BSplCLib::MaxDegree(); 
}

//=======================================================================
//function : IsURational
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsURational () const 
{
  return urational; 
}

//=======================================================================
//function : IsVRational
//purpose  : 
//=======================================================================

Standard_Boolean asiAlgo_BSplineSurface::IsVRational () const 
{
  return vrational; 
}

//=======================================================================
//function : Continuity
//purpose  : 
//=======================================================================

GeomAbs_Shape asiAlgo_BSplineSurface::Continuity () const 
{
  return ((Usmooth < Vsmooth) ? Usmooth : Vsmooth) ; 
}

//=======================================================================
//function : NbUKnots
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::NbUKnots () const 
{
  return uknots->Length(); 
}

//=======================================================================
//function : NbUPoles
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::NbUPoles () const 
{
  return poles->ColLength(); 
}

//=======================================================================
//function : NbVKnots
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::NbVKnots () const 
{
  return vknots->Length(); 
}

//=======================================================================
//function : NbVPoles
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::NbVPoles () const 
{
  return poles->RowLength(); 
}

//=======================================================================
//function : UDegree
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::UDegree () const 
{
  return udeg; 
}

//=======================================================================
//function : VDegree
//purpose  : 
//=======================================================================

Standard_Integer asiAlgo_BSplineSurface::VDegree () const 
{
  return vdeg; 
}

//=======================================================================
//function : UKnotDistribution
//purpose  : 
//=======================================================================

GeomAbs_BSplKnotDistribution asiAlgo_BSplineSurface::UKnotDistribution() const 
{ 
  return uknotSet; 
}


//=======================================================================
//function : VKnotDistribution
//purpose  : 
//=======================================================================

GeomAbs_BSplKnotDistribution asiAlgo_BSplineSurface::VKnotDistribution() const 
{
  return vknotSet; 
}

//=======================================================================
//function : InsertUKnots
//purpose  : 
//=======================================================================

void  asiAlgo_BSplineSurface::InsertUKnots
(const TColStd_Array1OfReal&    Knots, 
  const TColStd_Array1OfInteger& Mults,
  const Standard_Real            ParametricTolerance,
  const Standard_Boolean         Add)
{
  // Check and compute new sizes
  Standard_Integer nbpoles, nbknots;

  if ( !BSplCLib::PrepareInsertKnots(udeg,uperiodic,
    uknots->Array1(),umults->Array1(),
    Knots,&Mults,nbpoles,nbknots,
    ParametricTolerance,Add))
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::InsertUKnots");

  if ( nbpoles == poles->ColLength()) return;

  Handle(TColgp_HArray2OfPnt) npoles
    = new TColgp_HArray2OfPnt(1,nbpoles, 1,poles->RowLength());
  Handle(TColStd_HArray2OfReal) nweights =
    new TColStd_HArray2OfReal(1,nbpoles, 
      1,poles->RowLength(),
      1.0);
  Handle(TColStd_HArray1OfReal)    nknots = uknots;
  Handle(TColStd_HArray1OfInteger) nmults = umults;

  if ( nbknots != uknots->Length()) {
    nknots = new TColStd_HArray1OfReal(1,nbknots);
    nmults = new TColStd_HArray1OfInteger(1,nbknots);
  }

  if ( urational || vrational) {
    BSplSLib::InsertKnots(Standard_True,
      udeg, uperiodic,
      poles->Array2() , &weights->Array2(),
      uknots->Array1(), umults->Array1(),
      Knots, &Mults,
      npoles->ChangeArray2(),
      &nweights->ChangeArray2(),
      nknots->ChangeArray1(), nmults->ChangeArray1(),
      ParametricTolerance, Add);
  }
  else {
    BSplSLib::InsertKnots(Standard_True,
      udeg, uperiodic,
      poles->Array2() , BSplSLib::NoWeights(),
      uknots->Array1(), umults->Array1(),
      Knots, &Mults,
      npoles->ChangeArray2(),
      BSplSLib::NoWeights(),
      nknots->ChangeArray1(), nmults->ChangeArray1(),
      ParametricTolerance, Add);
  }

  poles = npoles;
  weights = nweights;
  uknots = nknots;
  umults = nmults;
  UpdateUKnots();

}

//=======================================================================
//function : InsertVKnots
//purpose  : 
//=======================================================================

void  asiAlgo_BSplineSurface::InsertVKnots
(const TColStd_Array1OfReal& Knots, 
  const TColStd_Array1OfInteger& Mults, 
  const Standard_Real ParametricTolerance,
  const Standard_Boolean Add)
{
  // Check and compute new sizes
  Standard_Integer nbpoles, nbknots;

  if ( !BSplCLib::PrepareInsertKnots(vdeg,vperiodic,
    vknots->Array1(),vmults->Array1(),
    Knots,&Mults,nbpoles,nbknots,
    ParametricTolerance, Add))
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::InsertVKnots");

  if ( nbpoles == poles->RowLength()) return;

  Handle(TColgp_HArray2OfPnt) npoles
    = new TColgp_HArray2OfPnt(1,poles->ColLength(), 1,nbpoles);
  Handle(TColStd_HArray2OfReal) nweights =
    new TColStd_HArray2OfReal(1,poles->ColLength(),
      1,nbpoles,
      1.0);
  Handle(TColStd_HArray1OfReal)    nknots = vknots;
  Handle(TColStd_HArray1OfInteger) nmults = vmults;

  if ( nbknots != vknots->Length()) {
    nknots = new TColStd_HArray1OfReal(1,nbknots);
    nmults = new TColStd_HArray1OfInteger(1,nbknots);
  }

  if ( urational || vrational) {
    BSplSLib::InsertKnots(Standard_False,
      vdeg, vperiodic,
      poles->Array2() , &weights->Array2(),
      vknots->Array1(), vmults->Array1(),
      Knots, &Mults,
      npoles->ChangeArray2(),
      &nweights->ChangeArray2(),
      nknots->ChangeArray1(), nmults->ChangeArray1(),
      ParametricTolerance, Add);
  }
  else {
    BSplSLib::InsertKnots(Standard_False,
      vdeg, vperiodic,
      poles->Array2() , BSplSLib::NoWeights(),
      vknots->Array1(), vmults->Array1(),
      Knots, &Mults,
      npoles->ChangeArray2(),
      BSplSLib::NoWeights(),
      nknots->ChangeArray1(), nmults->ChangeArray1(),
      ParametricTolerance, Add);
  }

  poles = npoles;
  weights = nweights;
  vknots = nknots;
  vmults = nmults;
  UpdateVKnots();

}

//=======================================================================
//function : RemoveUKnot
//purpose  : 
//=======================================================================

Standard_Boolean  asiAlgo_BSplineSurface::RemoveUKnot
(const Standard_Integer Index, 
  const Standard_Integer M, 
  const Standard_Real Tolerance)
{
  if ( M < 0 ) return Standard_True;

  Standard_Integer I1 = FirstUKnotIndex ();
  Standard_Integer I2 = LastUKnotIndex  ();

  if ( !uperiodic && (Index <= I1 || Index >= I2) )
  {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::RemoveUKnot: invalid Index");
  }
  else if ( uperiodic  && (Index < I1 || Index > I2))
  {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::RemoveUKnot: invalid Index for periodic case");
  }

  const TColgp_Array2OfPnt   & oldpoles = poles->Array2();

  Standard_Integer step = umults->Value(Index) - M;
  if (step <= 0 ) return Standard_True;

  Handle(TColgp_HArray2OfPnt) npoles = 
    new TColgp_HArray2OfPnt( 1, oldpoles.ColLength() - step,
      1, oldpoles.RowLength());
  Handle(TColStd_HArray1OfReal)    nknots = uknots;
  Handle(TColStd_HArray1OfInteger) nmults = umults;

  if ( M == 0) {
    nknots = new TColStd_HArray1OfReal(1,uknots->Length()-1);
    nmults = new TColStd_HArray1OfInteger(1,uknots->Length()-1);
  }
  Handle(TColStd_HArray2OfReal) nweights ;
  if (urational || vrational) {
    nweights = 
      new TColStd_HArray2OfReal( 1, npoles->ColLength(),
        1, npoles->RowLength());
    if (!BSplSLib::RemoveKnot(Standard_True,
      Index,M,udeg,uperiodic,
      poles->Array2(),&weights->Array2(),
      uknots->Array1(),umults->Array1(),
      npoles->ChangeArray2(),
      &nweights->ChangeArray2(),
      nknots->ChangeArray1(),nmults->ChangeArray1(),
      Tolerance))
      return Standard_False;
  }
  else {
    //
    // sync the size of the weights
    //
    nweights = 
      new TColStd_HArray2OfReal(1, npoles->ColLength(),
        1, npoles->RowLength(),
        1.0e0 );
    if (!BSplSLib::RemoveKnot(Standard_True,
      Index,M,udeg,uperiodic,
      poles->Array2(),BSplSLib::NoWeights(),
      uknots->Array1(),umults->Array1(),
      npoles->ChangeArray2(),
      BSplSLib::NoWeights(),
      nknots->ChangeArray1(),nmults->ChangeArray1(),
      Tolerance))
      return Standard_False;
  }

  poles = npoles;
  weights = nweights;
  uknots = nknots;
  umults = nmults;

  maxderivinvok = 0;
  UpdateUKnots();
  return Standard_True;
}

//=======================================================================
//function : RemoveVKnot
//purpose  : 
//=======================================================================

Standard_Boolean  asiAlgo_BSplineSurface::RemoveVKnot
(const Standard_Integer Index, 
  const Standard_Integer M,
  const Standard_Real Tolerance)
{
  if ( M < 0 ) return Standard_True;

  Standard_Integer I1 = FirstVKnotIndex ();
  Standard_Integer I2 = LastVKnotIndex  ();

  if ( !vperiodic && (Index <= I1 || Index >= I2) )
  {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::RemoveVKnot: invalid Index");
  }
  else if ( vperiodic  && (Index < I1 || Index > I2)) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::RemoveVKnot: invalid Index for periodic case");
  }

  const TColgp_Array2OfPnt   & oldpoles = poles->Array2();

  Standard_Integer step = vmults->Value(Index) - M;
  if (step <= 0 ) return Standard_True;

  Handle(TColgp_HArray2OfPnt) npoles = 
    new TColgp_HArray2OfPnt( 1, oldpoles.ColLength(),
      1, oldpoles.RowLength() - step);
  Handle(TColStd_HArray1OfReal)    nknots = vknots;
  Handle(TColStd_HArray1OfInteger) nmults = vmults;

  if ( M == 0) {
    nknots = new TColStd_HArray1OfReal(1,vknots->Length()-1);
    nmults = new TColStd_HArray1OfInteger(1,vknots->Length()-1);
  }
  Handle(TColStd_HArray2OfReal) nweights ;
  if (urational || vrational) {
    nweights = 
      new TColStd_HArray2OfReal( 1, npoles->ColLength(),
        1, npoles->RowLength()) ;


    if (!BSplSLib::RemoveKnot(Standard_False,
      Index,M,vdeg,vperiodic,
      poles->Array2(),&weights->Array2(),
      vknots->Array1(),vmults->Array1(),
      npoles->ChangeArray2(),
      &nweights->ChangeArray2(),
      nknots->ChangeArray1(),nmults->ChangeArray1(),
      Tolerance))
      return Standard_False;
  }
  else {
    //
    // sync the size of the weights array
    //
    nweights = 
      new TColStd_HArray2OfReal(1, npoles->ColLength(),
        1, npoles->RowLength(),
        1.0e0 );
    if (!BSplSLib::RemoveKnot(Standard_False,
      Index,M,vdeg,vperiodic,
      poles->Array2(),BSplSLib::NoWeights(),
      vknots->Array1(),vmults->Array1(),
      npoles->ChangeArray2(),
      BSplSLib::NoWeights(),
      nknots->ChangeArray1(),nmults->ChangeArray1(),
      Tolerance))
      return Standard_False;
  }

  poles = npoles;
  vknots = nknots;
  vmults = nmults;
  weights = nweights;
  maxderivinvok = 0;
  UpdateVKnots();
  return Standard_True;
}

//=======================================================================
//function : Resolution
//purpose  : 
//=======================================================================

void   asiAlgo_BSplineSurface::Resolution( const Standard_Real  Tolerance3D,
  Standard_Real&        UTolerance,
  Standard_Real&        VTolerance)
{
  if(!maxderivinvok){
    BSplSLib::Resolution(poles  ->Array2(),
      &weights->Array2(),
      uknots ->Array1(),
      vknots ->Array1(),
      umults ->Array1(),
      vmults ->Array1(),
      udeg,
      vdeg,
      urational,
      vrational,
      uperiodic,
      vperiodic,
      1.,
      umaxderivinv,
      vmaxderivinv) ;
    maxderivinvok = 1;
  }
  UTolerance = Tolerance3D * umaxderivinv;
  VTolerance = Tolerance3D * vmaxderivinv;
}

//=======================================================================
//function : CheckSurfaceData
//purpose  : Internal use only.
//=======================================================================
static void CheckSurfaceData
(const TColgp_Array2OfPnt&      SPoles,
 const TColStd_Array1OfReal&    SUKnots,
 const TColStd_Array1OfReal&    SVKnots,
 const TColStd_Array1OfInteger& SUMults,
 const TColStd_Array1OfInteger& SVMults,
 const Standard_Integer         UDegree,
 const Standard_Integer         VDegree,
 const Standard_Boolean         UPeriodic,
 const Standard_Boolean         VPeriodic)
{
  if (UDegree < 1 || UDegree > asiAlgo_BSplineSurface::MaxDegree () || 
      VDegree < 1 || VDegree > asiAlgo_BSplineSurface::MaxDegree ()) {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: invalid degree");
  }
  if (SPoles.ColLength () < 2 || SPoles.RowLength () < 2) {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: at least 2 poles required");
  }

  if (SUKnots.Length() != SUMults.Length() ||
      SVKnots.Length() != SVMults.Length()) {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: Knot and Mult array size mismatch");
  }

  Standard_Integer i;
  for (i = SUKnots.Lower(); i < SUKnots.Upper(); i++) {
    if (SUKnots(i+1) - SUKnots(i) <= Epsilon(Abs(SUKnots(i)))) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface: UKnots interval values too close");
    }
  }

  for (i = SVKnots.Lower(); i < SVKnots.Upper(); i++) {
    if (SVKnots(i+1) - SVKnots(i) <= Epsilon(Abs(SVKnots(i)))) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface: VKnots interval values too close");
    }
  }
  
  if (SPoles.ColLength() != BSplCLib::NbPoles(UDegree,UPeriodic,SUMults))
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: # U Poles and degree mismatch");

  if (SPoles.RowLength() != BSplCLib::NbPoles(VDegree,VPeriodic,SVMults))
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: # V Poles and degree mismatch");
}

//=======================================================================
//function : Rational
//purpose  : Internal use only.
//=======================================================================

static void Rational(const TColStd_Array2OfReal& Weights,
		     Standard_Boolean& Urational,
		     Standard_Boolean& Vrational)
{
  Standard_Integer I,J;
  J = Weights.LowerCol ();
  Vrational = Standard_False;
  while (!Vrational && J <= Weights.UpperCol()) {
    I = Weights.LowerRow();
    while (!Vrational && I <= Weights.UpperRow() - 1) {
      Vrational = (Abs(Weights (I, J) - Weights (I+1, J)) 
                   > Epsilon (Abs(Weights (I, J))));
      I++;
    }
    J++;
  }

  I = Weights.LowerRow ();
  Urational = Standard_False;
  while (!Urational && I <= Weights.UpperRow()) {
    J = Weights.LowerCol();
    while (!Urational && J <= Weights.UpperCol() - 1) {
      Urational = (Abs(Weights (I, J) - Weights (I, J+1))
                   > Epsilon (Abs(Weights (I, J))));
      J++;
    }
    I++;
  }
}

//=======================================================================
//function : Copy
//purpose  : 
//=======================================================================

Handle(Geom_Geometry) asiAlgo_BSplineSurface::Copy () const
{
  Handle(asiAlgo_BSplineSurface) S;
  if (urational || vrational) 
    S = new asiAlgo_BSplineSurface (poles->Array2() , weights->Array2(), 
				 uknots->Array1(), vknots->Array1(), 
				 umults->Array1(), vmults->Array1(), 
				 udeg     , vdeg, 
				 uperiodic, vperiodic);
  else
    S = new asiAlgo_BSplineSurface (poles->Array2(),
				 uknots->Array1(), vknots->Array1(), 
				 umults->Array1(), vmults->Array1(), 
				 udeg     , vdeg, 
				 uperiodic, vperiodic);
  return S;
}

//=======================================================================
//function : asiAlgo_BSplineSurface
//purpose  : 
//=======================================================================

asiAlgo_BSplineSurface::asiAlgo_BSplineSurface
(const TColgp_Array2OfPnt&      Poles, 
 const TColStd_Array1OfReal&    UKnots, 
 const TColStd_Array1OfReal&    VKnots,
 const TColStd_Array1OfInteger& UMults, 
 const TColStd_Array1OfInteger& VMults,
 const Standard_Integer         UDegree, 
 const Standard_Integer         VDegree,
 const Standard_Boolean         UPeriodic,
 const Standard_Boolean         VPeriodic
 ) :
 urational(Standard_False),
 vrational(Standard_False),
 uperiodic(UPeriodic),
 vperiodic(VPeriodic),
 udeg(UDegree),
 vdeg(VDegree),
 maxderivinvok(0)

{

  // check
  
  CheckSurfaceData(Poles,
		   UKnots   , VKnots,
		   UMults   , VMults,
		   UDegree  , VDegree,
		   UPeriodic, VPeriodic);

  // copy arrays

  poles   = new TColgp_HArray2OfPnt(1,Poles.ColLength(),
				    1,Poles.RowLength());
  poles->ChangeArray2() = Poles;

  weights = new TColStd_HArray2OfReal (1,Poles.ColLength(),
				       1,Poles.RowLength(), 1.0);

  uknots  = new TColStd_HArray1OfReal    (1,UKnots.Length());
  uknots->ChangeArray1() = UKnots;

  umults  = new TColStd_HArray1OfInteger (1,UMults.Length());
  umults->ChangeArray1() = UMults;

  vknots  = new TColStd_HArray1OfReal    (1,VKnots.Length());
  vknots->ChangeArray1() = VKnots;

  vmults  = new TColStd_HArray1OfInteger (1,VMults.Length());
  vmults->ChangeArray1() = VMults;

  UpdateUKnots();
  UpdateVKnots();
}

//=======================================================================
//function : asiAlgo_BSplineSurface
//purpose  : 
//=======================================================================

asiAlgo_BSplineSurface::asiAlgo_BSplineSurface
(const TColgp_Array2OfPnt&      Poles,
 const TColStd_Array2OfReal&    Weights,
 const TColStd_Array1OfReal&    UKnots,
 const TColStd_Array1OfReal&    VKnots,
 const TColStd_Array1OfInteger& UMults, 
 const TColStd_Array1OfInteger& VMults,
 const Standard_Integer         UDegree,
 const Standard_Integer         VDegree,
 const Standard_Boolean         UPeriodic,
 const Standard_Boolean         VPeriodic) :
 urational(Standard_False),
 vrational(Standard_False),
 uperiodic(UPeriodic),
 vperiodic(VPeriodic),
 udeg(UDegree),
 vdeg(VDegree),
 maxderivinvok(0)
{
  // check weights

  if (Weights.ColLength() != Poles.ColLength())
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: U Weights and Poles array size mismatch");

  if (Weights.RowLength() != Poles.RowLength())
    throw Standard_ConstructionError("asiAlgo_BSplineSurface: V Weights and Poles array size mismatch");

  Standard_Integer i,j;
  for (i = Weights.LowerRow(); i <= Weights.UpperRow(); i++) {
    for (j = Weights.LowerCol(); j <= Weights.UpperCol(); j++) {
      if (Weights(i,j) <= gp::Resolution())  
        throw Standard_ConstructionError("asiAlgo_BSplineSurface: Weights values too small");
    }
  }
  
  // check really rational
  
  Rational(Weights, urational, vrational);

  // check
  
  CheckSurfaceData(Poles,
		   UKnots   , VKnots,
		   UMults   , VMults,
		   UDegree  , VDegree,
		   UPeriodic, VPeriodic);

  // copy arrays

  poles   = new TColgp_HArray2OfPnt(1,Poles.ColLength(),
				    1,Poles.RowLength());
  poles->ChangeArray2() = Poles;

  weights = new TColStd_HArray2OfReal (1,Poles.ColLength(),
				       1,Poles.RowLength());
  weights->ChangeArray2() = Weights;

  uknots  = new TColStd_HArray1OfReal    (1,UKnots.Length());
  uknots->ChangeArray1() = UKnots;

  umults  = new TColStd_HArray1OfInteger (1,UMults.Length());
  umults->ChangeArray1() = UMults;

  vknots  = new TColStd_HArray1OfReal    (1,VKnots.Length());
  vknots->ChangeArray1() = VKnots;

  vmults  = new TColStd_HArray1OfInteger (1,VMults.Length());
  vmults->ChangeArray1() = VMults;

  UpdateUKnots();
  UpdateVKnots();
}

//=======================================================================
//function : ExchangeUV
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::ExchangeUV ()
{
  Standard_Integer LC = poles->LowerCol();
  Standard_Integer UC = poles->UpperCol();
  Standard_Integer LR = poles->LowerRow();
  Standard_Integer UR = poles->UpperRow();

  Handle(TColgp_HArray2OfPnt) npoles = new TColgp_HArray2OfPnt (LC, UC, LR, UR);
  Handle(TColStd_HArray2OfReal) nweights;
  if (!weights.IsNull())
  {
    nweights = new TColStd_HArray2OfReal (LC, UC, LR, UR);
  }

  const TColgp_Array2OfPnt& spoles = poles->Array2();
  const TColStd_Array2OfReal* sweights = !weights.IsNull() ? &weights->Array2() : NULL;
  
  TColgp_Array2OfPnt& snpoles = npoles->ChangeArray2();
  TColStd_Array2OfReal* snweights = !nweights.IsNull() ? &nweights->ChangeArray2() : NULL;
  for (Standard_Integer i = LC; i <= UC; i++)
  {
    for (Standard_Integer j = LR; j <= UR; j++)
    {
      snpoles (i, j) = spoles (j, i);
      if (snweights != NULL)
      {
        snweights->ChangeValue (i, j) = sweights->Value (j, i);
      }
    }
  }
  poles   = npoles;
  weights = nweights;

  std::swap (urational, vrational);
  std::swap (uperiodic, vperiodic);
  std::swap (udeg,   vdeg);
  std::swap (uknots, vknots);
  std::swap (umults, vmults);

  UpdateUKnots();
  UpdateVKnots();
}

//=======================================================================
//function : IncreaseDegree
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::IncreaseDegree (const Standard_Integer UDegree,
					  const Standard_Integer VDegree)
{ 
  if (UDegree != udeg) {
    if ( UDegree < udeg || UDegree > asiAlgo_BSplineSurface::MaxDegree())
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::IncreaseDegree: bad U degree value");
    
    Standard_Integer FromK1 = FirstUKnotIndex();
    Standard_Integer ToK2   = LastUKnotIndex();

    Standard_Integer Step   = UDegree - udeg;

    Handle(TColgp_HArray2OfPnt) npoles = new
      TColgp_HArray2OfPnt( 1, poles->ColLength() + Step * (ToK2 - FromK1),
			  1, poles->RowLength());

    Standard_Integer nbknots = BSplCLib::IncreaseDegreeCountKnots
      (udeg,UDegree,uperiodic,umults->Array1());

    Handle(TColStd_HArray1OfReal) nknots = 
      new TColStd_HArray1OfReal(1,nbknots);
    
    Handle(TColStd_HArray1OfInteger) nmults = 
      new TColStd_HArray1OfInteger(1,nbknots);

    Handle(TColStd_HArray2OfReal) nweights 
      = new TColStd_HArray2OfReal(1,npoles->ColLength(),
				  1,npoles->RowLength(), 1.);

    if (urational || vrational) {
      
      BSplSLib::IncreaseDegree
	(Standard_True, udeg, UDegree, uperiodic,
	 poles->Array2(),&weights->Array2(),
	 uknots->Array1(),umults->Array1(),
	 npoles->ChangeArray2(),&nweights->ChangeArray2(),
	 nknots->ChangeArray1(),nmults->ChangeArray1());
    }
    else {

      BSplSLib::IncreaseDegree
	(Standard_True, udeg, UDegree, uperiodic,
	 poles->Array2(),BSplSLib::NoWeights(),
	 uknots->Array1(),umults->Array1(),
	 npoles->ChangeArray2(),BSplSLib::NoWeights(),
	 nknots->ChangeArray1(),nmults->ChangeArray1());
    }
    udeg    = UDegree;
    poles   = npoles;
    weights = nweights;
    uknots  = nknots;
    umults  = nmults;
    UpdateUKnots();
  }

  if (VDegree != vdeg) {
    if ( VDegree < vdeg || VDegree > asiAlgo_BSplineSurface::MaxDegree())
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::IncreaseDegree: bad V degree value");
    
    Standard_Integer FromK1 = FirstVKnotIndex();
    Standard_Integer ToK2   = LastVKnotIndex();

    Standard_Integer Step   = VDegree - vdeg;

    Handle(TColgp_HArray2OfPnt) npoles = new
      TColgp_HArray2OfPnt( 1, poles->ColLength(),
			  1, poles->RowLength() + Step * (ToK2 - FromK1));

    Standard_Integer nbknots = BSplCLib::IncreaseDegreeCountKnots
      (vdeg,VDegree,vperiodic,vmults->Array1());

    Handle(TColStd_HArray1OfReal) nknots = 
      new TColStd_HArray1OfReal(1,nbknots);
    
    Handle(TColStd_HArray1OfInteger) nmults = 
      new TColStd_HArray1OfInteger(1,nbknots);

    Handle(TColStd_HArray2OfReal) nweights
      = new TColStd_HArray2OfReal(1,npoles->ColLength(),
				  1,npoles->RowLength(), 1.);

    if (urational || vrational) {
      
      BSplSLib::IncreaseDegree
	(Standard_False, vdeg, VDegree, vperiodic,
	 poles->Array2(),&weights->Array2(),
	 vknots->Array1(),vmults->Array1(),
	 npoles->ChangeArray2(),&nweights->ChangeArray2(),
	 nknots->ChangeArray1(),nmults->ChangeArray1());
    }
    else {

      BSplSLib::IncreaseDegree
	(Standard_False, vdeg, VDegree, vperiodic,
	 poles->Array2(),BSplSLib::NoWeights(),
	 vknots->Array1(),vmults->Array1(),
	 npoles->ChangeArray2(),BSplSLib::NoWeights(),
	 nknots->ChangeArray1(),nmults->ChangeArray1());
    }
    vdeg    = VDegree;
    poles   = npoles;
    weights = nweights;
    vknots  = nknots;
    vmults  = nmults;
    UpdateVKnots();
  }
}

//=======================================================================
//function : IncreaseUMultiplicity
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::IncreaseUMultiplicity
(const Standard_Integer UIndex, 
 const Standard_Integer M)
{
  TColStd_Array1OfReal k(1,1);
  k(1) = uknots->Value(UIndex);
  TColStd_Array1OfInteger m(1,1);
  m(1) = M - umults->Value(UIndex);
  InsertUKnots(k,m,Epsilon(1.),Standard_True);
}

//=======================================================================
//function : IncreaseUMultiplicity
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::IncreaseUMultiplicity
(const Standard_Integer FromI1, 
 const Standard_Integer ToI2,
 const Standard_Integer M)
{
  Handle(TColStd_HArray1OfReal) tk = uknots;
  TColStd_Array1OfReal k((uknots->Array1())(FromI1),FromI1,ToI2);
  TColStd_Array1OfInteger m(FromI1, ToI2);
  for (Standard_Integer i = FromI1; i <= ToI2; i++) 
    m(i) = M - umults->Value(i);
  InsertUKnots(k,m,Epsilon(1.),Standard_True);
}

//=======================================================================
//function : IncreaseVMultiplicity
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::IncreaseVMultiplicity
(const Standard_Integer VIndex, 
 const Standard_Integer M)
{
  TColStd_Array1OfReal k(1,1);
  k(1) = vknots->Value(VIndex);
  TColStd_Array1OfInteger m(1,1);
  m(1) = M - vmults->Value(VIndex);
  InsertVKnots(k,m,Epsilon(1.),Standard_True);
}

//=======================================================================
//function : IncreaseVMultiplicity
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::IncreaseVMultiplicity
(const Standard_Integer FromI1,
 const Standard_Integer ToI2,
 const Standard_Integer M)
{
  Handle(TColStd_HArray1OfReal) tk = vknots;
  TColStd_Array1OfReal k((vknots->Array1())(FromI1),FromI1,ToI2);
  TColStd_Array1OfInteger m(FromI1,ToI2);
  for (Standard_Integer i = FromI1; i <= ToI2; i++)
    m(i) = M - vmults->Value(i);
  InsertVKnots(k,m,Epsilon(1.),Standard_True);
}

//=======================================================================
//function : segment
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::segment(const Standard_Real U1,
                                  const Standard_Real U2,
                                  const Standard_Real V1,
                                  const Standard_Real V2,
                                  const Standard_Real EpsU,
                                  const Standard_Real EpsV,
                                  const Standard_Boolean SegmentInU,
                                  const Standard_Boolean SegmentInV)
{
  Standard_Real deltaU = U2 - U1;
  if (uperiodic) {
    Standard_Real aUPeriod = uknots->Last() - uknots->First();
    if (deltaU - aUPeriod > Precision::PConfusion())
      throw Standard_DomainError("asiAlgo_BSplineSurface::Segment");
    if (deltaU > aUPeriod)
      deltaU = aUPeriod;
  }

  Standard_Real deltaV = V2 - V1;
  if (vperiodic) {
    Standard_Real aVPeriod = vknots->Last() - vknots->First();
    if (deltaV - aVPeriod > Precision::PConfusion())
      throw Standard_DomainError("asiAlgo_BSplineSurface::Segment");
    if (deltaV > aVPeriod)
      deltaV = aVPeriod;
  }

  Standard_Real NewU1, NewU2, NewV1, NewV2;
  Standard_Real U, V;
  Standard_Integer indexU, indexV;

  indexU = 0;
  BSplCLib::LocateParameter(udeg, uknots->Array1(), umults->Array1(),
                            U1, uperiodic, uknots->Lower(), uknots->Upper(),
                            indexU, NewU1);
  indexU = 0;
  BSplCLib::LocateParameter(udeg, uknots->Array1(), umults->Array1(),
                            U2, uperiodic, uknots->Lower(), uknots->Upper(),
                            indexU, NewU2);
  if (SegmentInU) {
    // inserting the UKnots
    TColStd_Array1OfReal    UKnots(1, 2);
    TColStd_Array1OfInteger UMults(1, 2);
    UKnots(1) = Min(NewU1, NewU2);
    UKnots(2) = Max(NewU1, NewU2);
    UMults(1) = UMults(2) = udeg;

    InsertUKnots(UKnots, UMults, EpsU);
  }

  indexV = 0;
  BSplCLib::LocateParameter(vdeg, vknots->Array1(), vmults->Array1(),
                            V1, vperiodic, vknots->Lower(), vknots->Upper(),
                            indexV, NewV1);
  indexV = 0;
  BSplCLib::LocateParameter(vdeg, vknots->Array1(), vmults->Array1(),
                            V2, vperiodic, vknots->Lower(), vknots->Upper(),
                            indexV, NewV2);
  if (SegmentInV) {
    // Inserting the VKnots
    TColStd_Array1OfReal    VKnots(1, 2);
    TColStd_Array1OfInteger VMults(1, 2);

    VKnots(1) = Min(NewV1, NewV2);
    VKnots(2) = Max(NewV1, NewV2);
    VMults(1) = VMults(2) = vdeg;
    InsertVKnots(VKnots, VMults, EpsV);
  }

  if (uperiodic && SegmentInU) { // set the origine at NewU1
    Standard_Integer index = 0;
    BSplCLib::LocateParameter(udeg, uknots->Array1(), umults->Array1(),
      U1, uperiodic, uknots->Lower(), uknots->Upper(),
      index, U);
    if (Abs(uknots->Value(index + 1) - U) <= EpsU)
      index++;
    SetUOrigin(index);
    SetUNotPeriodic();
  }

  // compute index1 and index2 to set the new knots and mults 
  Standard_Integer index1U = 0, index2U = 0;
  Standard_Integer FromU1 = uknots->Lower();
  Standard_Integer ToU2 = uknots->Upper();
  BSplCLib::LocateParameter(udeg, uknots->Array1(), umults->Array1(),
                            NewU1, uperiodic, FromU1, ToU2, index1U, U);
  if (Abs(uknots->Value(index1U + 1) - U) <= EpsU)
    index1U++;
  BSplCLib::LocateParameter(udeg, uknots->Array1(), umults->Array1(),
                            NewU1 + deltaU, uperiodic, FromU1, ToU2, index2U, U);
  if (Abs(uknots->Value(index2U + 1) - U) <= EpsU || index2U == index1U)
    index2U++;

  Standard_Integer nbuknots = index2U - index1U + 1;

  Handle(TColStd_HArray1OfReal)
    nuknots = new TColStd_HArray1OfReal(1, nbuknots);
  Handle(TColStd_HArray1OfInteger)
    numults = new TColStd_HArray1OfInteger(1, nbuknots);

  Standard_Integer i, k = 1;
  for (i = index1U; i <= index2U; i++) {
    nuknots->SetValue(k, uknots->Value(i));
    numults->SetValue(k, umults->Value(i));
    k++;
  }
  if (SegmentInU) {
    numults->SetValue(1, udeg + 1);
    numults->SetValue(nbuknots, udeg + 1);
  }

  if (vperiodic&& SegmentInV) { // set the origine at NewV1
    Standard_Integer index = 0;
    BSplCLib::LocateParameter(vdeg, vknots->Array1(), vmults->Array1(),
      V1, vperiodic, vknots->Lower(), vknots->Upper(),
      index, V);
    if (Abs(vknots->Value(index + 1) - V) <= EpsV)
      index++;
    SetVOrigin(index);
    SetVNotPeriodic();
  }

  // compute index1 and index2 to set the new knots and mults 
  Standard_Integer index1V = 0, index2V = 0;
  Standard_Integer FromV1 = vknots->Lower();
  Standard_Integer ToV2 = vknots->Upper();
  BSplCLib::LocateParameter(vdeg, vknots->Array1(), vmults->Array1(),
                            NewV1, vperiodic, FromV1, ToV2, index1V, V);
  if (Abs(vknots->Value(index1V + 1) - V) <= EpsV)
    index1V++;
  BSplCLib::LocateParameter(vdeg, vknots->Array1(), vmults->Array1(),
                            NewV1 + deltaV, vperiodic, FromV1, ToV2, index2V, V);
  if (Abs(vknots->Value(index2V + 1) - V) <= EpsV || index2V == index1V)
    index2V++;

  Standard_Integer nbvknots = index2V - index1V + 1;

  Handle(TColStd_HArray1OfReal)
    nvknots = new TColStd_HArray1OfReal(1, nbvknots);
  Handle(TColStd_HArray1OfInteger)
    nvmults = new TColStd_HArray1OfInteger(1, nbvknots);

  k = 1;
  for (i = index1V; i <= index2V; i++) {
    nvknots->SetValue(k, vknots->Value(i));
    nvmults->SetValue(k, vmults->Value(i));
    k++;
  }
  if (SegmentInV) {
    nvmults->SetValue(1, vdeg + 1);
    nvmults->SetValue(nbvknots, vdeg + 1);
  }


  // compute index1 and index2 to set the new poles and weights
  Standard_Integer pindex1U
    = BSplCLib::PoleIndex(udeg, index1U, uperiodic, umults->Array1());
  Standard_Integer pindex2U
    = BSplCLib::PoleIndex(udeg, index2U, uperiodic, umults->Array1());

  pindex1U++;
  pindex2U = Min(pindex2U + 1, poles->ColLength());

  Standard_Integer nbupoles = pindex2U - pindex1U + 1;

  // compute index1 and index2 to set the new poles and weights
  Standard_Integer pindex1V
    = BSplCLib::PoleIndex(vdeg, index1V, vperiodic, vmults->Array1());
  Standard_Integer pindex2V
    = BSplCLib::PoleIndex(vdeg, index2V, vperiodic, vmults->Array1());

  pindex1V++;
  pindex2V = Min(pindex2V + 1, poles->RowLength());

  Standard_Integer nbvpoles = pindex2V - pindex1V + 1;


  Handle(TColStd_HArray2OfReal) nweights;

  Handle(TColgp_HArray2OfPnt)
    npoles = new TColgp_HArray2OfPnt(1, nbupoles, 1, nbvpoles);
  k = 1;
  Standard_Integer j, l;
  if (urational || vrational) {
    nweights = new TColStd_HArray2OfReal(1, nbupoles, 1, nbvpoles);
    for (i = pindex1U; i <= pindex2U; i++) {
      l = 1;
      for (j = pindex1V; j <= pindex2V; j++) {
        npoles->SetValue(k, l, poles->Value(i, j));
        nweights->SetValue(k, l, weights->Value(i, j));
        l++;
      }
      k++;
    }
  }
  else {
    for (i = pindex1U; i <= pindex2U; i++) {
      l = 1;
      for (j = pindex1V; j <= pindex2V; j++) {
        npoles->SetValue(k, l, poles->Value(i, j));
        l++;
      }
      k++;
    }
  }

  uknots = nuknots;
  umults = numults;
  vknots = nvknots;
  vmults = nvmults;
  poles = npoles;
  if (urational || vrational)
    weights = nweights;
  else
    weights = new TColStd_HArray2OfReal(1, poles->ColLength(),
                                        1, poles->RowLength(), 1.0);

  maxderivinvok = 0;
  UpdateUKnots();
  UpdateVKnots();
}

//=======================================================================
//function : Segment
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::Segment(const Standard_Real U1, 
                                  const Standard_Real U2,
                                  const Standard_Real V1,
                                  const Standard_Real V2,
                                  int degU,
                                  int degV,
                                  const Standard_Real theUTolerance,
                                  const Standard_Real theVTolerance)
{
  udeg = degU;
  vdeg = degV;
  if ((U2 < U1) || (V2 < V1))
    throw Standard_DomainError("asiAlgo_BSplineSurface::Segment");

  Standard_Real aMaxU = Max(Abs(U2), Abs(U1));
  Standard_Real EpsU = Max(Epsilon(aMaxU), theUTolerance);
  
  Standard_Real aMaxV = Max(Abs(V2), Abs(V1));
  Standard_Real EpsV = Max(Epsilon(aMaxV), theVTolerance);

  segment(U1, U2, V1, V2, EpsU, EpsV, Standard_True, Standard_True);
}

//=======================================================================
//function : CheckAndSegment
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::CheckAndSegment(const Standard_Real U1, 
                                          const Standard_Real U2,
                                          const Standard_Real V1,
                                          const Standard_Real V2,
                                          const Standard_Real theUTolerance,
                                          const Standard_Real theVTolerance)
{

  if ((U2 < U1) || (V2 < V1))
    throw Standard_DomainError("asiAlgo_BSplineSurface::CheckAndSegment");

  Standard_Real aMaxU = Max(Abs(U2), Abs(U1));
  Standard_Real EpsU = Max(Epsilon(aMaxU), theUTolerance);
  
  Standard_Real aMaxV = Max(Abs(V2), Abs(V1));
  Standard_Real EpsV = Max(Epsilon(aMaxV), theVTolerance);

  Standard_Boolean segment_in_U = Standard_True;
  Standard_Boolean segment_in_V = Standard_True;
  segment_in_U = ( Abs(U1 - uknots->Value(uknots->Lower())) > EpsU )
                        || ( Abs(U2 - uknots->Value(uknots->Upper())) > EpsU );
  segment_in_V = ( Abs(V1 - vknots->Value(vknots->Lower())) > EpsV )
                        || ( Abs(V2 - vknots->Value(vknots->Upper())) > EpsV );

  segment(U1, U2, V1, V2, EpsU, EpsV, segment_in_U, segment_in_V);
}

//=======================================================================
//function : SetUKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUKnot
(const Standard_Integer UIndex,
 const Standard_Real    K      )
{
  if (UIndex < 1 || UIndex > uknots->Length())
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetUKnot: Index and #knots mismatch");

  Standard_Integer NewIndex = UIndex;
  Standard_Real DU = Abs(Epsilon (K));
  if (UIndex == 1) {
    if (K >= uknots->Value (2) - DU)
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnot: K out of range");
  }
  else if (UIndex == uknots->Length()) {
    if (K <= uknots->Value (uknots->Length()-1) + DU)  {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnot: K out of range");
    }
  }
  else {
    if (K <= uknots->Value (NewIndex-1) + DU || 
	K >= uknots->Value (NewIndex+1) - DU ) { 
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnot: K out of range");
    } 
  }
  
  if (K != uknots->Value (NewIndex)) {
    uknots->SetValue (NewIndex, K);
    maxderivinvok = 0;
    UpdateUKnots();
  }
}

//=======================================================================
//function : SetUKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUKnots (const TColStd_Array1OfReal& UK) {

  Standard_Integer Lower = UK.Lower();
  Standard_Integer Upper = UK.Upper();
  if (Lower < 1 || Lower > uknots->Length() ||
      Upper < 1 || Upper > uknots->Length() ) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetUKnots: invalid array dimension");
  }
  if (Lower > 1) {
    if (Abs (UK (Lower) - uknots->Value (Lower-1)) <= gp::Resolution()) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnots: invalid knot value");
    }
  }
  if (Upper < uknots->Length ()) {
    if (Abs (UK (Upper) - uknots->Value (Upper+1)) <= gp::Resolution()) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnots: invalid knot value");
    }
  }
  Standard_Real K1 = UK (Lower);
  for (Standard_Integer i = Lower; i <= Upper; i++) {
    uknots->SetValue (i, UK(i));
    if (i != Lower) {
      if (Abs (UK(i) - K1) <= gp::Resolution()) {
        throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetUKnots: invalid knot value");
      }
      K1 = UK (i);
    }
  }

  maxderivinvok = 0;
  UpdateUKnots();
}

//=======================================================================
//function : SetUKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetUKnot
(const Standard_Integer UIndex,
 const Standard_Real    K,
 const Standard_Integer M)
{
  IncreaseUMultiplicity (UIndex, M);
  SetUKnot (UIndex, K);
}

//=======================================================================
//function : SetVKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVKnot
(const Standard_Integer VIndex,
 const Standard_Real    K)
{
  if (VIndex < 1 || VIndex > vknots->Length())
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetVKnot: Index and #knots mismatch");
  Standard_Integer NewIndex = VIndex + vknots->Lower() - 1;
  Standard_Real DV = Abs(Epsilon (K));
  if (VIndex == 1) {
    if (K >=  vknots->Value (2) - DV) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnot: K out of range");
    }
  }
  else if (VIndex == vknots->Length()) {
    if (K <= vknots->Value (vknots->Length()-1) + DV)  {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnot: K out of range");
    }
  }
  else {
    if (K <= vknots->Value (NewIndex-1) + DV || 
	K >= vknots->Value (NewIndex+1) - DV ) { 
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnot: K out of range");
    } 
  }
  
  if (K != vknots->Value (NewIndex)) {
    vknots->SetValue (NewIndex, K);
    maxderivinvok = 0;
    UpdateVKnots();
  }
}

//=======================================================================
//function : SetVKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVKnots (const TColStd_Array1OfReal& VK) {

  Standard_Integer Lower = VK.Lower();
  Standard_Integer Upper = VK.Upper();
  if (Lower < 1 || Lower > vknots->Length() ||
      Upper < 1 || Upper > vknots->Length() ) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetVKnots: invalid array dimension");
  }
  if (Lower > 1) {
    if (Abs (VK (Lower) - vknots->Value (Lower-1)) <= gp::Resolution()) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnots: invalid knot value");
    }
  }
  if (Upper < vknots->Length ()) {
    if (Abs (VK (Upper) - vknots->Value (Upper+1)) <= gp::Resolution()) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnots: invalid knot value");
    }
  }
  Standard_Real K1 = VK (Lower);
  for (Standard_Integer i = Lower; i <= Upper; i++) {
    vknots->SetValue (i, VK(i));
    if (i != Lower) {
      if (Abs (VK(i) - K1) <= gp::Resolution()) {
        throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetVKnots: invalid knot value");
      }
      K1 = VK (i);
    }
  }

  maxderivinvok = 0;
  UpdateVKnots();
}

//=======================================================================
//function : SetVKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetVKnot
(const Standard_Integer VIndex,
 const Standard_Real    K,
 const Standard_Integer M)
{
  IncreaseVMultiplicity (VIndex, M);
  SetVKnot (VIndex, K);
}

//=======================================================================
//function : InsertUKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::InsertUKnot
(const Standard_Real    U,
 const Standard_Integer M,
 const Standard_Real    ParametricTolerance,
 const Standard_Boolean Add)
{
  TColStd_Array1OfReal k(1,1);
  k(1) = U;
  TColStd_Array1OfInteger m(1,1);
  m(1) = M;
  InsertUKnots(k,m,ParametricTolerance,Add);
}

//=======================================================================
//function : InsertVKnot
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::InsertVKnot
(const Standard_Real    V,
 const Standard_Integer M,
 const Standard_Real    ParametricTolerance,
 const Standard_Boolean Add)
{
  TColStd_Array1OfReal k(1,1);
  k(1) = V;
  TColStd_Array1OfInteger m(1,1);
  m(1) = M;
  InsertVKnots(k,m,ParametricTolerance,Add);
}

//=======================================================================
//function : IncrementUMultiplicity
//purpose  : 
//=======================================================================

void  asiAlgo_BSplineSurface::IncrementUMultiplicity
(const Standard_Integer FromI1,
 const Standard_Integer ToI2,
 const Standard_Integer Step)
{
  Handle(TColStd_HArray1OfReal) tk = uknots;
  TColStd_Array1OfReal k( (uknots->Array1())(FromI1), FromI1, ToI2);
  TColStd_Array1OfInteger m( FromI1, ToI2) ;
  m.Init(Step);
  InsertUKnots( k, m, Epsilon(1.));
}

//=======================================================================
//function : IncrementVMultiplicity
//purpose  : 
//=======================================================================

void  asiAlgo_BSplineSurface::IncrementVMultiplicity
(const Standard_Integer FromI1,
 const Standard_Integer ToI2,
 const Standard_Integer Step)
{
  Handle(TColStd_HArray1OfReal) tk = vknots;
  TColStd_Array1OfReal k( (vknots->Array1())(FromI1), FromI1, ToI2);

  TColStd_Array1OfInteger m( FromI1, ToI2) ;
  m.Init(Step);
  
  InsertVKnots( k, m, Epsilon(1.));
}

//=======================================================================
//function : UpdateUKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UpdateUKnots()
{

  Standard_Integer MaxKnotMult = 0;
  BSplCLib::KnotAnalysis (udeg, uperiodic,
		uknots->Array1(), 
		umults->Array1(), 
		uknotSet, MaxKnotMult);
  
  if (uknotSet == GeomAbs_Uniform && !uperiodic)  {
    ufknots = uknots;
  }
  else {
    ufknots = new TColStd_HArray1OfReal 
      (1, BSplCLib::KnotSequenceLength(umults->Array1(),udeg,uperiodic));

    BSplCLib::KnotSequence (uknots->Array1(), 
			    umults->Array1(),
			    udeg,uperiodic,
			    ufknots->ChangeArray1());
  }
  
  if (MaxKnotMult == 0)  Usmooth = GeomAbs_CN;
  else {
    switch (udeg - MaxKnotMult) {
    case 0 :   Usmooth = GeomAbs_C0;   break;
    case 1 :   Usmooth = GeomAbs_C1;   break;
    case 2 :   Usmooth = GeomAbs_C2;   break;
    case 3 :   Usmooth = GeomAbs_C3;   break;
      default :  Usmooth = GeomAbs_C3;   break;
    }
  }
}

//=======================================================================
//function : UpdateVKnots
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::UpdateVKnots()
{
  Standard_Integer MaxKnotMult = 0;
  BSplCLib::KnotAnalysis (vdeg, vperiodic,
		vknots->Array1(), 
		vmults->Array1(), 
		vknotSet, MaxKnotMult);
  
  if (vknotSet == GeomAbs_Uniform && !vperiodic)  {
    vfknots = vknots;
  }
  else {
    vfknots = new TColStd_HArray1OfReal 
      (1, BSplCLib::KnotSequenceLength(vmults->Array1(),vdeg,vperiodic));

    BSplCLib::KnotSequence (vknots->Array1(), 
			    vmults->Array1(),
			    vdeg,vperiodic,
			    vfknots->ChangeArray1());
  }
  
  if (MaxKnotMult == 0)  Vsmooth = GeomAbs_CN;
  else {
    switch (vdeg - MaxKnotMult) {
    case 0 :   Vsmooth = GeomAbs_C0;   break;
    case 1 :   Vsmooth = GeomAbs_C1;   break;
    case 2 :   Vsmooth = GeomAbs_C2;   break;
    case 3 :   Vsmooth = GeomAbs_C3;   break;
      default :  Vsmooth = GeomAbs_C3;   break;
    }
  }
}


//=======================================================================
//function : Normalizes the parameters if the curve is periodic
//purpose  : that is compute the cache so that it is valid
//=======================================================================

void asiAlgo_BSplineSurface::PeriodicNormalization
(Standard_Real&  Uparameter, 
 Standard_Real&  Vparameter) const 
{
  Standard_Real Period, aMaxVal, aMinVal;
  
  if (uperiodic) {
    aMaxVal = ufknots->Value(ufknots->Upper() - udeg);
    aMinVal = ufknots->Value (udeg + 1);
    Standard_Real eps = Abs(Epsilon(Uparameter));
    Period =  aMaxVal - aMinVal;

    if(Period <= eps) 
      throw Standard_OutOfRange("asiAlgo_BSplineSurface::PeriodicNormalization: Uparameter is too great number");

    Standard_Boolean isLess, isGreater;
    isLess = aMinVal - Uparameter > 0;
    isGreater = Uparameter - aMaxVal > 0;
    if (isLess || isGreater) {
      Standard_Real aDPar, aNbPer;
      aDPar = (isLess) ? (aMaxVal - Uparameter) : (aMinVal - Uparameter);
      modf(aDPar / Period, &aNbPer);
      Uparameter += aNbPer * Period;
    }
  }
  if (vperiodic) {
    aMaxVal = vfknots->Value(vfknots->Upper() - vdeg);
    aMinVal = vfknots->Value (vdeg + 1);
    Standard_Real eps = Abs(Epsilon(Vparameter));
    Period = aMaxVal - aMinVal;

    if(Period <= eps) 
      throw Standard_OutOfRange("asiAlgo_BSplineSurface::PeriodicNormalization: Vparameter is too great number");

    Standard_Boolean isLess, isGreater;
    isLess = aMinVal - Vparameter > 0;
    isGreater = Vparameter - aMaxVal > 0;
    if (isLess || isGreater) {
      Standard_Real aDPar, aNbPer;
      aDPar = (isLess) ? (aMaxVal - Vparameter) : (aMinVal - Vparameter);
      modf(aDPar / Period, &aNbPer);
      Vparameter += aNbPer * Period;
    }
  }
}

//=======================================================================
//function : SetWeight
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetWeight (const Standard_Integer UIndex,
				     const Standard_Integer VIndex,
				     const Standard_Real    Weight)
{
  if (Weight <= gp::Resolution())
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetWeight: Weight too small");
  TColStd_Array2OfReal & Weights = weights->ChangeArray2();
  if (UIndex < 1 || UIndex > Weights.ColLength() ||
      VIndex < 1 || VIndex > Weights.RowLength() ) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetWeight: Index and #pole mismatch");
  }
  Weights (UIndex+Weights.LowerRow()-1, VIndex+Weights.LowerCol()-1) = Weight;
  Rational(Weights, urational, vrational);
}

//=======================================================================
//function : SetWeightCol
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetWeightCol
(const Standard_Integer       VIndex, 
 const TColStd_Array1OfReal&  CPoleWeights)
{
  TColStd_Array2OfReal & Weights = weights->ChangeArray2();   
  if (VIndex < 1 || VIndex > Weights.RowLength()) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetWeightCol: Index and #pole mismatch");
  }
  if (CPoleWeights.Lower() < 1 || 
      CPoleWeights.Lower() > Weights.ColLength() ||
      CPoleWeights.Upper() < 1 ||
      CPoleWeights.Upper() > Weights.ColLength()  ) {
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetWeightCol: invalid array dimension");
  }
  Standard_Integer I = CPoleWeights.Lower();
  while (I <= CPoleWeights.Upper()) {
    if (CPoleWeights(I) <= gp::Resolution()) { 
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetWeightCol: Weight too small");
    }
    Weights (I+Weights.LowerRow()-1, VIndex+Weights.LowerCol()-1) = 
      CPoleWeights (I);
    I++;
  }
  // Verifie si c'est rationnel
  Rational(Weights, urational, vrational);
}

//=======================================================================
//function : SetWeightRow
//purpose  : 
//=======================================================================

void asiAlgo_BSplineSurface::SetWeightRow
(const Standard_Integer       UIndex, 
 const TColStd_Array1OfReal&  CPoleWeights)
{
  TColStd_Array2OfReal & Weights = weights->ChangeArray2();   
  if (UIndex < 1 || UIndex > Weights.ColLength()) {
    throw Standard_OutOfRange("asiAlgo_BSplineSurface::SetWeightRow: Index and #pole mismatch");
  }
  if (CPoleWeights.Lower() < 1 ||
      CPoleWeights.Lower() > Weights.RowLength() ||
      CPoleWeights.Upper() < 1 ||
      CPoleWeights.Upper() > Weights.RowLength()  ) {
    
    throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetWeightRow: invalid array dimension");
  }
  Standard_Integer I = CPoleWeights.Lower();

  while (I <= CPoleWeights.Upper()) {
    if (CPoleWeights(I)<=gp::Resolution()) {
      throw Standard_ConstructionError("asiAlgo_BSplineSurface::SetWeightRow: Weight too small");
    }
    Weights (UIndex+Weights.LowerRow()-1, I+Weights.LowerCol()-1) = 
      CPoleWeights (I);
    I++;
  }
  // Verifie si c'est rationnel
  Rational(Weights, urational, vrational);
}

//=======================================================================
//function : DumpJson
//purpose  : 
//=======================================================================
void asiAlgo_BSplineSurface::DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)
  OCCT_DUMP_BASE_CLASS (theOStream, theDepth, Geom_BoundedSurface)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, urational)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vrational)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, uperiodic)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vperiodic)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, uknotSet)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vknotSet)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, Usmooth)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, Vsmooth)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, udeg)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vdeg)

  if (!poles.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, poles->Size())
  if (!weights.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, weights->Size())
  if (!ufknots.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, ufknots->Size())
  if (!vfknots.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vfknots->Size())

  if (!uknots.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, uknots->Size())
  if (!vknots.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vknots->Size())
  if (!umults.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, umults->Size())
  if (!vmults.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vmults->Size())

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, umaxderivinv)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, vmaxderivinv)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, maxderivinvok)
}
