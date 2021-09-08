//-----------------------------------------------------------------------------
// Created on: April 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActAPI_IPlotter.h>

//-----------------------------------------------------------------------------

//! Returns a "good" color index for the passed "some" index. This method
//! attempts to choose the least continuous colors, so that to make them
//! highly distinguishable.
//! \param index [in] your "some" index.
//! \return our "good" color index for you.
Quantity_NameOfColor ActAPI_IPlotter::Color_Sparse(const int index)
{
  switch ( index )
  {
    case 0:  return Quantity_NOC_RED;
    case 1:  return Quantity_NOC_GREEN;
    case 2:  return Quantity_NOC_BLUE1;
    case 3:  return Quantity_NOC_YELLOW;
    case 4:  return Quantity_NOC_GOLD;
    case 5:  return Quantity_NOC_ORANGE;
    case 6:  return Quantity_NOC_MAGENTA1;
    case 7:  return Quantity_NOC_SNOW;
    case 8:  return Quantity_NOC_SKYBLUE;
    case 9:  return Quantity_NOC_PINK;
    case 10: return Quantity_NOC_VIOLET;
    case 11: return Quantity_NOC_CYAN1;
    case 12: return Quantity_NOC_SPRINGGREEN;
    case 13: return Quantity_NOC_TOMATO3;
    case 14: return Quantity_NOC_DARKVIOLET;
    case 15: return Quantity_NOC_DODGERBLUE3;
    case 16: return Quantity_NOC_IVORY;
    case 17: return Quantity_NOC_ROYALBLUE;
    default: break;
  };

  return (Quantity_NameOfColor) index;
}

//-----------------------------------------------------------------------------

//! Destructor.
ActAPI_IPlotter::~ActAPI_IPlotter()
{}
