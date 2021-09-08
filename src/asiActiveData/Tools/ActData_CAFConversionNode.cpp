//-----------------------------------------------------------------------------
// Created on: March 2013
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
#include <ActData_CAFConversionNode.h>

//! Default constructor.
ActData_CAFConversionNode::ActData_CAFConversionNode()
: Standard_Transient(),
  m_iMinPID(ActData_CAFConversionParameter::TempPID)
{
}

//! Populates Conversion Node structure with the given DTO.
//! \param theDTO [in] incoming DTO.
//! \return this for subsequent streaming.
void ActData_CAFConversionNode::AddOrigin(const Handle(ActData_ParameterDTO)& theDTO)
{
  Standard_Integer anOriPID = theDTO->GID().PID;

  // Affect minimal PID if necessary
  if ( m_iMinPID == ActData_CAFConversionParameter::TempPID ||
       anOriPID < m_iMinPID )
    m_iMinPID = anOriPID;

  m_params.Append( new ActData_CAFConversionParameter(theDTO, anOriPID) );
}

//! Returns original ID of the Node.
//! \return Node ID.
ActAPI_DataObjectId ActData_CAFConversionNode::NID() const
{
  if ( m_params.IsEmpty() )
    return ActAPI_DataObjectId();

  return m_params.First()->GetData()->GID().NID;
}

//! Returns Conversion Parameter identified by the passed original PID.
//! \param theOriPID [in] original PID to access Conversion Parameter for.
//! \return Conversion Parameter instance.
Handle(ActData_CAFConversionParameter)
  ActData_CAFConversionNode::ParamByOrigin(const Standard_Integer theOriPID) const
{
  if ( theOriPID == ActData_CAFConversionParameter::TempPID )
    return NULL;

  for ( Standard_Integer i = 1; i <= m_params.Length(); ++i )
  {
    const Handle(ActData_CAFConversionParameter)& aCAFParam = m_params.Value(i);
    const ActData_CAFConversionParameter::History& aHistory = aCAFParam->GetHistory();

    if ( aHistory.OriginPID == theOriPID )
      return aCAFParam;
  }
  return NULL;
}

//! Inserts new Parameter before one with the given PID.
//! \param theDTO [in] DTO representing Parameter to insert.
//! \param theGIDBefore [in] GID of the Parameter to insert new one before.
//!        This argument is optional. If not passed, append operation is
//!        done instead of insertion.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionNode::Insert(const Handle(ActData_ParameterDTO)& theDTO,
                                                   const ActAPI_ParameterGID& theGIDBefore)
{
  Standard_Integer aIdxBefore = this->posParameter(theGIDBefore);

  // Create Conversion Parameter with evolution as NEW and PID as -1
  Handle(ActData_CAFConversionParameter)
    aCAFParam = new ActData_CAFConversionParameter(theDTO, ActData_CAFConversionParameter::TempPID);
  aCAFParam->ChangeHistory().Evolution = ActData_CAFConversionParameter::Evolution_New;

  // Add Conversion Parameter to collection
  if ( aIdxBefore != -1 )
    m_params.InsertBefore(aIdxBefore, aCAFParam);
  else
    m_params.Append(aCAFParam);

  return Standard_True;
}

//! Updates Parameter identified by the given GID with the passed data.
//! \param theGID [in] GID of Parameter to update.
//! \param theDTO [in] data to set.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionNode::Update(const ActAPI_ParameterGID& theGID,
                                                   const Handle(ActData_ParameterDTO)& theDTO)
{
  Standard_Integer aIdx = this->posParameter(theGID);
  if ( aIdx == -1 )
    return Standard_False;

  // Access already recorded data
  const Handle(ActData_CAFConversionParameter)& aCAFParam = m_params.Value(aIdx);
  const Handle(ActData_ParameterDTO)& aData = aCAFParam->GetData();

  // Check type
  if ( aData->ParamType() != theDTO->ParamType() )
    return Standard_False;

  // Perform evolution
  aCAFParam->ChangeData() = theDTO;
  aCAFParam->ChangeHistory().Evolution = ActData_CAFConversionParameter::Evolution_Updated;
  return Standard_True;
}

//! Removes Parameter with the given GID.
//! \param theGID [in] GID of the Parameter to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionNode::Remove(const ActAPI_ParameterGID& theGID)
{
  Standard_Integer aIdx = this->posParameter(theGID);
  if ( aIdx == -1 )
    return Standard_False;

  const Handle(ActData_CAFConversionParameter)& aCAFParam = m_params.Value(aIdx);
  aCAFParam->ChangeData()->ChangeGID().PID = ActData_CAFConversionParameter::TempPID;
  aCAFParam->ChangeHistory().Evolution = ActData_CAFConversionParameter::Evolution_Deleted;
  return Standard_True;
}

//! Performs normalization of PIDs. As a result, all stored Conversion
//! Parameters will be re-enumerated starting from the minimal original
//! PID. This step is suggested as a final one for working session with
//! Conversion Node.
void ActData_CAFConversionNode::NormalizePIDs()
{
  Standard_Integer aNewPID = m_iMinPID;
  for ( Standard_Integer i = 1; i <= m_params.Length(); ++i )
  {
    const Handle(ActData_CAFConversionParameter)& aCAFParam = m_params.Value(i);
    const Handle(ActData_ParameterDTO)& aDTO = aCAFParam->GetData();
    ActAPI_ParameterGID& aGID = aDTO->ChangeGID();

    // Skip removed Parameters
    if ( aCAFParam->GetHistory().Evolution == ActData_CAFConversionParameter::Evolution_Deleted )
      continue;

    // Change GID of Conversion Parameter
    if ( aGID.PID != aNewPID )
    {
      aGID.PID = aNewPID;
      ActData_CAFConversionParameter::History& aHistory = aCAFParam->ChangeHistory();
      aHistory.Evolution |= ActData_CAFConversionParameter::Evolution_Moved;
    }

    // Increment new PID
    aNewPID++;
  }
}

//! Internal method used to find Conversion Parameter by the given GID.
//! \param theGID [in] GID of the Parameter to find.
//! \return index of the found Parameter or -1 if nothing was found.
Standard_Integer
  ActData_CAFConversionNode::posParameter(const ActAPI_ParameterGID& theGID) const
{
  for ( Standard_Integer i = 1; i <= m_params.Length(); ++i )
  {
    const Handle(ActData_CAFConversionParameter)& aCAFParam = m_params.Value(i);
    const ActAPI_ParameterGID& aCurGID = aCAFParam->GetData()->GID();

    if ( ActAPI_ParameterGID::Hasher::IsEqual(aCurGID, theGID) )
      return i;
  }
  return -1;
}
