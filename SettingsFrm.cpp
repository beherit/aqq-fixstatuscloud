//---------------------------------------------------------------------------
// Copyright (C) 2013-2015 Krzysztof Grochocki
//
// This file is part of FixStatusCloud
//
// FixStatusCloud is free software: you can redistribute it and/or modify
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
// along with GNU Radio. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "SettingsFrm.h"
#include <inifiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "sBevel"
#pragma link "sButton"
#pragma link "sCheckBox"
#pragma link "sEdit"
#pragma link "sListView"
#pragma link "sPageControl"
#pragma link "sRadioButton"
#pragma link "sSkinManager"
#pragma link "sSkinProvider"
#pragma link "sSpinEdit"
#pragma resource "*.dfm"
TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
__declspec(dllimport)UnicodeString GetPluginUserDir();
__declspec(dllimport)UnicodeString GetThemeSkinDir();
__declspec(dllimport)UnicodeString GetContactNick(UnicodeString JID);
__declspec(dllimport)UnicodeString FrendlyFormatJID(UnicodeString JID);
__declspec(dllimport)bool ChkSkinEnabled();
__declspec(dllimport)bool ChkThemeAnimateWindows();
__declspec(dllimport)bool ChkThemeGlowing();
__declspec(dllimport)int GetHUE();
__declspec(dllimport)int GetSaturation();
__declspec(dllimport)int GetBrightness();
__declspec(dllimport)void LoadSettings();
//---------------------------------------------------------------------------
__fastcall TSettingsForm::TSettingsForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::WMTransparency(TMessage &Message)
{
  Application->ProcessMessages();
  if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormCreate(TObject *Sender)
{
  //Wlaczona zaawansowana stylizacja okien
  if(ChkSkinEnabled())
  {
	UnicodeString ThemeSkinDir = GetThemeSkinDir();
	//Plik zaawansowanej stylizacji okien istnieje
	if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	{
	  //Dane pliku zaawansowanej stylizacji okien
	  ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
	  sSkinManager->SkinDirectory = ThemeSkinDir;
	  sSkinManager->SkinName = "Skin.asz";
	  //Ustawianie animacji AlphaControls
	  if(ChkThemeAnimateWindows()) sSkinManager->AnimEffects->FormShow->Time = 200;
	  else sSkinManager->AnimEffects->FormShow->Time = 0;
	  sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
	  //Zmiana kolorystyki AlphaControls
	  sSkinManager->HueOffset = GetHUE();
	  sSkinManager->Saturation = GetSaturation();
	  sSkinManager->Brightness = GetBrightness();
	  //Aktywacja skorkowania AlphaControls
	  sSkinManager->Active = true;
	}
	//Brak pliku zaawansowanej stylizacji okien
	else sSkinManager->Active = false;
  }
  //Zaawansowana stylizacja okien wylaczona
  else sSkinManager->Active = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormShow(TObject *Sender)
{
  //Odczyt ustawien wtyczki
  aLoadSettings->Execute();
  //Wylaczenie przyciskow
  SaveButton->Enabled = false;
  //Poprawienie listy
  if(ContactsListView->Items->Count<8) ContactsListView->Column[0]->Width = 246;
  else ContactsListView->Column[0]->Width = 228;
  //Wywalenie tekstu z pola
  ContactEdit->Text = "";
  //Pokazanie domyslnej karty
  sPageControl->ActivePage = MainTabSheet;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aExitExecute(TObject *Sender)
{
  //Zamkniecie formy
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aLoadSettingsExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\FixStatusCloud\\\\Settings.ini");
  int pMode = Ini->ReadInteger("Settings","Mode",1);
  if(pMode==1) Mode1RadioButton->Checked = true;
  else if(pMode==2) Mode2RadioButton->Checked = true;
  TStringList *ExceptionsList = new TStringList;
  ContactsListView->Clear();
  Ini->ReadSection("Exceptions",ExceptionsList);
  int ExceptionsCount = ExceptionsList->Count;
  delete ExceptionsList;
  if(ExceptionsCount>0)
  {
	for(int Count=0;Count<ExceptionsCount;Count++)
	{
	  UnicodeString JID = Ini->ReadString("Exceptions","Item"+IntToStr(Count+1),"");
	  if(!JID.IsEmpty())
	  {
		ContactsListView->Items->Add();
		ContactsListView->Items->Item[ContactsListView->Items->Count-1]->Caption = GetContactNick(JID) + " (" + FrendlyFormatJID(JID) + ")";
		ContactsListView->Items->Item[ContactsListView->Items->Count-1]->SubItems->Add(JID);
	  }
	}
  }
  ContactsListView->AlphaSort();
  SiblingsExceptionsCheckBox->Checked = Ini->ReadBool("Settings","SiblingsExceptions",true);
  PlaySoundCheckBox->Checked = Ini->ReadBool("Settings","PlaySound",true);
  ShowStatusCheckBox->Checked = Ini->ReadBool("Settings","ShowStatus",false);
  OnStatusChangedCheckBox->Checked = Ini->ReadBool("Settings","OnStatusChanged",true);
  OnOfflineCheckBox->Checked = Ini->ReadBool("Settings","OnOffline",false);
  OpenMsgCheckBox->Checked = Ini->ReadBool("Settings","OpenMsg",true);
  GoogleTTSCheckBox->Checked = Ini->ReadBool("Settings","GoogleTTS",false);
  CloudTimeOutSpinEdit->Value = Ini->ReadInteger("Settings","CloudTimeOut",5);
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aLoadSettingsLiteExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\FixStatusCloud\\\\Settings.ini");
  TStringList *ExceptionsList = new TStringList;
  ContactsListView->Clear();
  Ini->ReadSection("Exceptions",ExceptionsList);
  int ExceptionsCount = ExceptionsList->Count;
  delete ExceptionsList;
  if(ExceptionsCount>0)
  {
	for(int Count=0;Count<ExceptionsCount;Count++)
	{
	  UnicodeString JID = Ini->ReadString("Exceptions","Item"+IntToStr(Count+1),"");
	  if(!JID.IsEmpty())
	  {
		ContactsListView->Items->Add();
		ContactsListView->Items->Item[ContactsListView->Items->Count-1]->Caption = GetContactNick(JID) + " (" + FrendlyFormatJID(JID) + ")";
		ContactsListView->Items->Item[ContactsListView->Items->Count-1]->SubItems->Add(JID);
	  }
	}
  }
  ContactsListView->AlphaSort();
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aSaveSettingsExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\FixStatusCloud\\\\Settings.ini");
  if(Mode1RadioButton->Checked) Ini->WriteInteger("Settings","Mode",1);
  else Ini->WriteInteger("Settings","Mode",2);
  Ini->EraseSection("Exceptions");
  if(ContactsListView->Items->Count)
  {
	for(int Count=0;Count<ContactsListView->Items->Count;Count++)
	 Ini->WriteString("Exceptions","Item"+IntToStr(Count+1),ContactsListView->Items->Item[Count]->SubItems->Strings[0]);
  }
  Ini->WriteBool("Settings","SiblingsExceptions",SiblingsExceptionsCheckBox->Checked);
  Ini->WriteBool("Settings","PlaySound",PlaySoundCheckBox->Checked);
  Ini->WriteBool("Settings","ShowStatus",ShowStatusCheckBox->Checked);
  Ini->WriteBool("Settings","OnStatusChanged",OnStatusChangedCheckBox->Checked);
  Ini->WriteBool("Settings","OnOffline",OnOfflineCheckBox->Checked);
  Ini->WriteBool("Settings","OpenMsg",OpenMsgCheckBox->Checked);
  Ini->WriteBool("Settings","GoogleTTS",GoogleTTSCheckBox->Checked);
  Ini->WriteInteger("Settings","CloudTimeOut",CloudTimeOutSpinEdit->Value);
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::SaveButtonClick(TObject *Sender)
{
  //Wylaczenie buttona
  SaveButton->Enabled = false;
  //Zapis ustawien
  aSaveSettings->Execute();
  //Zmiana ustawien w rdzeniu wtyczki
  LoadSettings();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::OkButtonClick(TObject *Sender)
{
  //Zapis ustawien
  aSaveSettings->Execute();
  //Zmiana ustawien w rdzeniu wtyczki
  LoadSettings();
  //Zamkniecie formy
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AddButtonClick(TObject *Sender)
{
  if(!ContactEdit->Text.IsEmpty())
  {
	//Sprawdzanie czy wskazany kontakt jest juz dodany
	bool DoNotAdd = false;
	for(int Count=0;Count<ContactsListView->Items->Count;Count++)
	{
	  if(ContactsListView->Items->Item[Count]->Caption==ContactEdit->Text)
	  {
		if(ContactsListView->Items->Item[Count]->SubItems->Strings[0]==ContactJIDEdit->Text)
		{
		  DoNotAdd = true;
		  Count = ContactsListView->Items->Count;
		}
	  }
	}
	//Wskazany kontakt nie zostal jeszcze dodany
	if(!DoNotAdd)
	{
      //Dodanie nowego elementu
	  ContactsListView->Items->Add();
	  ContactsListView->Items->Item[ContactsListView->Items->Count-1]->Caption = ContactEdit->Text;
	  ContactsListView->Items->Item[ContactsListView->Items->Count-1]->SubItems->Add(ContactJIDEdit->Text);
	  //Sortowanie elementow listy
	  ContactsListView->AlphaSort();
	  //Wlaczenie przycisku do zapisu
	  SaveButton->Enabled = true;
	  //Poprawienie listy
	  if(ContactsListView->Items->Count<8) ContactsListView->Column[0]->Width = 246;
	  else ContactsListView->Column[0]->Width = 228;
	}
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::DeleteButtonClick(TObject *Sender)
{
  if(ContactsListView->ItemIndex!=-1)
  {
	//Usuwanie wybranego elementu
	ContactsListView->Items->Item[ContactsListView->ItemIndex]->Delete();
	//Sortowanie elementow listy
	ContactsListView->AlphaSort();
	//Wlaczenie przycisku do zapisu
	SaveButton->Enabled = true;
	//Poprawienie listy
	if(ContactsListView->Items->Count<8) ContactsListView->Column[0]->Width = 246;
	else ContactsListView->Column[0]->Width = 228;
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aEnableSaveButtonExecute(TObject *Sender)
{
  SaveButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aDeleteAllClick(TObject *Sender)
{
  //Usuniecie wszystkich elementow z listy
  ContactsListView->Clear();
  //Wlaczenie przycisku do zapisu
  SaveButton->Enabled = true;
  //Poprawienie listy
  if(ContactsListView->Items->Count<8) ContactsListView->Column[0]->Width = 246;
  else ContactsListView->Column[0]->Width = 228;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning)
{
  AllowSkinning = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ContactsListViewChange(TObject *Sender, TListItem *Item,
          TItemChange Change)
{
  if(ContactsListView->Items->Count>8)
   ContactsListView->Column[0]->Width = 233;
  else
   ContactsListView->Column[0]->Width = 250;
}
//---------------------------------------------------------------------------

