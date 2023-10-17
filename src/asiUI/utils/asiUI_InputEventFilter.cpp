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

// Own include
#include <asiUI_InputEventFilter.h>

// asiUI includes
#include <asiUI_OverrideCursor.h>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Constructor.
asiUI_InputEventFilter::asiUI_InputEventFilter()
: QObject(NULL),
  m_pCursor()
{
  // events that will be filtered out
  for ( int evt = 0; evt < QEvent::User; ++evt )
    m_TypeOfFilteredEvents.insert( (QEvent::Type) evt );

  /*
    You may want to exclude specific event types.

    m_TypeOfFilteredEvents.insert(QEvent::Shortcut);
    m_TypeOfFilteredEvents.insert(QEvent::KeyPress);
    m_TypeOfFilteredEvents.insert(QEvent::KeyRelease);
    m_TypeOfFilteredEvents.insert(QEvent::ContextMenu);
    m_TypeOfFilteredEvents.insert(QEvent::MouseMove);
    m_TypeOfFilteredEvents.insert(QEvent::MouseButtonPress);
    m_TypeOfFilteredEvents.insert(QEvent::MouseButtonRelease);
    m_TypeOfFilteredEvents.insert(QEvent::MouseButtonDblClick);
    m_TypeOfFilteredEvents.insert(QEvent::Resize);

    // events that will be postponed until the event
    // filter is detached.
    m_TypeOfDelayedEvents.insert(QEvent::DeferredDelete);
  */

  m_pCursor.suspend();
  QApplication::instance()->installEventFilter(this);
}

//! Destructor.
asiUI_InputEventFilter::~asiUI_InputEventFilter()
{
  QApplication::instance()->removeEventFilter(this);
}

//! filter user input events.
//! \param theReceiver [in] the event receiver.
//! \param theEvent [in] the received event.
bool asiUI_InputEventFilter::eventFilter(QObject* theReceiver, QEvent* theEvent)
{
  bool isFiltered = m_TypeOfFilteredEvents.contains(theEvent->type());
  bool isDelayed  = !isFiltered && m_TypeOfDelayedEvents.contains(theEvent->type());

  if ( isFiltered || isDelayed )
  {
    // check parentship of the object.
    QObject* aParent = theReceiver;
    while ( aParent )
    {
      if ( m_ExcludedObjects.contains(aParent) )
        return QObject::eventFilter(theReceiver, theEvent);

      if ( m_WatchedParents.contains(aParent) )
      {
        if ( isDelayed )
        {
          postponeEvent(aParent, theReceiver, theEvent);
        }

        // do not propagate event any further if the object is managed by the filter.
        theEvent->ignore();
        return true;
      }

      aParent = aParent->parent();
    }
  }

  return QObject::eventFilter(theReceiver, theEvent);
}

void asiUI_InputEventFilter::AddObject(QObject* pObject)
{
  m_WatchedParents.insert(pObject);
}

//! Check whether the event filter has any of the parent qobject instances
//! under its control.
//! \return boolean flag indicating whether the filter is installed for
//!         any of the objects or not
bool asiUI_InputEventFilter::HasWatchedParents() const
{
  return m_WatchedParents.size() > 0;
}

//! Exclude object from being filtered.
//! \param theObj [in] the interaction with the object will be enabled.
void asiUI_InputEventFilter::ExcludeObject(QObject* theObject)
{
  if ( !theObject )
    return;

  m_ExcludedObjects.insert(theObject);
  connect(theObject, SIGNAL(destroyed(QObject*)),
          this,      SLOT(onObjectDestroyed(QObject*)));
}

//! Remove object from internal map when its destroyed.
//! \param theObject [in] the destroyed object.
void asiUI_InputEventFilter::onObjectDestroyed(QObject* theObject)
{
  m_ExcludedObjects.remove(theObject);
}

//! Remove application instance from internal map when its destroyed.
//! \param theObject [in] the destroyed object.
void asiUI_InputEventFilter::onParentDestroyed(QObject* theObject)
{
  m_WatchedParents.remove(theObject);
  m_DelayedEventMap.remove(theObject);
}

void asiUI_InputEventFilter::onDesktopActivated()
{
  if ( m_WatchedParents.contains(sender()) )
    m_pCursor.resume();
}

void asiUI_InputEventFilter::onDesktopDeactivated()
{
  if ( m_WatchedParents.contains(sender()) )
    m_pCursor.suspend();
}

//! Copy and postpone event until the event filter is removed from
//! watched parent object.
//! \param theWatchedParent [in] the watched parent.
//! \param theReceiver [in] the event receiver.
//! \param theEvent [in] the postponed event
void asiUI_InputEventFilter::postponeEvent(QObject* theWatchedParent,
                                           QObject* theReceiver,
                                           QEvent* theEvent)
{
  if ( !m_DelayedEventMap.contains(theWatchedParent) )
    m_DelayedEventMap.insert(theWatchedParent, ObjectEventMap());

  ObjectEventMap& aEventMap = m_DelayedEventMap[theWatchedParent];

  QEvent* aClone = 0;

  // clone the event
  if ( theEvent->type() == QEvent::DeferredDelete )
  {
    aClone = new QEvent(theEvent->type());
  }

  if ( aClone != 0 )
    aEventMap.insert(theReceiver, aClone);
}

//! Post all delayed events for the childs of the watched parent.
//! \param theWatchedParent [in] the watched parent object.
void asiUI_InputEventFilter::postDelayedEvents(QObject* theWatchedParent)
{
  if ( !m_DelayedEventMap.contains(theWatchedParent) )
    return;

  ObjectEventMap aEventMap = m_DelayedEventMap.take(theWatchedParent);
  ObjectEventMap::Iterator anIt = aEventMap.begin();
  for ( ; anIt != aEventMap.end(); anIt++ )
  {
    QApplication::postEvent(anIt.key(), anIt.value());
  }
}
