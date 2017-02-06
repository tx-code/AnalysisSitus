//-----------------------------------------------------------------------------
// Created on: 04 December 2015
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiUI_Viewer_h
#define asiUI_Viewer_h

// A-Situs includes
#include <asiUI.h>

// A-Situs (visualization) includes
#include <asiVisu_PrsManager.h>

// Qt includes
#pragma warning(push, 0)
#include <QWidget>
#pragma warning(pop)

//! Base class for all viewers.
class asiUI_EXPORT asiUI_Viewer : public QWidget
{
  Q_OBJECT

public:

  asiUI_Viewer(QWidget* parent = NULL);

  virtual ~asiUI_Viewer();

public:

  virtual void Repaint() = 0;

public:

  const vtkSmartPointer<asiVisu_PrsManager>& PrsMgr() const { return m_prs_mgr; }

protected:

  vtkSmartPointer<asiVisu_PrsManager> m_prs_mgr; //!< Presentation Manager.

};

#endif
