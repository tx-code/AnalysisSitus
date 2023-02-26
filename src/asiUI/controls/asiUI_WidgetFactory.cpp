//-----------------------------------------------------------------------------
// Created on: 22 February 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Anton Poletaev, Sergey Slyadnev
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
#include <asiUI_WidgetFactory.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_CommonFacilities.h>
#include <asiUI_DatumLineEdit.h>
#include <asiUI_DatumPathEdit.h>
#include <asiUI_DatumPathItem.h>
#include <asiUI_DatumComboBox.h>
#include <asiUI_DatumCheckBox.h>
#include <asiUI_DatumColorItem.h>
#include <asiUI_DatumColorEdit.h>
#include <asiUI_DatumSpinBoxDbl.h>
#include <asiUI_DatumDateEdit.h>
#include <asiUI_DatumSpinBox.h>
#include <asiUI_DatumSelector.h>
#include <asiUI_DatumLabel.h>
#include <asiUI_DatumCustomSelector.h>
#include <asiUI_DatumTable.h>
#include <asiUI_DatumTextEdit.h>
#include <asiUI_Logger.h>

// asiAlgo includes
#include <asiAlgo_Dictionary.h>

// Qt includes
#pragma warning(push, 0)
#include <QWidget>
#include <QAbstractItemView>
#pragma warning(pop)

//! Ctor.
asiUI_WidgetFactory::asiUI_WidgetFactory(asiUI_CommonFacilities* cf)
: Standard_Transient(), m_pCommonFacilities(cf)
{
}

//! Create datum editor for dictionary item.
//! If the "widget" type defined in "ui" section of data dictionary
//! doesn't corresponds to data type, the defaul suitable editor 
//! will be created (additionally, a warning message will be printed
//! in log window in debug mode).
//! \param theDataDicID [in] data dictionary identifier string.
//! \param theParent [in] parent widget for datum editor.
//! \param theDatumFlags [in] flags defining options and subwidgets controls
//!        for datum editor (see asiUI_Datum).
//! \return datum pointer, or null pointer if data dictionary id is invalid.
asiUI_Datum* asiUI_WidgetFactory::CreateEditor(const QString& theDataDicID,
                                                 QWidget* theParent,
                                                 int theDatumFlags)
{
  asiUI_Datum* aDatum = editor(theDataDicID, theParent, theDatumFlags);

  if ( aDatum != 0 && m_pCommonFacilities && m_pCommonFacilities->UnitManager )
  {
    QObject::connect(
      m_pCommonFacilities->UnitManager,
      SIGNAL(UnitSystemChanged(const asiAlgo_DictionaryUnitSystem&,
                               const asiAlgo_DictionaryUnitSystem&)),
      aDatum,
      SLOT(OnUnitSystemChanged(const asiAlgo_DictionaryUnitSystem&,
                               const asiAlgo_DictionaryUnitSystem&)));
  }

  return aDatum;
}

//! Create datum table.
//! \param theRowNumber [in] number of rows in the table.
//! \param theColumnNumber [in] number of columns in the table.
//! \param theParent [in] the parent widget.
//! \return new instance of datum table.
asiUI_DatumTable* asiUI_WidgetFactory::CreateDatumTable(const int theRowNumber,
                                                          const int theColumnNumber,
                                                          QWidget*  theParent)
{
  asiUI_DatumTable* aTable = new asiUI_DatumTable(this, theParent);

  aTable->SetRowCount(theRowNumber);
  aTable->SetColumnCount(theColumnNumber);

  return aTable;
}

////! Create instance of datum tree widget.
////! \param theParent [in] the parent widget that takes ownership of the datum tree.
////! \return new instance of datum tree.
//asiUI_DatumTree* asiUI_WidgetFactory::CreateDatumTree(QWidget* theParent)
//{
//  asiUI_DatumTree* aView = new asiUI_DatumTree(this, theParent);
//  if ( m_pApp.isNull() )
//    return aView; // no connect possible
//
//  if ( asiUI_UnitManager* aMgr = m_pApp->GetUnitManager() )
//  {
//    QObject::connect(aMgr, SIGNAL( UnitSystemChanged() ),
//      aView->GetEventHandler(), SLOT( onUnitsChanged() ) );
//  }
//
//  return aView;
//}

//! Create datum editor for dictionary item.
//! \param dataDicID [in] data dictionary identifier string.
//! \param parent [in] parent widget for datum editor.
//! \param datumFlags [in] flags defining options and subwidgets controls
//!        for datum editor (see asiUI_Datum).
//! \return datum pointer, or null pointer if data dictionary id is invalid.
asiUI_Datum*
  asiUI_WidgetFactory::editor(const QString& dataDicID,
                              QWidget*       parent,
                              int            datumFlags)
{
  if ( dataDicID.isEmpty() )
    return NULL;

  // Try to get a dictionary item.
  Handle(asiAlgo_DictionaryItem)
    dicItem = asiAlgo_Dictionary::GetDictionaryItem( QStr2AsciiStr(dataDicID) );
  //
  if ( dicItem.IsNull() )
  {
    if ( m_pCommonFacilities )
      m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Invalid dictionary ID '%1'."
                                                                   << QStr2AsciiStr(dataDicID) );

    return nullptr;
  }

  // get value type and widget type
  asiAlgo_DictionaryItem::DataType   aDataType   = dicItem->GetDataType();
  asiAlgo_DictionaryItem::WidgetType aWidgetType = dicItem->GetWidgetType();

  bool isDelegateEditor = ( parent != 0 && (
      qobject_cast<QAbstractItemView*>( parent ) != 0 ||
      qobject_cast<QAbstractItemView*>( parent->parent() ) != 0 ) );

  //-------------------------------------------------------------------------
  // Create editor widget which types is independent from the data type
  //-------------------------------------------------------------------------

  switch ( aWidgetType )
  {
    case asiAlgo_DictionaryItem::WT_Selector:
      return new asiUI_DatumSelector(dataDicID, parent, datumFlags);

    case asiAlgo_DictionaryItem::WT_CustomSelector:
      return new asiUI_DatumCustomSelector(dataDicID, parent, datumFlags);

    default:
      break; // continue -->
  }

  //-------------------------------------------------------------------------
  // Create editor widget which type corresponds to datum's data type.
  //-------------------------------------------------------------------------

  switch ( aDataType )
  {
    // Integer data type

    case asiAlgo_DictionaryItem::Integer :
    {
      if ( aWidgetType == asiAlgo_DictionaryItem::WT_LineEdit )
      {
        return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_ColorPicker )
      {
        return isDelegateEditor ? (asiUI_Datum*)
          new asiUI_DatumColorItem(dataDicID, parent, datumFlags) :
          new asiUI_DatumColorEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_SpinBox )
      {
        return new asiUI_DatumSpinBox(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_Label )
      {
        return new asiUI_DatumLabel(dataDicID, parent, datumFlags);
      }

      // notify that the inappropriate editor type specified.
      if ( m_pCommonFacilities )
        m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Inappropriate editor '%1' : '%2'."
                                                                     << QStr2AsciiStr(dataDicID)
                                                                     << "LineEdit" );

      return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
    }

    // Float data type

    case asiAlgo_DictionaryItem::Float :
    {
      if ( aWidgetType == asiAlgo_DictionaryItem::WT_LineEdit )
      {
        return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_SpinBox )
      {
        return new asiUI_DatumSpinBoxDbl(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_Label )
      {
        return new asiUI_DatumLabel(dataDicID, parent, datumFlags);
      }

      // notify that the inappropriate editor type specified.
      if ( m_pCommonFacilities )
        m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Inappropriate editor '%1' : '%2'."
                                                                     << QStr2AsciiStr(dataDicID)
                                                                     << "LineEdit" );

      return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
    }

    // String data type

    case asiAlgo_DictionaryItem::String :
    {
      if ( aWidgetType == asiAlgo_DictionaryItem::WT_LineEdit )
      {
        return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_OpenFilePath ||
                aWidgetType == asiAlgo_DictionaryItem::WT_SaveFilePath ||
                aWidgetType == asiAlgo_DictionaryItem::WT_DirPath )
      {
        if (isDelegateEditor)
        {
          asiUI_DatumPathItem* aDatum = new asiUI_DatumPathItem(dataDicID, parent, datumFlags);
          if ( m_pCommonFacilities )
          {
            aDatum->SetDirectory( m_pCommonFacilities->GetDefaultDirectory() );
          }
          return aDatum;
        }
        else
        {
          asiUI_DatumPathEdit* aDatum = new asiUI_DatumPathEdit(dataDicID, parent, datumFlags);
          if ( m_pCommonFacilities )
          {
            aDatum->SetDirectory( m_pCommonFacilities->GetDefaultDirectory() );
          }
          return aDatum;
        }
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_TextEditor )
      {
        return new asiUI_DatumTextEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_DatePicker )
      {
        return new asiUI_DatumDateEdit(dataDicID, parent, datumFlags);
      }
      else if ( aWidgetType == asiAlgo_DictionaryItem::WT_Label )
      {
        return new asiUI_DatumLabel(dataDicID, parent, datumFlags);
      }

      // notify that the inappropriate editor type specified.
      if ( m_pCommonFacilities )
        m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Inappropriate editor '%1' : '%2'."
                                                                     << QStr2AsciiStr(dataDicID)
                                                                     << "LineEdit" );

      return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
    }

    // List data type

    case asiAlgo_DictionaryItem::List :
    {
      // notify that the inappropriate editor type specified.
      if ( aWidgetType != asiAlgo_DictionaryItem::WT_ComboBox )
      {
        if ( m_pCommonFacilities )
          m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Inappropriate editor '%1' : '%2'."
                                                                       << QStr2AsciiStr(dataDicID)
                                                                       << "ComboBox" );
      }

      return new asiUI_DatumComboBox(dataDicID, parent, datumFlags);
    }

    case asiAlgo_DictionaryItem::Bool :
    {
      if ( aWidgetType != asiAlgo_DictionaryItem::WT_CheckBox )
      {
        if ( m_pCommonFacilities )
          m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Inappropriate editor '%1' : '%2'."
                                                                       << QStr2AsciiStr(dataDicID)
                                                                       << "CheckBox" );
      }

      return new asiUI_DatumCheckBox(dataDicID, parent, datumFlags);
    }
    case asiAlgo_DictionaryItem::Unknown : break;
  }

  // notify that the unknown data type specified
  if ( m_pCommonFacilities )
    m_pCommonFacilities->Progress.SendLogMessage( LogErr(Normal) << "Unknown data type for editor '%1' : '%2'."
                                                                 << QStr2AsciiStr(dataDicID)
                                                                 << "LineEdit" );

  return new asiUI_DatumLineEdit(dataDicID, parent, datumFlags);
}
