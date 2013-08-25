//---------------------------------------------------------------------------
// Copyright (C) 2013 Krzysztof Grochocki
//
// This file is part of FixStatusCloud
//
// FixStatusCloud is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// FixStatusCloud is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifndef SettingsFrmH
#define SettingsFrmH
#define WM_ALPHAWINDOWS (WM_USER + 666)
//---------------------------------------------------------------------------
#include "sBevel.hpp"
#include "sButton.hpp"
#include "sCheckBox.hpp"
#include "sEdit.hpp"
#include "sListView.hpp"
#include "sPageControl.hpp"
#include "sRadioButton.hpp"
#include "sSkinManager.hpp"
#include "sSkinProvider.hpp"
#include "sSpinEdit.hpp"
#include <System.Actions.hpp>
#include <System.Classes.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TSettingsForm : public TForm
{
__published:	// IDE-managed Components
	TsBevel *Bevel;
	TsButton *OkButton;
	TsButton *CancelButton;
	TsButton *SaveButton;
	TsSkinManager *sSkinManager;
	TsSkinProvider *sSkinProvider;
	TActionList *ActionList;
	TAction *aExit;
	TAction *aLoadSettings;
	TAction *aSaveSettings;
	TsPageControl *sPageControl;
	TsTabSheet *MainTabSheet;
	TsTabSheet *OtherTabSheet;
	TsRadioButton *Mode1RadioButton;
	TsRadioButton *Mode2RadioButton;
	TsListView *ContactsListView;
	TsEdit *ContactEdit;
	TsButton *AddButton;
	TsButton *DeleteButton;
	TsEdit *ContactJIDEdit;
	TAction *aEnableSaveButton;
	TsCheckBox *PlaySoundCheckBox;
	TsCheckBox *ShowStatusCheckBox;
	TsSpinEdit *CloudTimeOutSpinEdit;
	TsCheckBox *MultiExceptionsCheckBox;
	TsCheckBox *OnStatusChangedCheckBox;
	TsCheckBox *OnOfflineCheckBox;
	TPopupMenu *DeletePopupMenu;
	TMenuItem *aDeleteAll;
	TsCheckBox *OpenMsgCheckBox;
	TAction *aLoadSettingsLite;
	TsCheckBox *GoogleTTSCheckBox;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall aExitExecute(TObject *Sender);
	void __fastcall aLoadSettingsExecute(TObject *Sender);
	void __fastcall aSaveSettingsExecute(TObject *Sender);
	void __fastcall SaveButtonClick(TObject *Sender);
	void __fastcall OkButtonClick(TObject *Sender);
	void __fastcall AddButtonClick(TObject *Sender);
	void __fastcall DeleteButtonClick(TObject *Sender);
	void __fastcall aEnableSaveButtonExecute(TObject *Sender);
	void __fastcall aDeleteAllClick(TObject *Sender);
	void __fastcall aLoadSettingsLiteExecute(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TSettingsForm(TComponent* Owner);
	void __fastcall WMTransparency(TMessage &Message);
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ALPHAWINDOWS,TMessage,WMTransparency);
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
#endif
