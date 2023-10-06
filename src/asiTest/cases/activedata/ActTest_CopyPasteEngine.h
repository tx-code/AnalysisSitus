//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef ActTest_CopyPasteEngine_HeaderFile
#define ActTest_CopyPasteEngine_HeaderFile

// asiTest includes
#include "ActTest_DataFramework.h"

// Active Data unit tests
#include <ActTest_DummyModel.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_CopyPasteEngine.h>
#include <ActData_ParameterFactory.h>

//! \ingroup AD_TEST
//!
//! Test suite for Active Data.
//! This class performs unit testing of Copy/Paste functionality provided
//! by CopyPasteTool class shipped with Data Framework.
class ActTest_CopyPasteEngine : public asiTestEngine_TestCase
{
public:

  typedef NCollection_Sequence<ActAPI_DataObjectId> TreeLevelSeq;

public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_CopyPasteEngine;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "ActTest_CopyPasteEngine";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "Tools";
  }

  //! Returns the IDs of the test cases to generate reference data for.
  static void GenRefIds(std::set<int>& genrefIds)
  {
    (void) genrefIds;
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param functions [out] output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &testCopyPaste_PlainToPlain
              << &testCopyPaste_TreeToPlain
              << &testCopyPaste_TreeWithReferencesToPlain_1
              << &testCopyPaste_TreeWithReferencesToPlain_2
              << &testCopyPaste_TreeWithReferencesToPlain_3
              << &testCopyPaste_TreeWithReferencesToPlain_4
              << &testCopyPaste_TreeWithReferencesToPlain_5
              << &testCopyPaste_TreeWithEvalReferencesToPlain
              << &testCopyPaste_TreeWithPlainReferenceToPlain_1
              << &testCopyPaste_TreeWithPlainReferenceToPlain_2
              << &testCopyPaste_TreeWithPlainReferenceToPlain_3
              << &testCopyPaste_TreeWithPlainReferenceToPlain_4
              << &testCopyPaste_TreeWithListReferenceToPlain_1
              << &testCopyPaste_TreeWithListReferenceToPlain_2
              << &testCopyPaste_TreeWithListReferenceToPlain_3
              << &testCopyPaste_TreeWithListReferenceToPlain_4
              << &testCopyPaste_PlainMeshToPlain
              << &testCopyPaste_PasteWithDEAD_DFunctionArgument
              << &testCopyPaste_PasteWithDEAD_DFunctionResult
              << &testCopyPaste_PasteWithDEADReference;
  }

// Test functions:
private:

  static outcome testCopyPaste_PlainToPlain                     (const int funcID, const bool); // Case 1
  static outcome testCopyPaste_TreeToPlain                      (const int funcID, const bool); // Case 2
  static outcome testCopyPaste_TreeWithReferencesToPlain_1      (const int funcID, const bool); // Case 3.1
  static outcome testCopyPaste_TreeWithReferencesToPlain_2      (const int funcID, const bool); // Case 3.2
  static outcome testCopyPaste_TreeWithReferencesToPlain_3      (const int funcID, const bool); // Case 3.3
  static outcome testCopyPaste_TreeWithReferencesToPlain_4      (const int funcID, const bool); // Case 3.4
  static outcome testCopyPaste_TreeWithReferencesToPlain_5      (const int funcID, const bool); // Case 3.5
  static outcome testCopyPaste_TreeWithEvalReferencesToPlain    (const int funcID, const bool); // Case 4
  static outcome testCopyPaste_TreeWithPlainReferenceToPlain_1  (const int funcID, const bool); // Case 5.1
  static outcome testCopyPaste_TreeWithPlainReferenceToPlain_2  (const int funcID, const bool); // Case 5.2
  static outcome testCopyPaste_TreeWithPlainReferenceToPlain_3  (const int funcID, const bool); // Case 5.3
  static outcome testCopyPaste_TreeWithPlainReferenceToPlain_4  (const int funcID, const bool); // Case 5.4
  static outcome testCopyPaste_TreeWithListReferenceToPlain_1   (const int funcID, const bool); // Case 6.1
  static outcome testCopyPaste_TreeWithListReferenceToPlain_2   (const int funcID, const bool); // Case 6.2
  static outcome testCopyPaste_TreeWithListReferenceToPlain_3   (const int funcID, const bool); // Case 6.3
  static outcome testCopyPaste_TreeWithListReferenceToPlain_4   (const int funcID, const bool); // Case 6.4
  static outcome testCopyPaste_PlainMeshToPlain                 (const int funcID, const bool); // Case 7
  static outcome testCopyPaste_PasteWithDEAD_DFunctionArgument  (const int funcID, const bool); // Case 8
  static outcome testCopyPaste_PasteWithDEAD_DFunctionResult    (const int funcID, const bool); // Case 9
  static outcome testCopyPaste_PasteWithDEADReference           (const int funcID, const bool); // Case 10

private:

  static outcome
    populateSampleTree(const Handle(ActTest_DummyModel)& M,
                       ActAPI_DataObjectIdList& ANodeIDs,
                       ActAPI_DataObjectIdList& BNodeIDs,
                       ActAPI_DataObjectIdList& VARNodeIDs,
                       const std::string&       nameFunc = "",
                       const int                funcID   = 0);

  static void
    verifyTree(const Handle(ActAPI_INode)& theRoot,
               const ActData_CopyPasteEngine::RelocationTable& COPYReloc,
               const ActData_CopyPasteEngine::RelocationTable& PASTEReloc,
               TreeLevelSeq* TREELevels,
               const Standard_Integer LevelIndex,
               const Standard_Integer SiblingIndex,
               Standard_Boolean& IsOK);

  static TCollection_AsciiString
    dumpPath();

private:

  //! Auxiliary tool performing validation of Nodal references.
  class ReferenceValidator
  {
  public:

    //! Default constructor.
    ReferenceValidator() {}

  public:

    //! Nested validator for plain references.
    class Reference
    {
    public:

      //! Constructor initializing the nested reference with the given one.
      //! \param Param [in] Reference Parameter to validate.
      Reference(const Handle(ActAPI_IUserParameter)& Param)
      {
        param = ActData_ParameterFactory::AsReference(Param);
      }

      //! Checks whether the referenced target is equal to the given one.
      //! \param theDC [in] expected target.
      //! \return true/false.
      Standard_Boolean TargetIs(const Handle(ActAPI_IDataCursor)& theDC)
      {
        return ActAPI_IDataCursor::IsEqual(param->GetTarget(), theDC);
      }

    private:

      //! Reference Parameter being validated.
      Handle(ActData_ReferenceParameter) param;

    };

    //! Nested validator for reference lists.
    class ReferenceList
    {
    public:

      //! Constructor initializing the nested reference with the given one.
      //! \param theParam [in] Reference Parameter to validate.
      ReferenceList(const Handle(ActAPI_IUserParameter)& theParam)
      {
        param = ActData_ParameterFactory::AsReferenceList(theParam);
      }

      //! Checks whether the Reference List Parameter points to the given
      //! target.
      //! \param theDC [in] expected target.
      //! \return true/false.
      Standard_Boolean HasTarget(const Handle(ActAPI_IDataCursor)& theDC)
      {
        return param->HasTarget(theDC) > 0;
      }

      //! Checks whether the working Reference List is empty.
      //! \return true/false.
      Standard_Boolean IsEmpty()
      {
        return param->AccessReferenceList().IsNull() || param->AccessReferenceList()->IsEmpty();
      }

    private:

      //! Reference List Parameter being validated.
      Handle(ActData_ReferenceListParameter) param;

    };

    //! Nested validator for Nodes from referencing perspective.
    class Node
    {
    public:

      //! Constructs Node validator initialized with the given Node.
      //! \param Node [in] Data Node to validate.
      Node(const Handle(ActAPI_INode)& Node)
      {
        m_node = Node;
      }

      //! Checks the exact equality of the registered INPUT Readers to the
      //! passed list (stream) of Parameters.
      Standard_Boolean InputReadersAre(const ActAPI_ParameterStream& theStream)
      {
        if ( m_node->GetInputReaders()->Length() != theStream.List->Length() )
          return Standard_False;

        Handle(ActAPI_HParameterList) IR = m_node->GetInputReaders();
        for ( ActAPI_ParameterList::Iterator it( *IR.operator->() ); it.More(); it.Next() )
        {
          Standard_Boolean isObserverInStream = Standard_False;
          for ( ActAPI_ParameterList::Iterator sit( *theStream.List.operator->() ); sit.More(); sit.Next() )
          {
            if ( ActAPI_IDataCursor::IsEqual( it.Value(), sit.Value() ) )
            {
              isObserverInStream = Standard_True;
              break;
            }
          }
          if ( !isObserverInStream )
            return Standard_False;
        }

        return Standard_True;
      }

      //! Checks the exact equality of the registered OUTPUT Writers to the
      //! passed list (stream) of Parameters.
      Standard_Boolean OutputWritersAre(const ActAPI_ParameterStream& theStream)
      {
        if ( m_node->GetOutputWriters()->Length() != theStream.List->Length() )
          return Standard_False;

        Handle(ActAPI_HParameterList) OW = m_node->GetOutputWriters();
        for ( ActAPI_ParameterList::Iterator it( *OW.operator->() ); it.More(); it.Next() )
        {
          Standard_Boolean isObserverInStream = Standard_False;
          for ( ActAPI_ParameterList::Iterator sit( *theStream.List.operator->() ); sit.More(); sit.Next() )
          {
            if ( ActAPI_IDataCursor::IsEqual( it.Value(), sit.Value() ) )
            {
              isObserverInStream = Standard_True;
              break;
            }
          }
          if ( !isObserverInStream )
            return Standard_False;
        }

        return Standard_True;
      }

      //! Checks the exact equality of the registered REFERRERS to the
      //! passed list (stream) of Parameters.
      Standard_Boolean ReferrersAre(const ActAPI_ParameterStream& theStream)
      {
        if ( m_node->GetReferrers()->Length() != theStream.List->Length() )
          return Standard_False;

        Handle(ActAPI_HParameterList) R = m_node->GetReferrers();
        for ( ActAPI_ParameterList::Iterator it( *R.operator->() ); it.More(); it.Next() )
        {
          Standard_Boolean isObserverInStream = Standard_False;
          for ( ActAPI_ParameterList::Iterator sit( *theStream.List.operator->() ); sit.More(); sit.Next() )
          {
            if ( ActAPI_IDataCursor::IsEqual( it.Value(), sit.Value() ) )
            {
              isObserverInStream = Standard_True;
              break;
            }
          }
          if ( !isObserverInStream )
            return Standard_False;
        }

        return Standard_True;
      }

      //! Returns true if the validated Data Node has any observers, i.e. it
      //! has non-empty list of back-references.
      //! \return true/false.
      Standard_Boolean HasObservers()
      {
        return this->HasInputReaders() || this->HasOutputWriters() || this->HasReferrers();
      }

      //! Returns true if the validated Data Node has any INPUT Readers.
      //! \return true/false.
      Standard_Boolean HasInputReaders()
      {
        return !m_node->GetInputReaders()->IsEmpty();
      }

      //! Returns true if the validated Data Node has any OUTPUT Writers.
      //! \return true/false.
      Standard_Boolean HasOutputWriters()
      {
        return !m_node->GetOutputWriters()->IsEmpty();
      }

      //! Returns true if the validated Data Node has any Referrers.
      //! \return true/false.
      Standard_Boolean HasReferrers()
      {
        return !m_node->GetReferrers()->IsEmpty();
      }

    private:

      //! Data Node being validated.
      Handle(ActAPI_INode) m_node;

    };

    //! Nested validator for Tree Function Parameters.
    class TreeFunction
    {
    public:

      //! Constructs Tree Function validator initialized with the given
      //! reference.
      //! \param Param [in] Tree Function Parameter to validate.
      TreeFunction(const Handle(ActAPI_IUserParameter)& Param)
      {
        param = ActData_ParameterFactory::AsTreeFunction(Param);
      }

      //! Checks whether the validated Tree Function Parameter has output
      //! Parameters from the given Node.
      //! \param Node [in] Node to check.
      //! \return true/false.
      Standard_Boolean IsOutputWriterFor(const Handle(ActAPI_INode)& Node)
      {
        Handle(ActAPI_HParameterList) OW = Node->GetOutputWriters();
        for ( ActAPI_ParameterList::Iterator it( *OW.operator->() ); it.More(); it.Next() )
        {
          if ( ActAPI_IDataCursor::IsEqual(it.Value(), param) )
            return Standard_True;
        }

        return Standard_False;
      }

      //! Checks whether the validated Tree Function Parameter has output
      //! Parameter equal the given one.
      //! \param Param [in] Parameter to check.
      //! \return true/false.
      Standard_Boolean HasAsResult(const Handle(ActAPI_IUserParameter)& Param)
      {
        Handle(ActAPI_HParameterList) Results = param->Results();

        if ( Results.IsNull() )
          return Standard_False;

        for ( ActAPI_ParameterList::Iterator it( *Results.operator->() ); it.More(); it.Next() )
        {
          if ( ActAPI_IDataCursor::IsEqual(it.Value(), Param) )
            return Standard_True;
        }

        return Standard_False;
      }

      //! Checks whether the validated Tree Function Parameter has input
      //! Parameters from the given Node.
      //! \param Node [in] Node to check.
      //! \return true/false.
      Standard_Boolean IsInputReaderFor(const Handle(ActAPI_INode)& Node)
      {
        Handle(ActAPI_HParameterList) IR = Node->GetInputReaders();
        for ( ActAPI_ParameterList::Iterator it( *IR.operator->() ); it.More(); it.Next() )
        {
          if ( ActAPI_IDataCursor::IsEqual(it.Value(), param) )
            return Standard_True;
        }

        return Standard_False;
      }

      //! Checks whether the validated Tree Function Parameter has input
      //! Parameter equal the given one.
      //! \param Param [in] Parameter to check.
      //! \return true/false.
      Standard_Boolean HasAsArgument(const Handle(ActAPI_IUserParameter)& Param)
      {
        Handle(ActAPI_HParameterList) Args = param->Arguments();

        if ( Args.IsNull() )
          return Standard_False;

        for ( ActAPI_ParameterList::Iterator it( *Args.operator->() ); it.More(); it.Next() )
        {
          if ( ActAPI_IDataCursor::IsEqual(it.Value(), Param) )
            return Standard_True;
        }

        return Standard_False;
      }

    private:

      //! Tree Function Parameter being checked.
      Handle(ActData_TreeFunctionParameter) param;

    };

  };

};

#endif
