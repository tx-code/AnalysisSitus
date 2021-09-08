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

#ifndef ActData_CAFConversionParameter_HeaderFile
#define ActData_CAFConversionParameter_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

DEFINE_STANDARD_HANDLE(ActData_CAFConversionParameter, Standard_Transient)

//! \ingroup AD_DF
//!
//! Element of Conversion Node.
class ActData_CAFConversionParameter : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_CAFConversionParameter, Standard_Transient)

public:

  static const Standard_Integer TempPID = -1; //!< Temporary PID.

  //! Evolution mark.
  enum EvolutionType
  {
    Evolution_None    = 0x00001, //!< Not affected.
    Evolution_Updated = 0x00002, //!< Data updated.
    Evolution_New     = 0x00004, //!< New Parameter.
    Evolution_Deleted = 0x00008, //!< Removed Parameter.
    Evolution_Moved   = 0x00010  //!< Parameter moved as a whole.
  };

  //! Conversion history associated with Parameter.
  struct History
  {
    Standard_Integer OriginPID; //!< Original PID.
    Standard_Integer Evolution; //!< Evolution.

    //! Constructor.
    //! \param theOriginPID [in] original PID to set.
    //! \param theEvolution [in] evolution mark.
    History(const Standard_Integer theOriginPID,
            const EvolutionType theEvolution = Evolution_None) : OriginPID(theOriginPID),
                                                                 Evolution(theEvolution) {}
  };

public:

  ActData_EXPORT
    ActData_CAFConversionParameter(const Handle(ActData_ParameterDTO)& theDTO,
                                   const Standard_Integer theOriginPID = -1);

public:

  //! Accessor for the underlying data.
  //! \return underlying Parameter DTO.
  inline const Handle(ActData_ParameterDTO)& GetData() const
  {
    return m_data;
  }

  //! Accessor for the underlying data.
  //! \return underlying Parameter DTO.
  inline Handle(ActData_ParameterDTO)& ChangeData()
  {
    return m_data;
  }

  //! Accessor for history.
  //! \return convertion history associated with this Parameter.
  inline const History& GetHistory() const
  {
    return m_history;
  }

  //! Accessor for history.
  //! \return convertion history associated with this Parameter.
  inline History& ChangeHistory()
  {
    return m_history;
  }

private:

  //! Actual data in form of Parameter DTO.
  Handle(ActData_ParameterDTO) m_data;

  //! Conversion history.
  History m_history;

};

#endif
