//-----------------------------------------------------------------------------
// Created on: 21 November 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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

#ifndef asiAlgo_Message_HeaderFile
#define asiAlgo_Message_HeaderFile

// Own includes
#include <asiAlgo.h>
//
// OCCT includes
#include <Message.hxx>
#include <TopoDS_AlertAttribute.hxx>
#include <TopoDS_Shape.hxx>

//! Class to declare message functions
class asiAlgo_Message
{
public:
  //! Sends the text to default messenger
  //! @param text the string value
  static void sendMessage(const TCollection_AsciiString& text)
  {
    Message::SendInfo() << text << std::endl;
  }

  //! Sends the shape with the text to default messenger
  //! @param the text the string value, noting sent if empty
  //! @param shape the shape value
  static void sendShapeMessage(const TCollection_AsciiString& text,
                               const TopoDS_Shape& shape)
  {
    if (!text.IsEmpty())
    {
      Message::SendInfo() << text << std::endl;
    }
    Message::DefaultMessenger() << shape;
  }
};

#define MESSAGE_INFO_CUSTOM(Name)              asiAlgo_Message::sendMessage(Name);
#define MESSAGE_INFO_LEVEL(Name)               OCCT_ADD_MESSAGE_LEVEL_SENTRY(Name)
#define MESSAGE_INFO_SHAPE_CUSTOM(Shape, Name) asiAlgo_Message::sendShapeMessage(Name, Shape);

#endif