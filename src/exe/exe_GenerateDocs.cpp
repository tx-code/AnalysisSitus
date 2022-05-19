//-----------------------------------------------------------------------------
// Created on: 18 May 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <exe_GenerateDocs.h>

// asiTestEngine includes
#include <asiTestEngine_ReportRenderer.h>

// STD includes
#include <algorithm>
#include <iostream>
#include <fstream>

//-----------------------------------------------------------------------------

namespace
{
  //! Sorts commands by names.
  void SortCommands(std::vector<asiTcl_CommandInfo>& cmdInfos)
  {
    // Sort commands by names.
    std::sort( cmdInfos.begin(), cmdInfos.end(),
               [&](const asiTcl_CommandInfo& a, const asiTcl_CommandInfo& b)
               {
                 return a.Name < b.Name;
               } );
  }

  //! Highlights code snippets.
  void Beautify(std::vector<asiTcl_CommandInfo>& cmdInfos)
  {
    for ( auto& cmdInfo : cmdInfos )
    {
      std::stringstream helpStreamIn(cmdInfo.Help);
      std::stringstream helpStreamOut;

      // Find a line containing help string.
      std::string line;
      while ( std::getline(helpStreamIn, line) )
      {
        if ( line.find(cmdInfo.Name) == 0 )
        {
          helpStreamOut << "<div class='code-inline code-margin'>";
          helpStreamOut << line;
          helpStreamOut << "</div>";
        }
        else
        {
          helpStreamOut << line;
        }
      }

      cmdInfo.Help = helpStreamOut.str();
    }
  }

  //! Converts newline characters to HTML line breaks.
  void Nl2Br(std::vector<asiTcl_CommandInfo>& cmdInfos)
  {
    for ( auto& cmdInfo : cmdInfos )
    {
      asiAlgo_Utils::Str::ReplaceAll(cmdInfo.Help, "\n", "<br/>");
    }
  }

  //! Converts tag brackets to special HTML characters.
  void Normalize(std::vector<asiTcl_CommandInfo>& cmdInfos)
  {
    for ( auto& cmdInfo : cmdInfos )
    {
      asiAlgo_Utils::Str::ReplaceAll(cmdInfo.Help, "<", "&lt;");
      asiAlgo_Utils::Str::ReplaceAll(cmdInfo.Help, ">", "&gt;");
    }
  }
}

//-----------------------------------------------------------------------------

bool exe_GenerateDocs::Perform(const Handle(asiTcl_Interp)& interp,
                               const std::string&           filenameIn,
                               const std::string&           filenameOut)
{
  std::ifstream instream(filenameIn);
  std::stringstream buffer;
  buffer << instream.rdbuf();

  std::string readout;
  std::string tagOpen("<div id=\"id-commands\">");
  std::string tagClose("</div>");

  /* Commands */
  std::string replace;
  {
    std::vector<asiTcl_CommandInfo> cmdInfos;
    interp->GetAvailableCommands(cmdInfos);

    // Sort commands by names.
    ::SortCommands(cmdInfos);

    // Prepare.
    ::Normalize(cmdInfos);

    // Beautify.
    ::Beautify(cmdInfos);

    // Nl to br.
    ::Nl2Br(cmdInfos);

    Handle(asiTestEngine_ReportRenderer)
      Rdr = new asiTestEngine_ReportRenderer;

    // Generate table header
    Rdr->StartTable()
       ->StartTableRow()
       ->StartTableHCell("table-content-header")
       ->AddText("Command")
       ->EndTableHCell()
       ->StartTableHCell("table-content-header")
       ->AddText("Description")
       ->EndTableHCell()
       ->EndTableRow();

    // Iterate over commands.
    for ( const auto& cmdInfo : cmdInfos )
    {
      Rdr->StartTableRow()
         //
         ->StartTableCell("command-tr command-td-cell")
         ->StartPre()
         ->AddText("<span id=\"")
         ->AddText( cmdInfo.Name )
         ->AddText("\">")
         ->AddText( cmdInfo.Name )
         ->AddText("</span>")
         ->BreakRow()
         ->AddText("[")
         ->AddHRef( (filenameOut + "#" + cmdInfo.Name).c_str(), "link" )
         ->AddText("]")
         ->EndPre()
         ->EndTableCell()
         //
         ->StartTableCell("command-tr command-td-cell-help")
         ->AddText( cmdInfo.Help )
         ->EndTableCell()
         //
         ->EndTableRow();
    }

    // Finish table
    Rdr->EndTable();

    replace = Rdr->Flush();
  }

  std::stringstream outstream;

  while ( std::getline(buffer, readout) )
  {
    if ( readout == tagOpen )
    {
      outstream << tagOpen << "\n";
      outstream << replace << "\n";
    }
    else
    {
      outstream << readout;
      outstream << "\n";
    }
  }

  // Write back.
  std::ofstream outFile;
  outFile.open(filenameOut);
  outFile << outstream.rdbuf();

  return true;
}
