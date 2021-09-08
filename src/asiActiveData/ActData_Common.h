//-----------------------------------------------------------------------------
// Created on: February 2012
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

#ifndef ActData_Common_HeaderFile
#define ActData_Common_HeaderFile

// Active Data includes
#include <ActData.h>

// Active Data (API) includes
#include <ActAPI_INode.h>
#include <ActAPI_IParameter.h>

// Standard includes
#include <vector>

#define ActData_NumTags_NodeId          4
#define ActData_NumTags_MetaParameterId 5
#define ActData_NumTags_UserParameterId 6

//-----------------------------------------------------------------------------
// DOXY group definition
//-----------------------------------------------------------------------------
//! \defgroup AD_DF Data Framework
//!
//! Data Framework.
//-----------------------------------------------------------------------------

namespace ActData_Common
{
  //! Splits the passed ID by the conventional delimiter ":" onto tags.
  //! E.g., the result of splitting the id "0:1:2" will be the vector
  //! whose components are "0", "1" and "2".
  //! \param[in]  objectId input ID to split.
  //! \param[out] tags     resulting collection of tags after split.
  ActData_EXPORT void
    SplitTags(const ActAPI_DataObjectId&            objectId,
              std::vector<TCollection_AsciiString>& tags);

  //! Extracts ID of the Node by ID of the Parameter.
  //! \param[in] paramId ID of the Parameter.
  //! \return corresponding Node ID.
  ActData_EXPORT ActAPI_NodeId
    NodeIdByParameterId(const ActAPI_ParameterId& paramId);

  //! Removes trailing tags to trim the passed object ID to the User Parameter
  //! ID or Meta Parameter ID.
  //! \param[in]  objectId object ID to trim.
  //! \param[out] isValid  true if the passed ID is valid, false -- otherwise.
  //! \return trimmed objectId.
  ActData_EXPORT ActAPI_ParameterId
    TrimToParameterId(const ActAPI_DataObjectId& objectId,
                      bool&                      isValid);
} // ActData_Common namespace.

#endif
