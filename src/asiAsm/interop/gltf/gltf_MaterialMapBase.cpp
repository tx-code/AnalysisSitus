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

// Own include
#include <gltf_MaterialMapBase.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Label.hxx>

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialMapBase::gltf_MaterialMapBase(const TCollection_AsciiString& theFile)
: myFileName (theFile),
  myKeyPrefix ("mat_"),
  myNbMaterials (0),
  myIsFailed (false),
  myMatNameAsKey (true)
{
  TCollection_AsciiString aFileName, aFileExt;
  OSD_Path::FolderAndFileFromPath(theFile, myFolder, aFileName);
  asiAlgo_Utils::Str::FileNameAndExtension(aFileName, myShortFileNameBase, aFileExt);
}

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialMapBase::~gltf_MaterialMapBase()
{
  //
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAsm::gltf_MaterialMapBase::AddMaterial(const gltf_XdeVisualStyle& theStyle)
{
  if (myStyles.IsBound1 (theStyle))
  {
    return myStyles.Find1 (theStyle);
  }

  TCollection_AsciiString aMatKey, aMatName, aMatNameSuffix;
  int  aCounter    = 0;
  int* aCounterPtr = &myNbMaterials;
  if (myMatNameAsKey)
  {
    if (!theStyle.GetMaterial().IsNull()
     && !theStyle.GetMaterial()->IsEmpty())
    {
      aCounterPtr = &aCounter;
      Handle(TDataStd_Name) aNodeName;
      if (!theStyle.GetMaterial()->Label().IsNull()
       &&  theStyle.GetMaterial()->Label().FindAttribute (TDataStd_Name::GetID(), aNodeName))
      {
        aMatName = aNodeName->Get();
      }
      else
      {
        aMatName = "mat";
      }
      aMatNameSuffix = aMatName;
    }
    else
    {
      ++myNbMaterials;
      aMatNameSuffix = myKeyPrefix;
      aMatName = aMatNameSuffix + myNbMaterials;
    }
    aMatKey = aMatName;
  }
  else
  {
    aMatKey        = myNbMaterials++; // starts from 0
    aMatNameSuffix = myKeyPrefix;
    aMatName       = aMatNameSuffix + aMatKey;
  }

  for (;; ++(*aCounterPtr))
  {
    if (myStyles.IsBound2 (aMatKey))
    {
      if (myMatNameAsKey)
      {
        aMatName = aMatNameSuffix + (*aCounterPtr);
        aMatKey  = aMatName;
      }
      else
      {
        aMatKey  = *aCounterPtr;
        aMatName = aMatNameSuffix + aMatKey;
      }
      continue;
    }
    break;
  }

  myStyles.Bind (theStyle, aMatKey);
  DefineMaterial (theStyle, aMatKey, aMatName);
  return aMatKey;
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_MaterialMapBase::copyFileTo(const TCollection_AsciiString& theFileSrc,
                                              const TCollection_AsciiString& theFileDst)
{
  if (theFileSrc.IsEmpty()
   || theFileDst.IsEmpty())
  {
    return false;
  }
  else if (theFileSrc == theFileDst)
  {
    return true;
  }

  try
  {
    OSD_Path aSrcPath (theFileSrc);
    OSD_Path aDstPath (theFileDst);
    OSD_File aFileSrc (aSrcPath);
    if (!aFileSrc.Exists())
    {
      std::cout << "Failed to copy file - source file '" << theFileSrc + "' does not exist." << std::endl;
      return false;
    }
    aFileSrc.Copy (aDstPath);
    return !aFileSrc.Failed();
  }
  catch (Standard_Failure const& theException)
  {
    std::cout << "Failed to copy file: " << theException.GetMessageString() << std::endl;
    return false;
  }
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_MaterialMapBase::CopyTexture(TCollection_AsciiString&       theResTexture,
                                               const Handle(Image_Texture)&   theTexture,
                                               const TCollection_AsciiString& theKey)
{
  CreateTextureFolder();

  TCollection_AsciiString aTexFileName;
  TCollection_AsciiString aTextureSrc = theTexture->FilePath();
  if (!aTextureSrc.IsEmpty()
    && theTexture->FileOffset() <= 0
    && theTexture->FileLength() <= 0)
  {
    TCollection_AsciiString aSrcTexFolder;
    OSD_Path::FolderAndFileFromPath (aTextureSrc, aSrcTexFolder, aTexFileName);
    const TCollection_AsciiString aResTexFile = myTexFolder + aTexFileName;
    theResTexture = myTexFolderShort + aTexFileName;
    return copyFileTo (aTextureSrc, aResTexFile);
  }

  TCollection_AsciiString anExt = theTexture->ProbeImageFileFormat();
  if (anExt.IsEmpty())
  {
    anExt = "bin";
  }
  aTexFileName = theKey + "." + anExt;

  const TCollection_AsciiString aResTexFile = myTexFolder + aTexFileName;
  theResTexture = myTexFolderShort + aTexFileName;
  return theTexture->WriteImage (aResTexFile);
}

//-----------------------------------------------------------------------------

bool asiAsm::gltf_MaterialMapBase::CreateTextureFolder()
{
  if (!myTexFolder.IsEmpty())
  {
    return true;
  }

  myTexFolderShort = myShortFileNameBase + "_textures/";
  myTexFolder      = myFolder + "/" + myTexFolderShort;
  OSD_Path aTexFolderPath (myTexFolder);
  OSD_Directory aTexDir (aTexFolderPath);
  if (aTexDir.Exists())
  {
    return true;
  }

  OSD_Path aResFolderPath (myFolder);
  OSD_Directory aResDir (aResFolderPath);
  if (!aResDir.Exists())
  {
    return false;
  }
  const OSD_Protection aParentProt = aResDir.Protection();
  OSD_Protection aProt = aParentProt;
  if (aProt.User() == OSD_None)
  {
    aProt.SetUser (OSD_RWXD);
  }
  if (aProt.System() == OSD_None)
  {
    aProt.SetSystem (OSD_RWXD);
  }

  aTexDir.Build (aProt);
  if (aTexDir.Failed())
  {
    // fallback to the same folder as output model file
    myTexFolder = myFolder;
    myTexFolderShort.Clear();
    return true;
  }
  return true;
}
