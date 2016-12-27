//-----------------------------------------------------------------------------
// Created on: 27 December 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// asiUI includes
#include <asiUI_StatusBarImpl.h>
#include <asiUI_StatusBar.h>

//-----------------------------------------------------------------------------

//! Creates implementation and adds controls to the status bar.
//! \param statusBar [in] window's status bar.
asiUI_StatusBarImpl::asiUI_StatusBarImpl(asiUI_StatusBar* statusBar)
: asiUI_IStatusBar(),
  m_pStatusBar(statusBar)
{}

//-----------------------------------------------------------------------------

//! Shows the passed text as a status hint.
//! \param statusText [in] the text to be shown in status bar.
void asiUI_StatusBarImpl::SetStatusText(const TCollection_AsciiString& statusText)
{
  if ( m_pStatusBar )
    m_pStatusBar->showMessage( AsciiStr2QStr(statusText) );
}

//-----------------------------------------------------------------------------

//! Shows/hides the progress indicator in the status bar.
//! \param isOn [in] true/false.
void asiUI_StatusBarImpl::ShowProgress(const bool isOn)
{
  if ( m_pStatusBar )
    m_pStatusBar->ShowProgress(isOn);

  if ( !isOn )
  {
    this->SetProgressText("");
    this->UpdateProgress(0);
  }
}

//-----------------------------------------------------------------------------

//! Updates the progress indicator using specified percentage value.
//! \param percentage [in] percentage value to set.
void asiUI_StatusBarImpl::UpdateProgress(const int percentage)
{
  if ( m_pStatusBar )
    m_pStatusBar->UpdateProgress(percentage);
}

//-----------------------------------------------------------------------------

//! Sets text for progress bar widget.
//! \param progressText [in] text to display.
void asiUI_StatusBarImpl::SetProgressText(const TCollection_AsciiString& progressText)
{
  if ( m_pStatusBar )
    m_pStatusBar->SetText( AsciiStr2QStr(progressText) );
}

//-----------------------------------------------------------------------------

//! Sets progress bar to infinite mode.
//! \param isInfinite [in] true/false.
void asiUI_StatusBarImpl::SetProgressInfinite(const bool isInfinite)
{
  if ( m_pStatusBar )
    m_pStatusBar->SetProgressInfinite(isInfinite);
}

//-----------------------------------------------------------------------------

//! Shows the passed interactive information message.
//! \param textMsg [in] text to show as an interactive message.
void asiUI_StatusBarImpl::SetInfoMessage(const TCollection_AsciiString& textMsg)
{
  if ( m_pStatusBar )
    m_pStatusBar->SetInfoMessage( AsciiStr2QStr(textMsg) );
}

//-----------------------------------------------------------------------------

//! \return reference to a status bar widget. QObjects can use this pointer
//!         for binding custom reactions.
asiUI_StatusBar* asiUI_StatusBarImpl::GetStatusBar() const
{
  return m_pStatusBar;
}
