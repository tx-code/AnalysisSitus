/***************************************************************************
 *   Copyright (c) OPEN CASCADE SAS                                        *
 *                                                                         *
 *   This file is part of Open CASCADE Technology software library.        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 ***************************************************************************/

#include <DFBrowser.hxx>

#include <TDocStd_Document.hxx>
#include <DFBrowser_DFTree.hxx>

Handle(DFBrowser_DFTree) myTree;

#pragma warning(push, 0)
#include <QApplication>
#pragma warning(pop)

//======================================================================= 
//function : DFBrowser
//purpose  :  
//  arg 1  : Document
//=======================================================================

Standard_Integer DFBrowser::DFBrowserCall (const Handle(TDocStd_Document)& theDoc)

{
  if (DFBrowser_DFTree::NbBrowsers() > 0) {
    std::cout<<"DFBrowser : Error   : DFBrowser is already launched"<<std::endl;
    return 1;
  }
/*  */
//#ifndef WNT  
  char *b = new char[2];
  b[0]='a';b[1]=0;
  //QApplication *app = new QApplication(a,&b);
 //QApplication app(a,&b);
//#endif
 //*/
  myTree = new DFBrowser_DFTree(theDoc);

//   strcpy(myBrowserName,MR_DFBROWSER);
//   if (myDoc->IsSaved()) strcat(myBrowserName,(char *)(myDoc->GetName().ToExtString()));
//   else strcat(myBrowserName,"document");
  myTree->Update();

//  app->exec();////
  //app.exec();
  return 0; 
}

//==============================================================================
// DFBrowser::Factory
//==============================================================================
Standard_Integer DFBrowser::Factory(const Handle(TDocStd_Document)& theDoc)
{
  static Standard_Boolean aFactoryDFB = Standard_False;
  if (aFactoryDFB) return 1;
  aFactoryDFB = Standard_True;

  if(!DFBrowser::DFBrowserCall (theDoc) ) {
#ifdef DEB
    cout << "DFBrowser Plugin : DFBrowser commands are loaded" << endl;
#endif
  }
  return 0;
}

//int main(int argc, char **argv)
//{
//  char BUFF[1024];
//  GetEnvironmentVariable("THIS_DATA", BUFF, 1024);
//  TCollection_AsciiString fn =  BUFF;
//
//  const Handle(ActData_Application)& anApp = ActData_Application::Instance();
//
//  Handle(TDocStd_Document) aDoc;
//  ActData_Application::Instance()->NewDocument("BinOcaf", aDoc);
//
//  PCDM_ReaderStatus aStatus = PCDM_RS_OpenError;
//  try
//  {
//    aStatus = anApp->Open(fn, aDoc);
//  }
//  catch ( Standard_Failure exc )
//  {
//    cout << "OCCT exception:" << endl;
//    cout << exc.DynamicType()->Name() << endl;
//    cout << exc.GetMessageString() << endl;
//    return Standard_False;
//  }
//
//  if ( aStatus != PCDM_RS_OK )
//  {
//    std::cout << "Error: unrecognized format" << std::endl;
//    getchar();
//    return Standard_False;
//  }
//
//  //
//	return DFBrowser::Factory(aDoc);
//}