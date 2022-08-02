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
#include <asiAsm_GLTFMaterialMapBase.h>

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

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

glTFMaterialMapBase::glTFMaterialMapBase(const TCollection_AsciiString& file)
//
: m_fileName     (file),
  m_keyPrefix    ("mat_"),
  m_nbMaterials  (0),
  m_bIsFailed    (false),
  m_matNameAsKey (true)
{
  TCollection_AsciiString fileName, fileExt;
  OSD_Path::FolderAndFileFromPath(file, m_folder, fileName);
  asiAlgo_Utils::Str::FileNameAndExtension(fileName, m_shortFileNameBase, fileExt);
}

//-----------------------------------------------------------------------------

glTFMaterialMapBase::~glTFMaterialMapBase()
{}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  glTFMaterialMapBase::AddMaterial(const glTFXdeVisualStyle& style)
{
  if ( m_styles.IsBound1(style) )
  {
    return m_styles.Find1(style);
  }

  TCollection_AsciiString matKey, matName, matNameSuffix;
  int  counter     = 0;
  int* pCounterPtr = &m_nbMaterials;
  //
  if ( m_matNameAsKey )
  {
    if ( !style.GetMaterial().IsNull() && !style.GetMaterial()->IsEmpty() )
    {
      pCounterPtr = &counter;
      Handle(TDataStd_Name) nodeName;
      //
      if ( !style.GetMaterial()->Label().IsNull()
        &&  style.GetMaterial()->Label().FindAttribute(TDataStd_Name::GetID(), nodeName) )
      {
        matName = nodeName->Get();
      }
      else
      {
        matName = "mat";
      }
      matNameSuffix = matName;
    }
    else
    {
      ++m_nbMaterials;

      matNameSuffix = m_keyPrefix;
      matName       = matNameSuffix + m_nbMaterials;
    }
    matKey = matName;
  }
  else
  {
    matKey        = m_nbMaterials++; // starts from 0
    matNameSuffix = m_keyPrefix;
    matName       = matNameSuffix + matKey;
  }

  for ( ; ; ++(*pCounterPtr) )
  {
    if ( m_styles.IsBound2(matKey) )
    {
      if ( m_matNameAsKey )
      {
        matName = matNameSuffix + (*pCounterPtr);
        matKey  = matName;
      }
      else
      {
        matKey  = *pCounterPtr;
        matName =  matNameSuffix + matKey;
      }
      continue;
    }
    break;
  }

  m_styles.Bind(style, matKey);
  DefineMaterial(style, matKey, matName);
  return matKey;
}

//-----------------------------------------------------------------------------

bool glTFMaterialMapBase::copyFileTo(const TCollection_AsciiString& fileSrc,
                                     const TCollection_AsciiString& fileDst)
{
  if ( fileSrc.IsEmpty() || fileDst.IsEmpty() )
  {
    return false;
  }
  else if ( fileSrc == fileDst )
  {
    return true;
  }

  try
  {
    OSD_Path SrcPath(fileSrc);
    OSD_Path DstPath(fileDst);
    OSD_File FileSrc(SrcPath);
    //
    if ( !FileSrc.Exists() )
    {
      std::cout << "Failed to copy file - source file '" << fileSrc + "' does not exist." << std::endl;
      return false;
    }

    FileSrc.Copy(DstPath);
    return !FileSrc.Failed();
  }
  catch ( Standard_Failure const& e )
  {
    std::cout << "Failed to copy file: " << e.GetMessageString() << std::endl;
    return false;
  }
}

//-----------------------------------------------------------------------------

bool glTFMaterialMapBase::CopyTexture(TCollection_AsciiString&       resTexture,
                                      const Handle(Image_Texture)&   texture,
                                      const TCollection_AsciiString& key)
{
  CreateTextureFolder();

  TCollection_AsciiString texFileName;
  TCollection_AsciiString textureSrc = texture->FilePath();
  //
  if ( !textureSrc.IsEmpty()
     && texture->FileOffset() <= 0
     && texture->FileLength() <= 0 )
  {
    TCollection_AsciiString srcTexFolder;
    OSD_Path::FolderAndFileFromPath(textureSrc, srcTexFolder, texFileName);

    const TCollection_AsciiString resTexFile = m_texFolder + texFileName;
    resTexture = m_texFolderShort + texFileName;
    return copyFileTo(textureSrc, resTexFile);
  }

  TCollection_AsciiString ext = texture->ProbeImageFileFormat();
  //
  if ( ext.IsEmpty() )
  {
    ext = "bin";
  }
  texFileName = key + "." + ext;

  const TCollection_AsciiString resTexFile = m_texFolder + texFileName;
  resTexture = m_texFolderShort + texFileName;
  return texture->WriteImage(resTexFile);
}

//-----------------------------------------------------------------------------

bool glTFMaterialMapBase::CreateTextureFolder()
{
  if ( !m_texFolder.IsEmpty() )
  {
    return true;
  }

  m_texFolderShort = m_shortFileNameBase + "_textures/";
  m_texFolder      = m_folder + "/" + m_texFolderShort;

  OSD_Path texFolderPath(m_texFolder);
  OSD_Directory texDir(texFolderPath);
  //
  if ( texDir.Exists() )
  {
    return true;
  }

  OSD_Path resFolderPath(m_folder);
  OSD_Directory resDir(resFolderPath);
  //
  if ( !resDir.Exists() )
  {
    return false;
  }

  const OSD_Protection parentProt = resDir.Protection();
  OSD_Protection prot = parentProt;
  //
  if ( prot.User() == OSD_None )
  {
    prot.SetUser(OSD_RWXD);
  }
  if ( prot.System() == OSD_None )
  {
    prot.SetSystem(OSD_RWXD);
  }

  texDir.Build(prot);
  if ( texDir.Failed() )
  {
    // fallback to the same folder as output model file
    m_texFolder = m_folder;
    m_texFolderShort.Clear();
    return true;
  }

  return true;
}
