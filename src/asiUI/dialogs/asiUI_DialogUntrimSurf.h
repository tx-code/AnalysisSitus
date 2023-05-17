//-----------------------------------------------------------------------------
// Created on: 17 May 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

#ifndef asiUI_DialogUntrimSurf_h
#define asiUI_DialogUntrimSurf_h

// asiUI includes
#include <asiUI_LineEdit.h>
#include <asiUI_ViewerPart.h>
#include <asiUI_WidgetFactory.h>

// asiEngine include
#include <asiEngine_Model.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

//! Dialog for surface untrimming.
class asiUI_DialogUntrimSurf : public QDialog
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] wf       the widget factory.
  //! \param[in] model    the data model instance.
  //! \param[in] pViewer  the connected viewer.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  //! \param[in] parent   the parent widget.
  asiUI_EXPORT
    asiUI_DialogUntrimSurf(const Handle(asiUI_WidgetFactory)& wf,
                           const Handle(asiEngine_Model)&     model,
                           asiUI_ViewerPart*                  pViewer,
                           ActAPI_ProgressEntry               progress,
                           ActAPI_PlotterEntry                plotter,
                           QWidget*                           parent = nullptr);

  //! Dtor.
  asiUI_EXPORT virtual
    ~asiUI_DialogUntrimSurf();

public slots:

  //! Reaction on edge selection in the viewer.
  void onEdgePicked();

  //! Reaction on face selection in the viewer.
  void onFacePicked();

  //! Reaction on clicking "Apply" button.
  void onApply();

protected:

  //! Widgets.
  struct t_widgets
  {
    QPushButton*    pApply;     //!< Performs untrimming.
    QPushButton*    pClose;     //!< Closes the dialog.
    asiUI_LineEdit* pFaces;     //!< Indices of the faces to untrim.
    asiUI_LineEdit* pEdges;     //!< Indices of the boundary edges.
    asiUI_Datum*    pNumUSpans; //!< Number of U spans.
    asiUI_Datum*    pNumVSpans; //!< Number of V spans.
    asiUI_Datum*    pUDegree;   //!< U degree.
    asiUI_Datum*    pVDegree;   //!< V degree.

    //! Default ctor.
    t_widgets() : pApply     (nullptr),
                  pClose     (nullptr),
                  pFaces     (nullptr),
                  pEdges     (nullptr),
                  pNumUSpans (nullptr),
                  pNumVSpans (nullptr),
                  pUDegree   (nullptr),
                  pVDegree   (nullptr)
    {}

    void Release()
    {
      delete pApply;     pApply     = nullptr;
      delete pClose;     pClose     = nullptr;
      delete pFaces;     pFaces     = nullptr;
      delete pEdges;     pEdges     = nullptr;
      delete pNumUSpans; pNumUSpans = nullptr;
      delete pNumVSpans; pNumVSpans = nullptr;
      delete pUDegree;   pUDegree   = nullptr;
      delete pVDegree;   pVDegree   = nullptr;
    }
  };

  t_widgets               m_widgets;     //!< UI controls.
  QVBoxLayout*            m_pMainLayout; //!< Layout of the widget.
  asiUI_ViewerPart*       m_pViewer;     //!< External reference to viewer.
  Handle(asiEngine_Model) m_model;       //!< Data Model instance.

  /* Diagnostics */

  ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif // asiUI_DialogUntrimSurf_h
