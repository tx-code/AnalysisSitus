//-----------------------------------------------------------------------------
// Created on: 11 December 2022
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

#ifndef asiAlgo_PartBodyType_h
#define asiAlgo_PartBodyType_h

// asiAlgo includes
#include <asiAlgo_JsonDict.h>

// Active Data includes
#include <ActAPI_IPlotter.h>

//-----------------------------------------------------------------------------

//! Recognized part types. The type is resolved at the level of a single part
//! body as there could be compound parts which are not recognizable unless
//! exploded to the primitive bodies, such as solids.
enum asiAlgo_PartBodyType
{
  PartBodyType_Unrecognized = 0, //!< Pending for recognition.
  PartBodyType_FlatShape,        //!< Sheet metal body without folds.
  PartBodyType_FoldedSheetMetal, //!< Folded sheet metal body.
  PartBodyType_RectTube,         //!< Rectangular tube body.
  PartBodyType_CylTube,          //!< Cylindrical tube body.
  PartBodyType_OtherTube,        //!< Other type of tube.
  PartBodyType_Profile,          //!< Sheet metal profile.
  PartBodyType_CncMilling,       //!< CNC milled body.
  PartBodyType_CncLathe,         //!< CNC lathed body.
  PartBodyType_CncLatheMilling   //!< CNC lathed body, (optionally) combined with milling.
};

//-----------------------------------------------------------------------------

//! Auxiliary functions for working with body types.
namespace asiAlgo_PartBodyTypeUtils
{
  //! Checks if the passed enum encodes laser cutting process.
  //! \param[in] type object type to check.
  //! \return true/false.
  inline bool IsLaserCutting(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_FlatShape:
      case PartBodyType_FoldedSheetMetal:
      case PartBodyType_RectTube:
      case PartBodyType_CylTube:
      case PartBodyType_OtherTube:
      case PartBodyType_Profile:
        return true;
      case PartBodyType_Unrecognized:
      case PartBodyType_CncMilling:
      case PartBodyType_CncLathe:
      case PartBodyType_CncLatheMilling:
        return false;
      default: break;
    }

    return false;
  }

  //! Checks if the passed enum encodes CNC process.
  //! \param[in] type object type to check.
  //! \return true/false.
  inline bool IsCnc(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_FlatShape:
      case PartBodyType_FoldedSheetMetal:
      case PartBodyType_RectTube:
      case PartBodyType_CylTube:
      case PartBodyType_OtherTube:
      case PartBodyType_Profile:
      case PartBodyType_Unrecognized:
        return false;
      case PartBodyType_CncMilling:
      case PartBodyType_CncLathe:
      case PartBodyType_CncLatheMilling:
        return true;
      default: break;
    }

    return false;
  }

  //! Checks if the passed enum encodes a sheet metal.
  //! \param[in] type object type to check.
  //! \return true/false.
  inline bool IsSheetMetal(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_FlatShape:
      case PartBodyType_FoldedSheetMetal:
        return true;
      case PartBodyType_RectTube:
      case PartBodyType_CylTube:
      case PartBodyType_Unrecognized:
      case PartBodyType_OtherTube:
      case PartBodyType_Profile:
      case PartBodyType_CncMilling:
      case PartBodyType_CncLathe:
      case PartBodyType_CncLatheMilling:
        return false;
      default: break;
    }

    return false;
  }

  //! Checks if the passed enum encodes a tube.
  //! \param[in] type object type to check.
  //! \return true/false.
  inline bool IsTube(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_RectTube:
      case PartBodyType_CylTube:
        return true;
      case PartBodyType_Unrecognized:
      case PartBodyType_FlatShape:
      case PartBodyType_FoldedSheetMetal:
      case PartBodyType_OtherTube:
      case PartBodyType_Profile:
      case PartBodyType_CncMilling:
      case PartBodyType_CncLathe:
      case PartBodyType_CncLatheMilling:
        return false;
      default: break;
    }

    return false;
  }

  //! Returns body type name.
  //! \param[in] type body type in question.
  //! \return const char pointer to the body type name.
  inline const char* GetTypeName(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_Unrecognized:     return PropVal_Type_Unrecognized;
      case PartBodyType_FlatShape:        return PropVal_Type_FlatShape;
      case PartBodyType_FoldedSheetMetal: return PropVal_Type_FoldedSheetMetal;
      case PartBodyType_RectTube:         return PropVal_Type_RectTube;
      case PartBodyType_CylTube:          return PropVal_Type_CylTube;
      case PartBodyType_OtherTube:        return PropVal_Type_OtherTube;
      case PartBodyType_Profile:          return PropVal_Type_Profile;
      case PartBodyType_CncMilling:       return PropVal_Type_CncMilling;
      case PartBodyType_CncLathe:         return PropVal_Type_CncLathe;
      case PartBodyType_CncLatheMilling:  return PropVal_Type_CncLatheMilling;
      default: break;
    }

    return PropVal_Type_Unknown;
  }

  //! Returns body type by name.
  //! \param[in] name body type name in question.
  //! \return body type.
  inline asiAlgo_PartBodyType GetTypeByName(const std::string& name)
  {
    if ( name == PropVal_Type_Unrecognized )
      return PartBodyType_Unrecognized;
    if ( name == PropVal_Type_FlatShape )
      return PartBodyType_FlatShape;
    if ( name == PropVal_Type_FoldedSheetMetal )
      return PartBodyType_FoldedSheetMetal;
    if ( name == PropVal_Type_RectTube )
      return PartBodyType_RectTube;
    if ( name == PropVal_Type_CylTube )
      return PartBodyType_CylTube;
    if ( name == PropVal_Type_OtherTube )
      return PartBodyType_OtherTube;
    if ( name == PropVal_Type_Profile )
      return PartBodyType_Profile;
    if ( name == PropVal_Type_CncMilling )
      return PartBodyType_CncMilling;
    if ( name == PropVal_Type_CncLathe )
      return PartBodyType_CncLathe;
    if ( name == PropVal_Type_CncLatheMilling )
      return PartBodyType_CncLatheMilling;

    return PartBodyType_Unrecognized;
  }

  //! Returns body type color.
  //! \param[in] type body type in question.
  //! \return the associated color.
  inline ActAPI_Color GetTypeColor(const asiAlgo_PartBodyType type)
  {
    switch ( type )
    {
      case PartBodyType_Unrecognized:     return ActAPI_Color(255./255, 130./255, 130./255, Quantity_TOC_RGB);
      case PartBodyType_FlatShape:        return ActAPI_Color(165./255, 222./255, 230./255, Quantity_TOC_RGB);
      case PartBodyType_FoldedSheetMetal: return ActAPI_Color(190./255, 230./255, 140./255, Quantity_TOC_RGB);
      case PartBodyType_RectTube:         return ActAPI_Color(160./255, 130./255, 250./255, Quantity_TOC_RGB);
      case PartBodyType_CylTube:          return ActAPI_Color(230./255, 180./255, 250./255, Quantity_TOC_RGB);
      case PartBodyType_OtherTube:        return ActAPI_Color(255./255, 190./255, 117./255, Quantity_TOC_RGB);
      case PartBodyType_Profile:          return ActAPI_Color(170./255, 190./255, 200./255, Quantity_TOC_RGB);
      case PartBodyType_CncMilling:       return ActAPI_Color(  0./255, 228./255,   0./255, Quantity_TOC_RGB);
      case PartBodyType_CncLathe:         return ActAPI_Color(  0./255, 228./255, 100./255, Quantity_TOC_RGB);
      case PartBodyType_CncLatheMilling:  return ActAPI_Color(  0./255, 228./255, 228./255, Quantity_TOC_RGB);
      default: break;
    }

    return ActAPI_Color(155./255, 0., 0., Quantity_TOC_RGB);
  }

} // asiAlgo_PartBodyTypeUtils namespace.

#endif
