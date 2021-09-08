//-----------------------------------------------------------------------------
// Created on: June 2012
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

#ifndef ActData_LogBook_HeaderFile
#define ActData_LogBook_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_TreeFunctionParameter.h>

//! \ingroup AD_DF
//!
//! Auxiliary Data Cursor for accessing LogBook section of Data Model. LogBook
//! is used to record Tree Function Execution requests before actual execution
//! session. We distinguish two types of Tree Functions:
//!
//! 1. Normal (light);
//! 2. Heavy.
//!
//! NORMAL Tree Function can be pushed for execution in several ways:
//!
//! - One of its INPUTs has been modified;
//! - Tree Function is FORCED to execute;
//! - None of the INPUTs is modified, however, Tree Function signalizes that
//!   it "wants" to be executed. Normally this way is used to provide
//!   sophisticated initiation logic into Tree Function triggering mechanism.
//!   We call this triggering option "implicit parameterization" as it
//!   normally implies that INPUTs are not connected to Tree Function in a
//!   classic way.
//!
//! All these rules are applicable for HEAVY Functions as well. However,
//! satisfying these rules does not lead to the actual execution of Tree
//! Function algorithm. Therefore, satisfying these rules is only essential
//! but not enough condition of launching. In order for HEAVY Tree Function
//! to execute, the corresponding Tree Function Parameter (representing the
//! actual instance of Function bound to a specific Node) must be registered
//! in a LogBook via HEAVY DEPLOYMENT record. Notice that once Tree Functions
//! are executed the LogBook is cleaned up, so HEAVY DEPLOYMENT records
//! will disappear. This prevents error-prone cumulating of HEAVY DEPLOYMENT
//! requests during application life cycle.
//!
//! If HEAVY Tree Function is recorded in HEAVY DEPLOYMENT section of the
//! LogBook, it is executed in a usual way. Propagation goes on till the next
//! HEAVY Function is reached (if any). For the next Function the triggering
//! rules are checked again. This means that in order to re-execute completely
//! some sub-graph, you should prepare HEAVY DEPLOYMENT records for each HEAVY
//! Tree Function in that sub-graph. Otherwise the propagation will stop on the
//! next HEAVY Function.
//!
//! If HEAVY Tree Function is not recorded in HEAVY DEPLOYMENT section of the
//! LogBook, the Tree Function mechanism does not stop immediately. It rather
//! continues execution in the idle mode in order to propagate dedicated
//! PENDING status for each involved OUTPUT Parameter. This process is similar
//! to propagating of invalidation wave, however, it concerns only HEAVY
//! Tree Functions (while VALIDATION statuses are used in a common way).
class ActData_LogBook
{
public:

  //! CAF data structure accepted by this Data Cursor.
  enum StructureTags
  {
    StructureTag_Touched     = 1, //!< Sub-Label for TOUCHED references.
    StructureTag_Impacted    = 2, //!< Sub-Label for IMPACTED references.
    StructureTag_Forced      = 3, //!< Sub-Label for FORCED references.
    StructureTag_HeavyDeploy = 4  //!< Sub-Label for HEAVY DEPLOYMENT references.
  };

// Services:
public:

  ActData_EXPORT static Standard_Boolean
    IsModifiedCursor(const Handle(ActAPI_IDataCursor)& theDC);

  ActData_EXPORT static Standard_Boolean
    IsPendingCursor(const Handle(ActAPI_IDataCursor)& theDC);

// Construction:
public:

  ActData_EXPORT
    ActData_LogBook(const TDF_Label& theRootLab);

public:

  ActData_EXPORT void
    ClearReferencesFor(const TDF_Label& theLab);

public:

  //! Accessor for the root Label.
  //! \return root Label.
  const TDF_Label& Label() const
  {
    return m_root;
  }

// Normal execution:
public:

  ActData_EXPORT void
    Touch(const TDF_Label& theLab);

  ActData_EXPORT void
    Touch(const Handle(ActAPI_IUserParameter)& theParam);

  ActData_EXPORT Standard_Boolean
    IsTouched(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    IsTouched(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    Impact(const TDF_Label& theLab);

  ActData_EXPORT void
    Impact(const Handle(ActAPI_IUserParameter)& theParam);

  ActData_EXPORT Standard_Boolean
    IsImpacted(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    IsImpacted(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT Standard_Boolean
    IsModified(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    IsModified(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    ReleaseModified();

// Forced execution:
public:

  ActData_EXPORT void
    Force(const Handle(ActData_TreeFunctionParameter)& theFuncParam);

  ActData_EXPORT Standard_Boolean
    IsForced(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    IsForced(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    ReleaseForced();

// Heavy deployment:
public:

  ActData_EXPORT void
    HeavyDeploy(const Handle(ActData_TreeFunctionParameter)& theFuncParam);

  ActData_EXPORT Standard_Boolean
    IsHeavyDeployment(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    IsHeavyDeployment(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    ReleaseHeavyDeployment();

// Kernel methods:
public:

  void addToReferenceMap(const TDF_Label& theLab,
                          const StructureTags theTag);

  void addToReferenceMap(const Handle(ActAPI_IUserParameter)& theParam,
                         const StructureTags theTag);

  Standard_Boolean isReferenced(const TDF_Label& theLab,
                                const StructureTags theTag) const;

  Standard_Boolean isReferenced(const Handle(ActAPI_IUserParameter)& theParam,
                                const StructureTags theTag) const;

  void clearReferences(const StructureTags theTag);

  void clearReferences(const TDF_Label& theLabel,
                       const StructureTags theTag);

private:

  //! Root Label of LogBook section.
  TDF_Label m_root;

};

#endif
