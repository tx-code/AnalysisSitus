//-----------------------------------------------------------------------------
// Created on: 12 December 2021
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

// Own include
#include <asiTestEngine_Utils.h>

// asiTestEngine includes
#include <asiTestEngine_ReportRenderer.h>
#include <asiTestEngine_ReportStyle.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDateTime>
#include <QDir>
#pragma warning(pop)

//-----------------------------------------------------------------------------

std::string asiTestEngine_Utils::GetGroupNameFromFile(const std::string& filename)
{
  return asiAlgo_Utils::Str::LastDirname(filename);
}

//-----------------------------------------------------------------------------

std::string asiTestEngine_Utils::GetUniqueDirName()
{
  // Prepare temporary directory for dumping results.
  QDateTime dateTime    = QDateTime::currentDateTime();
  QString   dateTimeStr = dateTime.toString("yyyy-MM-dd-HHmmss");

  // If such a directory already exists, add a numerical suffix for
  // uniqueness.
  if ( QDir(dateTimeStr).exists() )
  {
    int counter = 0;

#ifdef _WIN32
    static LONG INTERNAL = 0;
    counter = (int) InterlockedIncrement(&INTERNAL);
#else
    static unsigned long INTERNAL = 0;
    static Standard_Mutex MUTEX;

    MUTEX.Lock();
    counter = ++INTERNAL;
    MUTEX.Unlock();
#endif

    dateTimeStr += "_";
    dateTimeStr += QString::number(counter);
  }

  return dateTimeStr.toUtf8().constData();
}

//-----------------------------------------------------------------------------

bool asiTestEngine_Utils::CreateIndex(const TCollection_AsciiString&    outputDir,
                                      const asiTestEngine_ResultGroups& resultGroups,
                                      const int                         numSucceeded,
                                      const int                         numFailed,
                                      const int                         numBad,
                                      const int                         numNoref,
                                      const int                         numGenref)
{
  Handle(asiTestEngine_ReportRenderer) Rdr = new asiTestEngine_ReportRenderer;

  // Global style for HTML body.
  asiTestEngine_ReportStyle BodyStyle;
  BodyStyle.SetFontFamily("Verdana");

  // Global style for TD elements.
  asiTestEngine_ReportStyle CellStyle;
  CellStyle.SetFontSize(11);

  // Global style for header cells.
  asiTestEngine_ReportStyle HCellStyle;
  HCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(215, 215, 200) );

  // Global style for TD elements for "good" results.
  asiTestEngine_ReportStyle GoodCellStyle;
  GoodCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(35, 160, 100) );

  // Global style for TD elements for "expected bad" results.
  asiTestEngine_ReportStyle BadCellStyle;
  BadCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(240, 240, 115) );

  // Global style for TD elements for "failure" results.
  asiTestEngine_ReportStyle FailureCellStyle;
  FailureCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(240, 115, 115) );

  // Global style for TD elements for "no reference data" results.
  asiTestEngine_ReportStyle NoRefCellStyle;
  NoRefCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(190, 190, 190) );

  // Global style for TD elements for "generate reference data" results.
  asiTestEngine_ReportStyle GenRefCellStyle;
  GenRefCellStyle.SetBgColor( asiTestEngine_ReportStyle::Color(190, 190, 240) );

  // Global style for tables.
  asiTestEngine_ReportStyle TableStyle;
  TableStyle.SetBorder(1);
  TableStyle.SetPadding(5);

  // Generate HTML heading section.
  Rdr->AddDoctype()
     ->StartHtml()
     //
     ->StartHeader()
     ->AddMeta()
     ->StartStyle()
     ->AddClass("body_class",         BodyStyle)
     ->AddClass("table_class",        TableStyle)
     ->AddClass("cell_class",         CellStyle)
     ->AddClass("good_cell_class",    GoodCellStyle)
     ->AddClass("bad_cell_class",     BadCellStyle)
     ->AddClass("failure_cell_class", FailureCellStyle)
     ->AddClass("noref_cell_class",   NoRefCellStyle)
     ->AddClass("genref_cell_class",  GenRefCellStyle)
     ->AddClass("header_cell_class",  HCellStyle)
     ->EndStyle()
     ->EndHeader()
     //
     ->StartBody("body_class");

  // Add summary.
  Rdr->StartTable("table_class")
     //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class")
     ->AddText("Total cases")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numSucceeded + numFailed + numBad + numNoref + numGenref)
     ->EndTableHCell()
     ->EndTableRow()
     //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class good_cell_class")
     ->AddText("Num. succeeded")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numSucceeded)
     ->EndTableHCell()
     ->EndTableRow()
     //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class failure_cell_class")
     ->AddText("Num. failed")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numFailed)
     ->EndTableHCell()
     ->EndTableRow()
     //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class bad_cell_class")
     ->AddText("Num. bad")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numBad)
     ->EndTableHCell()
     ->EndTableRow()
     //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class noref_cell_class")
     ->AddText("Num. noref")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numNoref)
     ->EndTableHCell()
     ->EndTableRow()
    //
     ->StartTableRow()
     ->StartTableHCell("table_class cell_class genref_cell_class")
     ->AddText("Num. generated refs")
     ->EndTableHCell()
     ->StartTableHCell("table_class cell_class")
     ->AddText(numGenref)
     ->EndTableHCell()
     ->EndTableRow()
     //
     ->EndTable()
     ->BreakRow();

  // Generate table header.
  Rdr->StartTable("table_class")
     ->StartTableRow();
  //
  for ( asiTestEngine_ResultGroups::Iterator git(resultGroups);
        git.More(); git.Next() )
  {
     Rdr->StartTableHCell("table_class cell_class")
        ->AddText( git.GetName() )
        ->EndTableHCell();
  }
  //
  Rdr->EndTableRow();

  /*
   * Render information for each file.
   */

  // Get max number of results in groups.
  int maxNumRes = 0;
  for ( asiTestEngine_ResultGroups::Iterator git(resultGroups);
        git.More(); git.Next() )
  {
    const asiTestEngine_ResultGroup& group = git.GetGroup();

    if ( int( group.outFilenames.size() ) > maxNumRes )
      maxNumRes = int( group.outFilenames.size() );
  }

  // Dump results by groups.
  for ( int f = 0; f < maxNumRes; ++f )
  {
    Rdr->StartTableRow();

    // Dump references to the corresponding pages.
    for ( asiTestEngine_ResultGroups::Iterator git(resultGroups);
          git.More(); git.Next() )
    {
      const asiTestEngine_ResultGroup& group = git.GetGroup();

      if ( f < int( group.outFilenames.size() ) )
      {
        TCollection_AsciiString
          title = asiAlgo_Utils::Str::BaseFilename(group.outFilenames[f], false);
        //
        if ( group.multiBodyFlags[f] )
          title += " (*)"; // To denote multi-body case.

        TCollection_AsciiString url   = TCollection_AsciiString("./files/")
                                      + asiAlgo_Utils::Str::BaseFilename(group.outFilenames[f], true);

        TCollection_AsciiString cellClass;
        //
        if ( group.outStatuses[f] == asiTestEngine_Status::Ok )
          cellClass = "good_cell_class";
        else if ( group.outStatuses[f] == asiTestEngine_Status::Failed )
          cellClass = "failure_cell_class";
        else if ( group.outStatuses[f] == asiTestEngine_Status::Bad )
          cellClass = "bad_cell_class";
        else if ( group.outStatuses[f] == asiTestEngine_Status::NoRef )
          cellClass = "noref_cell_class";
        else if ( group.outStatuses[f] == asiTestEngine_Status::GenRef )
          cellClass = "genref_cell_class";

        // Render header block.
        Rdr->StartTableCell( (TCollection_AsciiString("table_class cell_class ") + cellClass).ToCString() );
        //
        Rdr->AddHRef( url.ToCString(), title.ToCString() );
        //
        Rdr->EndTableHCell();
      }
      else
        Rdr->StartTableCell("table_class cell_class")
           ->AddText("")
           ->EndTableHCell();
    }

    Rdr->EndTableRow();
  }

  // Finish table.
  Rdr->EndTable();

  // Finalize HTML.
  Rdr->EndBody()->EndHtml();

  // Prepare filename for HTML report.
  std::string
    filename = asiAlgo_Utils::Str::Slashed( outputDir.ToCString() ) + "index.html";

  // Create file for HTML report.
  std::ofstream file;
  file.open(filename, std::ios::out | std::ios::trunc);
  //
  if ( !file.is_open() )
    return false;

  // Dump rendered information to file.
  file << Rdr->Flush();

  // Release file.
  file.close();

  return true;
}

//! Returns "some topological shape" for you. Useful if you do not care
//! of the actual topological data but do need to have something valid.
//! \return OCCT topological shape.
TopoDS_Shape asiTestEngine_Utils::RandomShape()
{
  int RAND_INDX = rand() % 10;
  TopoDS_Shape aResult;
  switch ( RAND_INDX )
  {
    case 0:
      aResult = BRepPrimAPI_MakeBox(100, 100, 100);
      break;
    case 1:
      aResult = BRepPrimAPI_MakeBox(5, 20, 40);
      break;
    case 2:
      aResult = BRepPrimAPI_MakeCone(10, 5, 10);
      break;
    case 3:
      aResult = BRepPrimAPI_MakeCone(5, 25, 20);
      break;
    case 4:
      aResult = BRepPrimAPI_MakeCylinder(10, 20);
      break;
    case 5:
      aResult = BRepPrimAPI_MakeSphere(10, 2*M_PI / 3);
      break;
    case 6:
      aResult = BRepPrimAPI_MakeSphere(10, 7*M_PI / 8);
      break;
    case 7:
      aResult = BRepPrimAPI_MakeTorus(5, 10);
      break;
    case 8:
      aResult = BRepPrimAPI_MakeTorus(10, 15, M_PI / 3);
      break;
    case 9:
      aResult = BRepPrimAPI_MakeTorus(10, 15, M_PI / 2);
      break;
  }
  return aResult;
}

//! Returns "some Boolean value" for you.
//! \return Boolean value.
bool asiTestEngine_Utils::RandomBoolean()
{
  int RAND_INDX = rand() % 10;

  if ( RAND_INDX < 5 )
    return true;

  return false;
}

//! Returns "some integer value" for you.
//! \return integer value.
int asiTestEngine_Utils::RandomInteger()
{
  int RAND_INDX = rand() % 10;
  return RAND_INDX;
}

//! Returns "some real value" for you.
//! \return real value.
double asiTestEngine_Utils::RandomReal()
{
  int RAND_INDX = rand() % 10;
  double aResult = RAND_INDX * 1.5;
  return aResult;
}

//! Returns "some integer array" for you.
//! \param theLower [in] lower index.
//! \param theUpper [in] upper index.
//! \return integer array.
Handle(HIntArray) asiTestEngine_Utils::RandomIntegerArray(const int theLower,
                                                          const int theUpper)
{
  Handle(HIntArray) aResArr = new HIntArray(theLower, theUpper);
  for ( int i = theLower; i <= theUpper; ++i )
    aResArr->SetValue( i, RandomInteger() );

  return aResArr;
}

//! Returns "some Boolean array" for you.
//! \param theLower [in] lower index.
//! \param theUpper [in] upper index.
//! \return Boolean array.
Handle(HBoolArray) asiTestEngine_Utils::RandomBooleanArray(const int theLower,
                                                           const int theUpper)
{
  Handle(HBoolArray) aResArr = new HBoolArray(theLower, theUpper);
  for ( int i = theLower; i <= theUpper; ++i )
    aResArr->SetValue( i, RandomBoolean() );

  return aResArr;
}

//! Returns "some real array" for you.
//! \param theLower [in] lower index.
//! \param theUpper [in] upper index.
//! \return real array.
Handle(HRealArray) asiTestEngine_Utils::RandomRealArray(const int theLower,
                                                        const int theUpper)
{
  Handle(HRealArray) aResArr = new HRealArray(theLower, theUpper);
  for ( int i = theLower; i <= theUpper; ++i )
    aResArr->SetValue( i, RandomReal() );

  return aResArr;
}
