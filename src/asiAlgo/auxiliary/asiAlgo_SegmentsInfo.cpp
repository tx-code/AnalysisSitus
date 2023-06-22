//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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
#include <asiAlgo_SegmentsInfo.h>

// asiAlgo includes
#include <asiAlgo_JsonDict.h>
#include <asiAlgo_Utils.h>

// STL includes
#include <ostream>

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfo::asiAlgo_SegmentsInfo(const double       _id,
                                           const std::string& _type,
                                           const double       _cuttingLength)
  : id( _id ),
    type( _type ),
    cuttingLength( _cuttingLength )
{}

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfo::asiAlgo_SegmentsInfo()
  : asiAlgo_SegmentsInfo( -1, "undefined", 0.0 )
{}

//-----------------------------------------------------------------------------

bool asiAlgo_SegmentsInfo::IsEqual(const asiAlgo_SegmentsInfo& info,
                                   const double                linToler,
                                   const double                angTolerDeg) const
{
  // ID.
  if ( this->id != info.id )
  {
    return false;
  }

  // Type.
  if ( this->type != info.type )
  {
    return false;
  }

  // Cutting length.
  if ( Abs( this->cuttingLength - info.cuttingLength ) > linToler )
  {
    return false;
  }

  // Next segment ID.
  if ( this->nextSegment.has_value() != info.nextSegment.has_value() )
  {
    return false;
  }

  if ( this->nextSegment.has_value() &&
       this->nextSegment != info.nextSegment )
  {
    return false;
  }

  // Angle to next segment.
  if ( this->angleToNextSegment.has_value() != info.angleToNextSegment.has_value() )
  {
    return false;
  }

  if ( this->angleToNextSegment.has_value() &&
       Abs( *this->angleToNextSegment - *info.angleToNextSegment ) > angTolerDeg )
  {
    return false;
  }

  // Radius.
  if ( this->radius.has_value() != info.radius.has_value() )
  {
    return false;
  }

  if ( this->radius.has_value() &&
       Abs( *this->radius - *info.radius ) > linToler )
  {
    return false;
  }

  // Angle.
  if ( this->angle.has_value() != info.angle.has_value() )
  {
    return false;
  }

  if ( this->angle.has_value() &&
       Abs( *this->angle - *info.angle ) > angTolerDeg )
  {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_SegmentsInfo::AreEqual(const asiAlgo_SegmentsInfoVec& v1,
                                    const asiAlgo_SegmentsInfoVec& v2,
                                    const double                   linToler,
                                    const double                   angTolerDeg)
{
  // Compare vectors size.
  if ( v1.size() != v2.size() )
  {
    return false;
  }

  // Compare pairs of elements.
  for ( int i = 0; i < v1.size(); ++i )
  {
    if ( !v1[i].IsEqual( v2[i], linToler, angTolerDeg ) )
    {
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_SegmentsInfo::FromJSON(void*                 pJsonGenericObj,
                                    asiAlgo_SegmentsInfo& info)
{
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Iterate members of the fillet chain object.
  for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
        mit != pJsonObj->MemberEnd(); mit++ )
  {
    std::string prop( mit->name.GetString() );

    // ID.
    if ( prop == asiPropName_Id )
    {
      if ( !mit->value.IsNull() )
        info.id = mit->value.GetInt();
    }

    // Type.
    else if ( prop == asiPropName_Type )
    {
      if ( !mit->value.IsNull() )
        info.type = mit->value.GetString();
    }

    // Cutting length.
    else if ( prop == asiPropName_CuttingLength )
    {
      if ( !mit->value.IsNull() )
        info.cuttingLength = mit->value.GetDouble();
    }

    // Next segment ID.
    else if ( prop == asiPropName_NextSegment )
    {
      if ( !mit->value.IsNull() )
        info.nextSegment = mit->value.GetInt();
    }

    // Angle to next segment.
    else if ( prop == asiPropName_AngleToNextSegment )
    {
      if ( !mit->value.IsNull() )
        info.angleToNextSegment = mit->value.GetDouble();
    }

    // Radius.
    else if ( prop == asiPropName_Radius )
    {
      if ( !mit->value.IsNull() )
        info.radius = mit->value.GetDouble();
    }

    // Angle.
    else if ( prop == asiPropName_Angle )
    {
      if ( !mit->value.IsNull() )
        info.angle = mit->value.GetDouble();
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_SegmentsInfo::FromJSON(void*                    pJsonGenericObj,
                                    asiAlgo_SegmentsInfoVec& infoVec)
{
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Loop over the array of results for each segment information block.
  for ( t_jsonValue::ValueIterator bendIt = pJsonObj->Begin();
        bendIt != pJsonObj->End(); bendIt++ )
  {
    t_jsonValue segObj = bendIt->GetObject();

    asiAlgo_SegmentsInfo segInfo;
    asiAlgo_SegmentsInfo::FromJSON( &segObj, segInfo );

    infoVec.push_back( segInfo );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_SegmentsInfo::ToJSON(const asiAlgo_SegmentsInfo& info,
                                  const int                   indent,
                                  std::ostream&               out,
                                  const bool                  pureJSON)
{
  std::string ws = pureJSON ? "" : std::string(indent, ' ');
  std::string nl = pureJSON ? "" : "\n" + ws;
  std::string qt = "\"";

  {
    // ID.
    out << nl << qt << asiPropName_Id << qt << ": " << info.id;

    // Type.
    out << "," << nl << qt << asiPropName_Type << qt << ": " << qt << info.type << qt;

    // Cutting length.
    out << "," << nl << qt << asiPropName_CuttingLength << qt << ": " << info.cuttingLength;

    // Next segment ID.
    if ( info.nextSegment.has_value() )
    {
      std::string nextStr = asiAlgo_Utils::Str::ToString( *info.nextSegment );

      out << "," << nl << qt << asiPropName_NextSegment << qt << ": " << nextStr;

      // Angle to next segment.
      std::string angleToNextStr = info.nextSegment.has_value() ? asiAlgo_Utils::Str::ToString( *info.angleToNextSegment )
                                                                : "null";
      out << "," << nl << qt << asiPropName_AngleToNextSegment << qt << ": " << angleToNextStr;
    }

    // For circular curves.
    if ( info.radius.has_value() )
    {
      std::string rStr = asiAlgo_Utils::Str::ToString( *info.radius );

      out << "," << nl << qt << asiPropName_Radius << qt << ": " << rStr;

      // Angle.
      std::string aStr = info.angle.has_value() ? asiAlgo_Utils::Str::ToString( *info.angle )
                                                : "null";

      out << "," << nl << qt << asiPropName_Angle << qt << ": " << aStr;
    }
  }
}
