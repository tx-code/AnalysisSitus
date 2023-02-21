//-----------------------------------------------------------------------------
// Created on: 29 October 2021
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
#include <asiAlgo_ConvertCanonicalSummary.h>
#include <asiAlgo_JsonDict.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

#if defined USE_RAPIDJSON
  // Rapidjson includes
  #include <rapidjson/document.h>

  typedef rapidjson::Document::Array     t_jsonArray;
  typedef rapidjson::Document::ValueType t_jsonValue;
  typedef rapidjson::Document::Object    t_jsonObject;
#endif

//-----------------------------------------------------------------------------

namespace
{
  bool AreEqual(const tl::optional< std::pair<int, int> >& a,
                const tl::optional< std::pair<int, int> >& b)
  {
    if ( !a.has_value() && !b.has_value() )
      return true;

    if ( !a.has_value() && b.has_value() )
      return false;

    if ( a.has_value() && !b.has_value() )
      return false;

    if ( (a->first == b->first) && (a->second == b->second) )
      return true;

    return false;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalSummary::IsEqual(const asiAlgo_ConvertCanonicalSummary& other) const
{
  // Validity flag is optional.
  if ( isValid.has_value() && other.isValid.has_value() && (*isValid != *isValid) )
    return false;

  if ( !::AreEqual(nbSurfBezier,     other.nbSurfBezier)     ) return false;
  if ( !::AreEqual(nbSurfSpl,        other.nbSurfSpl)        ) return false;
  if ( !::AreEqual(nbSurfConical,    other.nbSurfConical)    ) return false;
  if ( !::AreEqual(nbSurfCyl,        other.nbSurfCyl)        ) return false;
  if ( !::AreEqual(nbSurfOffset,     other.nbSurfOffset)     ) return false;
  if ( !::AreEqual(nbSurfSph,        other.nbSurfSph)        ) return false;
  if ( !::AreEqual(nbSurfLinExtr,    other.nbSurfLinExtr)    ) return false;
  if ( !::AreEqual(nbSurfOfRevol,    other.nbSurfOfRevol)    ) return false;
  if ( !::AreEqual(nbSurfToroidal,   other.nbSurfToroidal)   ) return false;
  if ( !::AreEqual(nbSurfPlane,      other.nbSurfPlane)      ) return false;
  if ( !::AreEqual(nbCurveBezier,    other.nbCurveBezier)    ) return false;
  if ( !::AreEqual(nbCurveSpline,    other.nbCurveSpline)    ) return false;
  if ( !::AreEqual(nbCurveCircle,    other.nbCurveCircle)    ) return false;
  if ( !::AreEqual(nbCurveEllipse,   other.nbCurveEllipse)   ) return false;
  if ( !::AreEqual(nbCurveHyperbola, other.nbCurveHyperbola) ) return false;
  if ( !::AreEqual(nbCurveLine,      other.nbCurveLine)      ) return false;
  if ( !::AreEqual(nbCurveOffset,    other.nbCurveOffset)    ) return false;
  if ( !::AreEqual(nbCurveParabola,  other.nbCurveParabola)  ) return false;

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonicalSummary::FromJSON(void*                            pJsonGenericObj,
                                               asiAlgo_ConvertCanonicalSummary& ccSummary)
{
#if defined USE_RAPIDJSON
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Iterate over the props.
  for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
        mit != pJsonObj->MemberEnd(); mit++ )
  {
    std::string prop( mit->name.GetString() );

    if ( prop == asiPropName_ExtrasCanRecIsValid )
    {
      if ( !mit->value.IsNull() )
        ccSummary.isValid = mit->value.GetBool();
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfBezier )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfBezier);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfSpl )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfSpl);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfConical )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfConical);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfCyl )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfCyl);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfOffset )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfOffset);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfSph )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfSph);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfLinExtr )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfLinExtr);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfOfRevol )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfOfRevol);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfToroidal )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfToroidal);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecSurfPlane )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbSurfPlane);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveBezier )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveBezier);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveSpline )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveSpline);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveCircle )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveCircle);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveEllipse )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveEllipse);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveHyperbola )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveHyperbola);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveLine )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveLine);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveOffset )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveOffset);
      }
    }

    else if ( prop == asiPropName_ExtrasCanRecCurveParabola )
    {
      if ( !mit->value.IsNull() )
      {
        t_jsonArray arr = mit->value.GetArray();
        asiAlgo_Utils::Json::ReadPair(&arr, ccSummary.nbCurveParabola);
      }
    }
  }
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonicalSummary::ToJSON(const asiAlgo_ConvertCanonicalSummary& ccSummary,
                                             const int                              indent,
                                             std::ostream&                          out)
{
  std::string ws(indent, ' ');
  std::string nl = "\n" + ws;

  /* Dump props */

  if ( ccSummary.isValid.has_value() )
    out << nl << "\"" << asiPropName_ExtrasCanRecIsValid << "\": " << (*ccSummary.isValid ? "true" : "false");
  else
    out << nl << "\"" << asiPropName_ExtrasCanRecIsValid << "\": " << "null";

  if ( ccSummary.nbSurfBezier.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfBezier     << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfBezier);
  //
  if ( ccSummary.nbSurfSpl.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfSpl        << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfSpl);
  //
  if ( ccSummary.nbSurfConical.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfConical    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfConical);
  //
  if ( ccSummary.nbSurfCyl.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfCyl        << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfCyl);
  //
  if ( ccSummary.nbSurfOffset.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfOffset     << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfOffset);
  //
  if ( ccSummary.nbSurfSph.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfSph        << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfSph);
  //
  if ( ccSummary.nbSurfLinExtr.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfLinExtr    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfLinExtr);
  //
  if ( ccSummary.nbSurfOfRevol.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfOfRevol    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfOfRevol);
  //
  if ( ccSummary.nbSurfToroidal.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfToroidal   << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfToroidal);
  //
  if ( ccSummary.nbSurfPlane.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecSurfPlane      << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbSurfPlane);
  //
  if ( ccSummary.nbCurveBezier.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveBezier    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveBezier);
  //
  if ( ccSummary.nbCurveSpline.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveSpline    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveSpline);
  //
  if ( ccSummary.nbCurveCircle.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveCircle    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveCircle);
  //
  if ( ccSummary.nbCurveEllipse.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveEllipse   << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveEllipse);
  //
  if ( ccSummary.nbCurveHyperbola.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveHyperbola << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveHyperbola);
  //
  if ( ccSummary.nbCurveLine.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveLine      << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveLine);
  //
  if ( ccSummary.nbCurveOffset.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveOffset    << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveOffset);
  //
  if ( ccSummary.nbCurveParabola.has_value() )
    out << "," << nl << "\"" << asiPropName_ExtrasCanRecCurveParabola  << "\": " << asiAlgo_Utils::Json::FromPair(ccSummary.nbCurveParabola);
}
