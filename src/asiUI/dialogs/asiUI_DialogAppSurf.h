//-----------------------------------------------------------------------------
// Created on: 26 February 2023
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

#ifndef asiUI_DialogAppSurf_h
#define asiUI_DialogSppSurf_h

// asiUI includes
#include <asiUI_DatumTable.h>
#include <asiUI_LineEdit.h>
#include <asiUI_SelectFile.h>
#include <asiUI_ViewerPart.h>

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

//! Dialog for surface fitting.
class asiUI_DialogAppSurf : public QDialog
{
  Q_OBJECT

public:

  //! Fitting methods.
  enum Method
  {
    Method_PLATE = 0, //!< PLATE interpolation.
    Method_APPSURF1,  //!< APPSURF1 algorithm.
    Method_APPSURF2   //!< APPSURF2 algorithm.
  };

public:

  //! Ctor.
  //! \param[in] wf       the widget factory.
  //! \param[in] model    the data model instance.
  //! \param[in] pViewer  the connected viewer.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  //! \param[in] parent   the parent widget.
  asiUI_EXPORT
    asiUI_DialogAppSurf(const Handle(asiUI_WidgetFactory)& wf,
                        const Handle(asiEngine_Model)&     model,
                        asiUI_ViewerPart*                  pViewer,
                        ActAPI_ProgressEntry               progress,
                        ActAPI_PlotterEntry                plotter,
                        QWidget*                           parent = nullptr);

  //! Dtor.
  asiUI_EXPORT
    virtual ~asiUI_DialogAppSurf();

public slots:

  //! Reaction on edge selection in the viewer.
  void onEdgePicked();

  //! Reaction on face selection in the viewer.
  //! \param[in] pickRes pick result.
  void onFacePicked(asiVisu_PickerResult* pickRes);

  //! Reaction on changing "method" combobox.
  //! \param[in] methodIdx new 0-based type index.
  void onChangeMethod(const int methodIdx);

  //! XYZ file is selected.
  void onXYZSelected();

  //! Change in points table resets XYZ text entered.
  void onPointsChanged();

  //! Reaction on clicking "Apply" button.
  void onApply();

private:

  //! Collects all points from the table in the returned point cloud.
  Handle(asiAlgo_BaseCloud<double>)
    getTablePoints() const;

protected:

  //! Widgets.
  struct t_widgets
  {
    QPushButton*      pApply;               //!< Apply surface fitting method.
    QPushButton*      pClose;               //!< Closes the dialog.
    QComboBox*        pMethodSel;           //!< Selector for the surface fitting method.
    asiUI_LineEdit*   pEdges;               //!< Indices of edges.
    QLabel*           pSelectXYZLabel;      //!< XYZ file selector label.
    asiUI_SelectFile* pSelectXYZ;           //!< XYZ file selector.
    asiUI_DatumTable* pPoints;              //!< Point coordinates.
    QPushButton*      pInsertRow;           //!< Appends a new row after the selected row or to the end.
    QPushButton*      pRemoveRow;           //!< Removes the currently selected row.
    QLabel*           pInitialSurfaceLabel; //!< String name of the optional initial surface description.
    asiUI_LineEdit*   pInitialSurface;      //!< String name of the optional initial surface.
    asiUI_Datum*      pFairingCoeff;        //!< Fairing coefficient ranging.
    asiUI_Datum*      pNumIters;            //!< Number of iterations.

    //! Default ctor.
    t_widgets() : pApply               (nullptr),
                  pClose               (nullptr),
                  pMethodSel           (nullptr),
                  pEdges               (nullptr),
                  pSelectXYZLabel      (nullptr),
                  pSelectXYZ           (nullptr),
                  pPoints              (nullptr),
                  pInsertRow           (nullptr),
                  pRemoveRow           (nullptr),
                  pInitialSurfaceLabel (nullptr),
                  pInitialSurface      (nullptr),
                  pFairingCoeff        (nullptr),
                  pNumIters            (nullptr)
    {}

    void Release()
    {
      delete pApply;               pApply               = nullptr;
      delete pClose;               pClose               = nullptr;
      delete pMethodSel;           pMethodSel           = nullptr;
      delete pEdges;               pEdges               = nullptr;
      delete pSelectXYZLabel;      pSelectXYZLabel      = nullptr;
      delete pSelectXYZ;           pSelectXYZ           = nullptr;
      delete pPoints;              pPoints              = nullptr;
      delete pInsertRow;           pInsertRow           = nullptr;
      delete pRemoveRow;           pRemoveRow           = nullptr;
      delete pInitialSurfaceLabel; pInitialSurfaceLabel = nullptr;
      delete pInitialSurface;      pInitialSurface      = nullptr;
      delete pFairingCoeff;        pFairingCoeff        = nullptr;
      delete pNumIters;            pNumIters            = nullptr;
    }
  };

  t_widgets               m_widgets;           //!< UI controls.
  QVBoxLayout*            m_pMainLayout;       //!< Layout of the widget.
  asiUI_ViewerPart*       m_pViewer;           //!< External reference to viewer.
  Handle(asiEngine_Model) m_model;             //!< Data Model instance.
  bool                    m_blockPointsChange; //!< block for points table change.

  //! Selected surface fitting method.
  Method m_method;

  /* Diagnostics */

  ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif // asiUI_DialogAppSurf_h
