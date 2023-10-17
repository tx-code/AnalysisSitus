//-----------------------------------------------------------------------------
// Created on: August 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2012-present, Sergey Slyadnev
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

#ifndef asiUI_InputEventFilter_HeaderFile
#define asiUI_InputEventFilter_HeaderFile

#include <asiUI_OverrideCursor.h>

// Qt includes
#pragma warning(push, 0)
#include <QCursor>
#include <QPointer>
#include <QObject>
#include <QMap>
#include <QSet>
#include <QEvent>
#pragma warning(pop)

//! Event filter that blocks out user input events such as mouse clicks, or keyboard input.
//! Originally this filter is designed for disabling GUI elements of application instance
//! during background task execution.
//!
//! This class follows RAII idiom, i.e., to use it, just allocate an instance of this
//! class on the stack memory and add objects to filter. Once destroyed, this filter
//! will "release" all "locked" objects automatically.
class asiUI_InputEventFilter : public QObject
{
  Q_OBJECT
  
public:

  //! Ctor.
  asiUI_EXPORT
    asiUI_InputEventFilter();

  //! Dtor.
  asiUI_EXPORT virtual
    ~asiUI_InputEventFilter();

public:

  asiUI_EXPORT virtual bool
    eventFilter(QObject* theReceiver, QEvent* theEvent);

public:

  asiUI_EXPORT void
    AddObject(QObject* pObject);

  asiUI_EXPORT bool
    HasWatchedParents() const;

  asiUI_EXPORT void
    ExcludeObject(QObject* theObject);

private slots:

  void onObjectDestroyed(QObject* theObject);

  void onParentDestroyed(QObject* theObject);

  void onDesktopActivated();

  void onDesktopDeactivated();

private:

  void postponeEvent(QObject* theWatchedParent,
                     QObject* theReceiver,
                     QEvent*  theEvent);

  void postDelayedEvents(QObject* theWatchedParent);

// instance data
private:

  typedef QSet<QEvent::Type>             EventType;
  typedef QSet<QObject*>                 ObjectSet;
  typedef QSet<QObject*>                 ApplicationSet;
  typedef QMap<QObject*, QEvent*>        ObjectEventMap;
  typedef QMap<QObject*, ObjectEventMap> DelayedEventMap;

private:

  // map of filtered of delayed events
  EventType m_TypeOfDelayedEvents;
  EventType m_TypeOfFilteredEvents;
  
  // map of events to be sent, when
  // filter is removed from some object
  DelayedEventMap m_DelayedEventMap;

  // map of objects excluded from event
  // filtering
  ObjectSet m_ExcludedObjects;

  // map of watched parent applications
  ObjectSet m_WatchedParents;

  asiUI_OverrideCursor m_pCursor;
};

#endif
