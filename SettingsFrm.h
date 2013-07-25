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
