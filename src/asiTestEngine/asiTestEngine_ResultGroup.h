//-----------------------------------------------------------------------------
// Created on: 09 December 2021
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

#ifndef asiTestEngine_ResultGroup_HeaderFile
#define asiTestEngine_ResultGroup_HeaderFile

// asiTestEngine includes
#include <asiTestEngine_Status.h>

// asiAlgo includes
#include <asiAlgo_VisualSettings.h>

// asiTcl includes
#include <asiTcl_Interp.h>

//-----------------------------------------------------------------------------

//! Single group of test results.
struct asiTestEngine_ResultGroup
{
  std::vector<TCollection_AsciiString> caseNames;      //!< Test cases' names.
  std::vector<TCollection_AsciiString> outFilenames;   //!< File reports for each case in a group.
  std::vector<asiTestEngine_Status>    outStatuses;    //!< Statuses for each case in a group.
  std::vector<bool>                    multiBodyFlags; //!< Boolean flags to mark multi-body types.
};

//! Multiple result groups to form the grid of results.
struct asiTestEngine_ResultGroups
{
  struct StrHasher
  {
    static int HashCode(const std::string& str,
                        const int          upper)
    {
      return ::HashCode(str.c_str(), int( str.length() ), upper);
    }

    static bool IsEqual(const std::string& str1,
                        const std::string& str2)
    {
      return str1 == str2;
    }
  };

  //! Groups by names.
  NCollection_DataMap<std::string, asiTestEngine_ResultGroup, StrHasher> groups;

  class Iterator
  {
  public:

    Iterator(const asiTestEngine_ResultGroups& groups) : m_groups(groups)
    {
      m_it.Initialize(m_groups.groups);
    }

    const std::string&               GetName()  const { return m_it.Key(); }
    const asiTestEngine_ResultGroup& GetGroup() const { return m_it.Value(); }
    void                             Next()           { m_it.Next(); }
    bool                             More()     const { return m_it.More(); }

  private:

    Iterator(const Iterator&) = delete;
    void operator=(const Iterator&) = delete;

    const asiTestEngine_ResultGroups& m_groups;
    NCollection_DataMap<std::string, asiTestEngine_ResultGroup, StrHasher>::Iterator m_it;
  };

  //! Accessor for a group by name. If such a group does not exist, it will
  //! be created.
  asiTestEngine_ResultGroup& operator()(const std::string& name)
  {
    if ( !groups.IsBound(name) )
      groups.Bind( name, asiTestEngine_ResultGroup() );
    //
    asiTestEngine_ResultGroup& res = groups(name);
    return res;
  }
};

void PrepareVisualSettings(const Handle(asiTcl_Interp)& interp,
                           const int                    argc,
                           const char**                 argv,
                           asiAlgo_VisualSettings&      settings)
{
  interp->GetKeyValue<float> (argc, argv, "snapshot-line-width",   settings.LineWidth);
  interp->GetKeyValueHex     (argc, argv, "snapshot-color",        settings.Color);
  interp->GetKeyValue<float> (argc, argv, "snapshot-ambient",      settings.Ambient);
  interp->GetKeyValue<float> (argc, argv, "snapshot-diffuse",      settings.Diffuse);
  interp->GetKeyValue<float> (argc, argv, "snapshot-specular",     settings.Specular);
  interp->GetKeyValueHex     (argc, argv, "snapshot-display-mode", settings.DisplayMode);
}

#endif
