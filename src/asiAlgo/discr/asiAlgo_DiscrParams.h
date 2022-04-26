//-----------------------------------------------------------------------------
// Created on: 26 April 2022
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

#ifndef asiAlgo_DiscrParams_HeaderFile
#define asiAlgo_DiscrParams_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

#include <Standard_TypeDef.hxx>
#include <Precision.hxx>
#ifdef DEBUG_MeshParameters
#include <Standard_NoSuchObject.hxx>
#endif

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// This class describes the discretization parameters.
class Params
{
 public:
  // ---------- INITIALIZATION ----------

  Params () :
    myMinElemSize     (0.001),
    myDeviationAngle  (1e100),
    myMaxElemSize     (1e100),
    myDeflection      (1e100),
    myAspectRatio     (0.),
    isDeviationAngle  (false),
    isMaxElemSize     (false),
    isDeflection      (false),
    isAspectRatio     (false),
    myIsCheckDelaunay (false)
  {}

  void SetMinElemSize (const double aMinElemSize)
  { myMinElemSize = aMinElemSize; }

  void SetDeviationAngle (const double aDeviationAngle)
  {
    myDeviationAngle = aDeviationAngle;
    isDeviationAngle = true;
    Update();
  }

  void DisableDeviationAngle()
  {
    isDeviationAngle = false;
  }

  void SetAspectRatio (const double anAspectRatio)
  {
    myAspectRatio = anAspectRatio;
    isAspectRatio = true;
  }

  void DisableAspectRatio()
  {
    isAspectRatio = false;
  }

  void SetMaxElemSize (const double aMaxElemSize)
  { 
    myMaxElemSize = Max(Precision::Confusion(), aMaxElemSize);
    isMaxElemSize = true;
  }

  void DisableMaxElemSize ()
  { isMaxElemSize = false; }

  void SetDeflection (const double aDeflection)
  { 
    myDeflection = Max(Precision::Confusion(), aDeflection);
    isDeflection = true;
  }

  void DisableDeflection ()
  { isDeflection = false; }

  void SetCheckDelaunay(const bool isToCheck)
  {
    myIsCheckDelaunay = isToCheck;
  }

  // ---------- QUERYING -------------

  double MinElemSize         () const    { return myMinElemSize; }

  double AspectRatio         () const    { return myAspectRatio; }

  bool IsAspectRatio    () const    { return isAspectRatio; }

  bool IsDeviationAngle () const    { return isDeviationAngle; }

  double DeviationAngle      () const
  {
#ifdef DEBUG_MeshParameters
    if (!isDeviationAngle)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::DeviationAngle");
#endif
    return myDeviationAngle;
  }

  double CosAngle            () const
  {
#ifdef DEBUG_MeshParameters
    if (!isDeviationAngle)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::CosAngle");
#endif
    return myCosAngle;
  }

  double SinAngle            () const
  {
#ifdef DEBUG_MeshParameters
    if (!isDeviationAngle)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::SinAngle");
#endif
    return mySinAngle;
  }

  double SinHalfAngle        () const
  {
#ifdef DEBUG_MeshParameters
    if (!isDeviationAngle)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::SinHalfAngle");
#endif
    return mySinHalfAngle;
  }

  bool IsMaxElemSize    () const    { return isMaxElemSize; }

  double MaxElemSize         () const
  {
#ifdef DEBUG_MeshParameters
    if (!isMaxElemSize)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::MaxElemSize");
#endif
    return myMaxElemSize;
  }

  bool IsDeflection     () const    { return isDeflection; }

  double Deflection          () const
  {
#ifdef DEBUG_MeshParameters
    if (!isDeflection)
      throw Standard_NoSuchObject("asiFaceter_MeshParameters::Deflection");
#endif
    return myDeflection;
  }

  bool IsCheckDelaunay() const
  {
    return myIsCheckDelaunay;
  }

 private:

  asiAlgo_EXPORT void Update();

  // ---------- PRIVATE FIELDS ----------

  double myMinElemSize;
  double myDeviationAngle;
  double myMaxElemSize;
  double myDeflection;
  double myCosAngle;
  double mySinAngle;
  double mySinHalfAngle;
  double myAspectRatio;
  bool   isDeviationAngle;
  bool   isMaxElemSize;
  bool   isDeflection;
  bool   isAspectRatio;
  bool   myIsCheckDelaunay;
};

}
}

#endif
