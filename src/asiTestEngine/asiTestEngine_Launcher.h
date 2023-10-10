//-----------------------------------------------------------------------------
// Created on: 11 June 2013
//-----------------------------------------------------------------------------
// Copyright (c) 2013-2018, Sergey Slyadnev
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

#ifndef asiTestEngine_Launcher_HeaderFile
#define asiTestEngine_Launcher_HeaderFile

// asiTestEngine includes
#include <asiTestEngine_TestCase.h>

// OCCT includes
#include <Standard_Type.hxx>

//-----------------------------------------------------------------------------
// Launcher API
//-----------------------------------------------------------------------------

//! Base class for Test Case Launcher.
class asiTestEngine_CaseLauncherAPI : public Standard_Transient
{
public:

  virtual bool
    Launch() = 0;

  virtual int
    CaseID() const = 0;

  virtual int
    NumberOfExecuted() const = 0;

  virtual int
    NumberOfFailed() const = 0;

  virtual int
    NumberOfGenRef() const = 0;

  virtual AsiTestFunction
    TestFunction(const int idx) const = 0;

  virtual const std::map<std::string, std::string>&
    Variables() const = 0;

  virtual std::vector<outcome>
    Results() const = 0;

};

//-----------------------------------------------------------------------------
// Launcher for single Test Case
//-----------------------------------------------------------------------------

//! Template-based implementation of Launcher mechanism dedicated to certain
//! Test Case.
template <typename CaseType>
class asiTestEngine_CaseLauncher : public asiTestEngine_CaseLauncherAPI
{
public:

  //! Default constructor.
  asiTestEngine_CaseLauncher() : asiTestEngine_CaseLauncherAPI()
  {
    m_iNumGenRef = 0;
  }

public:

  //! Launches the Test Case of the given type. Returns true in case of
  //! success, false -- otherwise.
  //! \return true/false.
  virtual bool Launch()
  {
    // Collect Test Functions to run.
    asiTestFunctions functions;
    CaseType::Functions(functions);

    // Boolean flags for the test functions asked for genref.
    std::set<int> genrefIds;
    CaseType::GenRefIds(genrefIds);

    // Special treatment for 0: means "all cases".
    const bool allGenref = ( genrefIds.find(0) != genrefIds.end() );

    // Run functions one by one.
    bool areAllOk = true;
    for ( int f = 0; f < (int) functions.Size(); ++f )
    {
      const AsiTestFunction& func = functions.Func(f);

      // Check if update is asked for the reference data.
      bool genref = false;
      //
      if ( allGenref || ( genrefIds.find(f + 1) != genrefIds.end() ) )
      {
        genref = true;
        ++m_iNumGenRef;
      }

      // This condition means that we've got a query to generate reference
      // data for some selected tests. If so, let's not execute any other
      // tests as normally the collect code does not want other tests to run
      // in such scenarios.
      const bool quickGen = !allGenref && !genrefIds.empty();
      //
      if ( quickGen && !genref )
        continue;

      // Run test.
      outcome res = ( *func )( f + 1, genref );

      m_funcResults.push_back(res);

      if ( !res.ok && areAllOk )
        areAllOk = false;
    }
    return areAllOk;
  }

  //! Returns ID of the managed Test Case.
  //! \return ID of the managed Test Case.
  virtual int CaseID() const
  {
    return CaseType::ID();
  }

  //! Returns the total number of executed Test Functions for the managed
  //! Test Case.
  //! \return number of executed Test Functions
  virtual int NumberOfExecuted() const
  {
    return (int) m_funcResults.size();
  }

  //! Returns the total number of failed Test Functions for the managed
  //! Test Case.
  //! \return number of failed Test Functions
  virtual int NumberOfFailed() const
  {
    int numFailed = 0;
    for ( int f = 0; f < (int) m_funcResults.size(); ++f )
    {
      if ( !m_funcResults[f].ok )
        numFailed++;
    }
    return numFailed;
  }

  //! \return number of Test Functions with generated refs.
  virtual int NumberOfGenRef() const
  {
    return m_iNumGenRef;
  }

  //! Returns Test Function referred to by the given 0-based index.
  //! \param idx [in] index of the Test Function to access.
  //! \return Test Function.
  virtual AsiTestFunction TestFunction(const int idx) const
  {
    // Collect Test Functions to run
    asiTestFunctions functions;
    CaseType::Functions(functions);

    // Access Test Function by index
    return functions.Func(idx);
  }

  //! Returns expansion map for local description variables of Test
  //! Function with the given index.
  //! \return expansion map.
  virtual const std::map<std::string, std::string>& Variables() const
  {
    return CaseType::ExpansionMap();
  }

  //! Returns the results of Test Functions run.
  //! \return the output statuses of all run test functions.
  virtual std::vector<outcome> Results() const
  {
    return m_funcResults;
  }

private:

  asiTestEngine_CaseLauncher(const asiTestEngine_CaseLauncher&) {}
  void operator=(const asiTestEngine_CaseLauncher&) {}

private:

  std::vector<outcome> m_funcResults; //!< Execution results.
  int                  m_iNumGenRef;  //!< Number of functions with regenerated reference data.

};

//-----------------------------------------------------------------------------
// Launcher for entire test suite
//-----------------------------------------------------------------------------

//! Launcher for entire test suite. The instance of this class should be
//! populated with the actual Test Case Launchers you want to execute.
class asiTestEngine_Launcher
{
public:

  //! Default constructor.
  asiTestEngine_Launcher() {}

public:

  asiTestEngine_EXPORT asiTestEngine_Launcher&
    operator<<(const Handle(asiTestEngine_CaseLauncherAPI)& CaseLauncher);

  asiTestEngine_EXPORT bool
    Launch(std::ostream* out = nullptr) const;

protected:

  asiTestEngine_EXPORT bool
    generateReport(std::ostream* out,
                   std::string&  filename) const;

  asiTestEngine_EXPORT std::string
    uniqueDirName() const;

private:

  asiTestEngine_Launcher(const asiTestEngine_Launcher&) = delete;
  void operator=(const asiTestEngine_Launcher&) = delete;

private:

  //! Internal collection of Test Case Launchers.
  std::vector< Handle(asiTestEngine_CaseLauncherAPI) > m_launchers;

public:

  asiTestEngine_EXPORT static std::string current_temp_dir; //!< Current temporary directory.

  //! Returns sub-directory used for dumping files.
  //! \return subdirectory dedicated to temporary files.
  inline static std::string current_temp_dir_files()
  {
    return asiAlgo_Utils::Str::Slashed(current_temp_dir) + "files";
  }

  //! Returns sub-directory used for reading source files.
  //! \return subdirectory dedicated to source files.
  inline static std::string current_temp_dir_source()
  {
    return asiAlgo_Utils::Str::Slashed( asiAlgo_Utils::Env::AsiTestData() );
  }

};

#endif
