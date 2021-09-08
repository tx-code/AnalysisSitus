//-----------------------------------------------------------------------------
// Created on: November 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, OPEN CASCADE SAS
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

// Active Data includes
#include <ActData_Common.h>

//-----------------------------------------------------------------------------

void ActData_Common::SplitTags(const ActAPI_DataObjectId&            objectId,
                               std::vector<TCollection_AsciiString>& tags)
{
  TCollection_AsciiString token;
  Standard_Integer t = 1;
  //
  do
  {
    token = objectId.Token(":", t++);
    if ( token.Length() )
      tags.push_back(token);
  }
  while ( token.Length() );
}

//-----------------------------------------------------------------------------

ActAPI_NodeId
  ActData_Common::NodeIdByParameterId(const ActAPI_ParameterId& paramId)
{
  std::vector<ActAPI_DataObjectId> tags;
  ActData_Common::SplitTags(paramId, tags);

  int tagsToSkip = 0;
  if ( tags.size() == ActData_NumTags_MetaParameterId )
    tagsToSkip = 1;
  else if ( tags.size() == ActData_NumTags_UserParameterId )
    tagsToSkip = 2;
  else
    Standard_ProgramError::Raise("Unexpected format of Parameter ID.");

  ActAPI_DataObjectId nodeId;
  for ( size_t k = 0; k < tags.size() - tagsToSkip; ++k )
  {
    if ( k > 0 )
      nodeId += ":";
    //
    nodeId += tags[k];
  }

  return nodeId;
}

//-----------------------------------------------------------------------------

ActAPI_ParameterId
  ActData_Common::TrimToParameterId(const ActAPI_DataObjectId& objectId,
                                    bool&                      isValid)
{
  std::vector<ActAPI_DataObjectId> tags;
  ActData_Common::SplitTags(objectId, tags);

  // Validate the passed ID.
  if ( tags.size() < ActData_NumTags_MetaParameterId )
  {
    isValid = false;
    return ActAPI_ParameterId();
  }
  //
  isValid = true;

  ActAPI_ParameterId trimmedId;
  const int limit = Min( ActData_NumTags_UserParameterId, int( tags.size() ) );
  //
  for ( int k = 0; k < limit; ++k )
  {
    if ( k > 0 )
      trimmedId += ":";
    //
    trimmedId += tags[k];
  }

  return trimmedId;
}
