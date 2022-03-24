//-----------------------------------------------------------------------------
// Created on: 24 March 2022
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

// Own include
#include <asiAlgo_BaseSTEP.h>

// Standard includes
#include <map>

//-----------------------------------------------------------------------------

double asiAlgo_BaseSTEP::fromSiName(const TCollection_AsciiString& unitStr) const
{
  static const std::map< TCollection_AsciiString, double >
    prefixMap =
  {
    { "yocto", 1.0e-24 },
    { "zepto", 1.0e-21 },
    { "atto",  1.0e-18 },
    { "femto", 1.0e-15 },
    { "pico",  1.0e-12 },
    { "nano",  1.0e-9 },
    { "micro", 1.0e-6 },
    { "milli", 1.0e-3 },
    { "centi", 1.0e-2 },
    { "deci",  1.0e-1 },
    { "deca",  1.0e1 },
    { "hecto", 1.0e2 },
    { "kilo",  1.0e3 },
    { "mega",  1.0e6 },
    { "giga",  1.0e9 },
    { "tera",  1.0e12 },
    { "pera",  1.0e15 },
    { "exa",   1.0e18 },
    { "zetta", 1.0e21 },
    { "yotta", 1.0e24 }
  };

  static const std::map< TCollection_AsciiString, double >
    baseMap =
  {
    { "angstrom",   1.0e-10 },
    { "micron",     1.0e-6 },
    { "twip",       0.0000176389 },
    { "thou",       0.0000254 },
    { "mil",        0.0000254 },
    { "barleycorn", 0.0084667 },
    { "inch",       0.0254 },
    { "hand",       0.1016 },
    { "foot",       0.3048 },
    { "feet",       0.3048 },
    { "yard",       0.9144 },
    { "metre",      1.0 },
    { "chain",      20.1168 },
    { "furlong",    201.168 },
    { "mile",       1609.344 },
    { "league",     4828.032 }
  };

  // Prepare input value.
  TCollection_AsciiString unitStrLower( unitStr );
  unitStrLower.LowerCase();

  // Final scaling factor.
  double scaleFactor = 1.0;

  // Find base unit name.
  for ( const auto& tuple : baseMap )
  {
    if ( unitStrLower.EndsWith(tuple.first) )
    {
      scaleFactor *= tuple.second;
      break;
    }
  }

  // Apply prefix modifier if any.
  for ( const auto& tuple : prefixMap)
  {
    if ( unitStrLower.StartsWith(tuple.first) )
    {
      scaleFactor *= tuple.second;
      break;
    }
  }

  // Now we have a scale factor for conversation to metres.
  // But we want to use millimetres.
  return scaleFactor * 1000.0;
}
