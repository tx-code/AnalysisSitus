//-----------------------------------------------------------------------------
// Created on: 13 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Elizaveta Krylova
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

#ifndef asiUI_DialogBuildGordon_h
#define asiUI_DialogBuildGordon_h

// asiUI includes
#include <asiUI_LineEdit.h>
#include <asiUI_MobiusProgressNotifier.h>
#include <asiUI_ViewerPart.h>
#include <asiUI_WidgetFactory.h>

// asiEngine include
#include <asiEngine_Model.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QComboBox>
#include <QDialog>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

//! Dialog for Gordon surface building.
class asiUI_DialogBuildGordon : public QDialog
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
    asiUI_DialogBuildGordon(const Handle(asiUI_WidgetFactory)& wf,
                            const Handle(asiEngine_Model)&     model,
                            asiUI_ViewerPart*                  pViewer,
                            ActAPI_ProgressEntry               progress,
                            ActAPI_PlotterEntry                plotter,
                            QWidget*                           parent = nullptr);

  //! Dtor.
  asiUI_EXPORT
    virtual ~asiUI_DialogBuildGordon();

public slots:

  //! Reaction on edge selection in the viewer.
  void onEdgePicked(bool isProfile);

  //! Reaction on clicking "Apply" button.
  void onApply();

  //! Cancel button clicked.
  void onCancel();

  //! Profile edge selection button clicked.
  void onProfile();

  //! Guides edge selection button clicked.
  void onGuide();

protected:

  //! Widgets.
  struct t_widgets
  {
    QPushButton*      pApply;               //!< Apply surface fitting method.
    QPushButton*      pClose;               //!< Closes the dialog.
    QPushButton*      pChooseProfile;       //!< Choose profiles.
    QPushButton*      pChooseGuides;        //!< Choose guides.
    asiUI_LineEdit*   pProfiles;            //!< Indices of profile edges.
    asiUI_LineEdit*   pGuides;              //!< Indices of guide edges.
    asiUI_Datum*      pUDegree;             //!< U degree.
    asiUI_Datum*      pVDegree;             //!< V degree.
    asiUI_LineEdit*   pNumDiscrPoints;      //!< Number of discretisated points.
    QWidget*          pProgressFrame;       //!< Progress widget.
    QProgressBar*     pProgressBar;         //!< Progress bar.

    //! Default ctor.
    t_widgets() : pApply          (nullptr),
                  pClose          (nullptr),
                  pChooseProfile  (nullptr),
                  pChooseGuides   (nullptr),
                  pProfiles       (nullptr),
                  pGuides         (nullptr),
                  pUDegree        (nullptr),
                  pVDegree        (nullptr),
                  pNumDiscrPoints (nullptr),
                  pProgressFrame  (nullptr),
                  pProgressBar    (nullptr)
    {}

    void Release()
    {
      delete pApply;          pApply          = nullptr;
      delete pClose;          pClose          = nullptr;
      delete pChooseProfile;  pChooseProfile  = nullptr;
      delete pChooseGuides;   pChooseGuides   = nullptr;
      delete pProfiles;       pProfiles       = nullptr;
      delete pGuides;         pGuides         = nullptr;
      delete pUDegree;        pUDegree        = nullptr;
      delete pVDegree;        pVDegree        = nullptr;
      delete pNumDiscrPoints; pNumDiscrPoints = nullptr;
      delete pProgressFrame;  pProgressFrame  = nullptr;
      delete pProgressBar;    pProgressBar    = nullptr;
    }
  };

  t_widgets                  m_widgets;           //!< UI controls.
  QVBoxLayout*               m_pMainLayout;       //!< Layout of the widget.
  asiUI_ViewerPart*          m_pViewer;           //!< External reference to viewer.
  Handle(asiEngine_Model)    m_model;             //!< Data Model instance.
  bool                       m_blockPointsChange; //!< block for points table change.

  /* Diagnostics */

  ActAPI_ProgressEntry       m_progress;    //!< Progress notifier.
  ActAPI_PlotterEntry        m_plotter;     //!< Imperative plotter.

  mobius::core_ProgressEntry m_mobProgress; //!< Mobius progress controller.

};

//! Dialog for edge selection.
class asiUI_DialogBuildGordonSelectEdges : public QDialog
{
  Q_OBJECT

  public:

  //! Ctor.
  //! \param[in] model     the data model instance.
  //! \param[in] isProfile the flag for correct edge ids transportation.
  //! \param[in] parent    the parent widget.
  asiUI_EXPORT
    asiUI_DialogBuildGordonSelectEdges(asiUI_DialogBuildGordon* mainDialog,
                                       bool                     isProfile,
                                       QWidget*                 parent = nullptr);

  //! Dtor.
  asiUI_EXPORT
    virtual ~asiUI_DialogBuildGordonSelectEdges() {}

public slots:

  //! Reaction on clicking "Apply" button.
  void onApply();

  //! Cancel button clicked.
  void onCancel();

protected:

  //! Widgets.
  struct t_widgets
  {
    QPushButton* pApply; //!< Apply surface fitting method.
    QPushButton* pClose; //!< Closes the dialog.

    //! Default ctor.
    t_widgets() : pApply (nullptr),
                  pClose (nullptr)
    {}

    void Release()
    {
      delete pApply; pApply = nullptr;
      delete pClose; pClose = nullptr;
    }
  };

  t_widgets                m_widgets;     //!< UI controls.
  QVBoxLayout*             m_pMainLayout; //!< Layout of the widget.
  asiUI_DialogBuildGordon* m_mainDialog;  //!< Main dialog for surface constructing.
  bool                     m_isProfile;   //!< Boolean flag for correct edge ids transportation.

};

#endif // asiUI_DialogBuildGordon_h
