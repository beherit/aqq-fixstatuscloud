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

#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma argsused
#include <PluginAPI.h>
#include "SettingsFrm.h"
#include <IdHashMessageDigest.hpp>
#define SAYAQQ_SYSTEM_SAYTEXT L"SayAQQ/System/SayText"

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  return 1;
}
//---------------------------------------------------------------------------

//Uchwyt-do-formy-ustawien---------------------------------------------------
TSettingsForm *hSettingsForm;
//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//Szybkie-wlaczenie/wylaczenie-notyfikacji-----------------------------------
TPluginAction FixStatusCloudFastItem;
//Szybki-dostep-do-ustawien-wtyczki------------------------------------------
TPluginAction FixStatusCloudFastSettingsItem;
//Lista-nickow-kontatkow-----------------------------------------------------
TStringList *ContactList = new TStringList;
TCustomIniFile* ContactsNickList = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
//Lista-JID-kontaktow-plci-zenskiej------------------------------------------
TStringList *FemalesList = new TStringList;
//Identyfikator-kontaktu-----------------------------------------------------
UnicodeString ContactJID;
//ID-wywolania-enumeracji-listy-kontaktow------------------------------------
DWORD ReplyListID = 0;
//Tymczasowe-zablokowanie-dzialania-wtyczki----------------------------------
bool BlockFunction = false;
bool NetworkConnecting = false;
//Gdy-zostalo-uruchomione-wyladowanie-wtyczki-wraz-z-zamknieciem-komunikatora
bool ForceUnloadExecuted = false;
//Przywracanie-ustawien-dzwieku-w-rdzeniu-AQQ--------------------------------
bool RestoreStatusSound = false;
//Uchwyt-do-okna-timera------------------------------------------------------
HWND hTimerFrm;
//SETTINGS-------------------------------------------------------------------
int ModeChk;
TStringList *ExceptionsList = new TStringList;
TStringList *MultiExceptionsList = new TStringList;
bool MultiExceptionsChk;
bool PlaySoundChk;
bool ShowStatusChk;
bool OnStatusChangedChk;
bool OnOfflineChk;
bool OpenMsgChk;
bool GoogleTTSChk;
int CloudTimeOutVal;
//IKONY-W-INTERFEJSIE--------------------------------------------------------
int FASTACCESS;
//TIMERY---------------------------------------------------------------------
#define TIMER_UNBLOCKFUNCTION 10
#define TIMER_CHKSETTINGS 20
//FORWARD-AQQ-HOOKS----------------------------------------------------------
int __stdcall OnBeforeUnload(WPARAM wParam, LPARAM lParam);
int __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam);
int __stdcall OnContactSelected(WPARAM wParam, LPARAM lParam);
int __stdcall OnListReady(WPARAM wParam, LPARAM lParam);
int __stdcall OnReplyList(WPARAM wParam, LPARAM lParam);
int __stdcall OnSetLastState(WPARAM wParam, LPARAM lParam);
int __stdcall OnShowInfo(WPARAM wParam, LPARAM lParam);
int __stdcall OnStateChange(WPARAM wParam, LPARAM lParam);
int __stdcall OnSystemPopUp(WPARAM wParam, LPARAM lParam);
int __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam);
int __stdcall OnWindowEvent(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceFixStatusCloudFastItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceFixStatusCloudFastSettingsItem(WPARAM wParam, LPARAM lParam);
//FORWARD-TIMER--------------------------------------------------------------
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu zawierajacego informacje o kontaktach
UnicodeString GetContactsUserDir()
{
  return StringReplace((wchar_t *)PluginLink.CallService(AQQ_FUNCTION_GETUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Data\\\\Contacts\\\\";
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do skorki kompozycji
UnicodeString GetThemeSkinDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
}
//---------------------------------------------------------------------------

//Sprawdzanie czy  wlaczona jest zaawansowana stylizacja okien
bool ChkSkinEnabled()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString SkinsEnabled = Settings->ReadString("Settings","UseSkin","1");
  delete Settings;
  return StrToBool(SkinsEnabled);
}
//---------------------------------------------------------------------------

//Sprawdzanie ustawien animacji AlphaControls
bool ChkThemeAnimateWindows()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString AnimateWindowsEnabled = Settings->ReadString("Theme","ThemeAnimateWindows","1");
  delete Settings;
  return StrToBool(AnimateWindowsEnabled);
}
//---------------------------------------------------------------------------
bool ChkThemeGlowing()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString GlowingEnabled = Settings->ReadString("Theme","ThemeGlowing","1");
  delete Settings;
  return StrToBool(GlowingEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien koloru AlphaControls
int GetHUE()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETHUE,0,0);
}
//---------------------------------------------------------------------------
int GetSaturation()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETSATURATION,0,0);
}
//---------------------------------------------------------------------------

//Pobieranie pseudonimu kontaktu podajac jego JID
UnicodeString GetContactNick(UnicodeString JID)
{
  UnicodeString Nick = ContactsNickList->ReadString("Nick",JID,"");
  if(Nick.IsEmpty())
  {
	if(JID.Pos("@")) JID.Delete(JID.Pos("@"),JID.Length());
	return JID;
  }
  return Nick;
}
//---------------------------------------------------------------------------

//Przyjazniejsze formatowanie JID
UnicodeString FrendlyFormatJID(UnicodeString JID)
{
  //nk.pl
  if(JID.Pos("@nktalk.pl")) return "nk.pl";
  //Facebook
  if(JID.Pos("@chat.facebook.com")) return "Facebook";
  //GTalk
  if(JID.Pos("@public.talk.google.com")) return "GTalk";
  //Skype
  if(JID.Pos("@skype.plugin.aqq.eu")) return "Skype";
  //GG
  if(JID.Pos("@plugin.gg.aqq.eu")) return "GG";
  //Inne wtyczki
  if(JID.Pos("@plugin"))
  {
	JID.Delete(JID.Pos("@"),JID.Length());
	return JID;
  }
  //Reszta
  return JID;
}
//---------------------------------------------------------------------------

//Sprawdzanie czy dzwieki w AQQ sa wlaczone
bool ChkSoundEnabled()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString SoundOff = Settings->ReadString("Sound","SoundOff","0");
  delete Settings;
  return !StrToBool(SoundOff);  
}
//---------------------------------------------------------------------------

//Serwis szybkiego wlaczenia/wylaczenia notyfikacji
int __stdcall ServiceFixStatusCloudFastItem(WPARAM wParam, LPARAM lParam)
{
  //Dodawanie/usuwanie kontatku do/z listy wyjatkow
  if(ExceptionsList->IndexOf(ContactJID)!=-1) ExceptionsList->Delete(ExceptionsList->IndexOf(ContactJID));
  else ExceptionsList->Add(ContactJID);
  //Zapisywanie ustawien
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\FixStatusCloud\\\\Settings.ini");
  Ini->EraseSection("Exceptions");
  if(ExceptionsList->Count)
  {
	for(int Count=0;Count<ExceptionsList->Count;Count++)
	 Ini->WriteString("Exceptions","Item"+IntToStr(Count+1),ExceptionsList->Strings[Count]);
  }
  delete Ini;
  //Zmiay na formie ustawien
  if((hSettingsForm)&&(hSettingsForm->Visible))
   hSettingsForm->aLoadSettingsLite->Execute();

  return 0;
}
//---------------------------------------------------------------------------

//Zmiana stanu elementu szybkiego wlaczenia/wylaczenia notyfikacji
void ChangeFixStatusCloudFastItem(bool Checked)
{
  TPluginActionEdit PluginActionEdit;
  PluginActionEdit.cbSize = sizeof(TPluginActionEdit);
  PluginActionEdit.pszName = L"FixStatusCloudFastItemButton";
  if(ModeChk==1) PluginActionEdit.Caption = L"Nie pokazuj zmiany statusu";
  else if(ModeChk==2) PluginActionEdit.Caption = L"Pokazuj zmianê statusu";
  PluginActionEdit.Hint = L"";
  PluginActionEdit.Enabled = true;
  PluginActionEdit.Visible = true;
  PluginActionEdit.IconIndex = -1;
  PluginActionEdit.Checked = Checked;
  PluginLink.CallService(AQQ_CONTROLS_EDITPOPUPMENUITEM,0,(LPARAM)(&PluginActionEdit));
}
//---------------------------------------------------------------------------

//Usuwanie elementu szybkiego wlaczenia/wylaczenia notyfikacji
void DestroyFixStatusCloudFastItem()
{
  FixStatusCloudFastItem.cbSize = sizeof(TPluginAction);
  FixStatusCloudFastItem.pszName = L"FixStatusCloudFastItemButton";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&FixStatusCloudFastItem));
}
//---------------------------------------------------------------------------

//Tworzenie elementu szybkiego wlaczenia/wylaczenia notyfikacji
void BuildFixStatusCloudFastItem()
{
  //Ustalanie pozycji elementu "Wizytowka"
  TPluginItemDescriber PluginItemDescriber;
  PluginItemDescriber.cbSize = sizeof(TPluginItemDescriber);
  PluginItemDescriber.FormHandle = 0;
  PluginItemDescriber.ParentName = L"muItem";
  PluginItemDescriber.Name = L"muProfile";
  PPluginAction Action = (PPluginAction)PluginLink.CallService(AQQ_CONTROLS_GETPOPUPMENUITEM,0,(LPARAM)(&PluginItemDescriber));
  int Position = Action->Position;
  //Tworzenie elementu wtyczki
  FixStatusCloudFastItem.cbSize = sizeof(TPluginAction);
  FixStatusCloudFastItem.pszName = L"FixStatusCloudFastItemButton";
  FixStatusCloudFastItem.pszCaption = L"FixStatusCloud";
  FixStatusCloudFastItem.Position = Position + 1;
  FixStatusCloudFastItem.IconIndex = -1;
  FixStatusCloudFastItem.pszService = L"sFixStatusCloudFastItem";
  FixStatusCloudFastItem.pszPopupName = L"muItem";
  FixStatusCloudFastItem.PopupPosition = 0;
  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&FixStatusCloudFastItem));
}
//---------------------------------------------------------------------------

//Serwis szybkiego dostepu do ustawien wtyczki
int __stdcall ServiceFixStatusCloudFastSettingsItem(WPARAM wParam, LPARAM lParam)
{
  //Przypisanie uchwytu do formy ustawien
  if(!hSettingsForm)
  {
	Application->Handle = (HWND)SettingsForm;
	hSettingsForm = new TSettingsForm(Application);
  }
  //Pokaznie okna ustawien
  hSettingsForm->Show();

  return 0;
}
//---------------------------------------------------------------------------

//Usuwanie elementu szybkiego dostepu do ustawien wtyczki
void DestroyFixStatusCloudFastSettingsItem()
{
  FixStatusCloudFastSettingsItem.cbSize = sizeof(TPluginAction);
  FixStatusCloudFastSettingsItem.pszName = L"FixStatusCloudFastSettingsItemButton";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&FixStatusCloudFastSettingsItem));
}
//---------------------------------------------------------------------------

//Tworzenie elementu szybkiego dostepu do ustawien wtyczki
void BuildFixStatusCloudFastSettingsItem()
{
  FixStatusCloudFastSettingsItem.cbSize = sizeof(TPluginAction);
  FixStatusCloudFastSettingsItem.pszName = L"FixStatusCloudFastSettingsItemButton";
  FixStatusCloudFastSettingsItem.pszCaption = L"FixStatusCloud";
  FixStatusCloudFastSettingsItem.Position = 0;
  FixStatusCloudFastSettingsItem.IconIndex = FASTACCESS;
  FixStatusCloudFastSettingsItem.pszService = L"sFixStatusCloudFastSettingsItem";
  FixStatusCloudFastSettingsItem.pszPopupName = L"popPlugins";
  FixStatusCloudFastSettingsItem.PopupPosition = 0;
  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&FixStatusCloudFastSettingsItem));
}
//---------------------------------------------------------------------------

//Procka okna timera
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if((uMsg==WM_TIMER)&&(!ForceUnloadExecuted))
  {
	//Odblokowanie dzialania wtyczki
	if(wParam==TIMER_UNBLOCKFUNCTION)
	{
	  //Zatrzymanie timera
	  KillTimer(hTimerFrm,TIMER_UNBLOCKFUNCTION);
	  //Odblokowanie dzialania wtyczki
	  BlockFunction = false;
	}
	//Sprawdzanie czy zostaly zmienione pewne rzeczy w ustawieniach komunikatora
	else if(wParam==TIMER_CHKSETTINGS)
	{
	  //Zatrzymanie timera
	  KillTimer(hTimerFrm,TIMER_CHKSETTINGS);
	  //Sprawdzanie czy dzwiek statusu jest wlaczony
	  TStrings* IniList = new TStringList();
	  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
	  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	  Settings->SetStrings(IniList);
	  UnicodeString StatusSoundEnabled = Settings->ReadString("Sound","SoundStatusActive","1");
	  //Wylaczenie dzwieku statusu za usera
	  if(StrToBool(StatusSoundEnabled))
	  {
		//Nowe ustawienia
		TSaveSetup SaveSetup;
		SaveSetup.Section = L"Sound";
		SaveSetup.Ident = L"SoundStatusActive";
		SaveSetup.Value = L"0";
		//Zapis ustawien
		PluginLink.CallService(AQQ_FUNCTION_SAVESETUP,1,(LPARAM)(&SaveSetup));
		//Odswiezenie ustawien
		PluginLink.CallService(AQQ_FUNCTION_REFRESHSETUP,0,0);
		//Zapisanie pozniejszego przywrocenia
		RestoreStatusSound = true;
	  }
	  //Tworzenie timera
	  SetTimer(hTimerFrm,TIMER_CHKSETTINGS,500,(TIMERPROC)TimerFrmProc);
	}

	return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

//Hook na wylaczenie komunikatora poprzez usera
int __stdcall OnBeforeUnload(WPARAM wParam, LPARAM lParam)
{
  //Info o rozpoczeciu procedury zamykania komunikatora
  ForceUnloadExecuted = true;

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane stanu kontaktu
int __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam)
{
  if(!ForceUnloadExecuted)
  {
	//Pobranie danych dotyczacych kontatku
	TPluginContact ContactsUpdateContact = *(PPluginContact)wParam;
	UnicodeString Nick = (wchar_t*)ContactsUpdateContact.Nick;
	UnicodeString JID = (wchar_t*)ContactsUpdateContact.JID;
	//Zapisywanie nicku kontatku
	ContactsNickList->WriteString("Nick",JID,Nick);
	//Otwieranie pliku INI kontatku
	TIniFile *Ini = new TIniFile(GetContactsUserDir()+JID+".ini");
	//Pobieranie informacji o plci kontaktu
	UnicodeString Gender = Ini->ReadString("Buddy","Gender","");
	//Jezeli kontakt jest kobieta :)
	if(Gender=="RkVNQUxF")
	{
	  //Dodawanie kontatku do listy kobiet
	  if(FemalesList->IndexOf(JID)==-1) FemalesList->Add(JID);
	}
	delete Ini;
	//Pokazywanie chmurki informacyjnej
	if((!BlockFunction)&&((lParam==CONTACT_UPDATE_NOOFFLINE)||(lParam==CONTACT_UPDATE_ONLINE)||((OnStatusChangedChk)&&(lParam==CONTACT_UPDATE_ONLYSTATUS))||((OnOfflineChk)&&(lParam==CONTACT_UPDATE_OFFLINE))))
	{
	  //Wyklucz z notyfikacji jedynie zdefiniowane kontakty LUB poka¿ notyfikacjê tylko dla wybranych kontaktów
	  if(((ModeChk==1)&&((ExceptionsList->IndexOf(JID)==-1)||((MultiExceptionsChk)&&(MultiExceptionsList->IndexOf(Nick)==-1))))
      ||((ModeChk==2)&&((ExceptionsList->IndexOf(JID)!=-1)||((MultiExceptionsChk)&&(MultiExceptionsList->IndexOf(Nick)!=-1)))))
	  {
		//Pobranie szczegolowych danych dotyczacych kontatku
		UnicodeString Resource = (wchar_t*)ContactsUpdateContact.Resource;
		int UserIdx = ContactsUpdateContact.UserIdx;
		//Dodawanie kontaktu do listy
		if(ContactList->IndexOf(Nick)==-1) ContactList->Add(Nick);
		//Pokazanie chmurki informacyjnej
		TPluginShowInfo PluginShowInfo;
		PluginShowInfo.cbSize = sizeof(TPluginShowInfo);
		PluginShowInfo.Event = tmePseudoStatus;
		PluginShowInfo.Text = Nick.w_str();
		PluginShowInfo.ImagePath = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPNG_FILEPATH,PluginLink.CallService(AQQ_FUNCTION_GETSTATEPNG_INDEX,0,(LPARAM)&ContactsUpdateContact),0);
		PluginShowInfo.TimeOut = CloudTimeOutVal * 1000;
		if(OpenMsgChk)
		{
		  if(Resource.IsEmpty()) PluginShowInfo.ActionID = ("EXEC_MSG:" + IntToStr(UserIdx) + ";" + JID).w_str();
		  else PluginShowInfo.ActionID = ("EXEC_MSG:" + IntToStr(UserIdx) + ";" + JID + "/" + Resource).w_str();
		}
		else PluginShowInfo.ActionID = L"";
		PluginShowInfo.Tick = GetTickCount();
		PluginLink.CallService(AQQ_FUNCTION_SHOWINFO,0,(LPARAM)(&PluginShowInfo));
		//Pokazanie dodatkowej chmurki z opisem
		if(ShowStatusChk)
		{
		  UnicodeString Status = (wchar_t*)ContactsUpdateContact.Status;
		  Status = Status.Trim();
		  if(!Status.IsEmpty())
		  {
			PluginShowInfo.cbSize = sizeof(TPluginShowInfo);
			PluginShowInfo.Event = tmePseudoStatus;
			PluginShowInfo.Text = Status.w_str();
			PluginShowInfo.ImagePath = L"";
			PluginShowInfo.TimeOut = CloudTimeOutVal * 1000;
			if(OpenMsgChk)
			{
			  if(Resource.IsEmpty()) PluginShowInfo.ActionID = ("EXEC_MSG:" + IntToStr(UserIdx) + ";" + JID).w_str();
			  else PluginShowInfo.ActionID = ("EXEC_MSG:" + IntToStr(UserIdx) + ";" + JID + "/" + Resource).w_str();
			}
			else PluginShowInfo.ActionID = L"";
			PluginLink.CallService(AQQ_FUNCTION_SHOWINFO,0,(LPARAM)(&PluginShowInfo));
		  }
		}
		//Odtworzenie dzwieku
		if((PlaySoundChk)&&(ChkSoundEnabled())) PluginLink.CallService(AQQ_SYSTEM_PLAYSOUND,SOUND_STATUS,1);
		//Google TTS
		if((GoogleTTSChk)&&(ChkSoundEnabled())&&(lParam!=CONTACT_UPDATE_ONLYSTATUS))
		{
		  //Pobieranie stanu kontatku
		  int State = ContactsUpdateContact.State;
		  //Zmienna tekstu do przeczytania
		  UnicodeString TextToSpeech;
		  //Male
		  if(FemalesList->IndexOf(JID)==-1)
		  {
			//Rozlaczony
			if(State==0) TextToSpeech = "\"" + Nick + " jest roz³¹czony\".";
			//Polaczony
			else if(State==1) TextToSpeech = "\"" + Nick + " jest dostêpny\".";
			//Wolny
			else if(State==2) TextToSpeech = "\"" + Nick + " jest chêtny do rozmowy\".";
			//Oddalony
			else if(State==3) TextToSpeech = "\"" + Nick + " jest oddalony\".";
			//Nieobecny
			else if(State==4) TextToSpeech = "\"" + Nick + " jest nieobecny\".";
			//Nie przeszkadzac
			else if(State==5) TextToSpeech = "\"" + Nick + " jest na nie przeszkadzaæ\".";
			//Niewidoczny
			else if(State==6) TextToSpeech = "\"" + Nick + " siê ukrywa\"!";
		  }
		  //Female
		  else
		  {
			//Rozlaczony
			if(State==0) TextToSpeech = "\"" + Nick + " jest roz³¹czona\".";
			//Polaczony
			else if(State==1) TextToSpeech = "\"" + Nick + " jest dostêpna\".";
			//Wolny
			else if(State==2) TextToSpeech = "\"" + Nick + " jest chêtna do rozmowy\".";
			//Oddalony
			else if(State==3) TextToSpeech = "\"" + Nick + " jest oddalona\".";
			//Nieobecny
			else if(State==4) TextToSpeech = "\"" + Nick + " jest nieobecna\".";
			//Nie przeszkadzac
			else if(State==5) TextToSpeech = "\"" + Nick + " jest na nie przeszkadzaæ\".";
			//Niewidoczny
			else if(State==6) TextToSpeech = "\"" + Nick + " siê ukrywa\"!";
		  }
		  //Proba przeczytania tekstu przez wtyczke SayAQQ
		  TPluginHook PluginHook;
		  PluginHook.HookName = SAYAQQ_SYSTEM_SAYTEXT;
		  PluginHook.wParam = 0;
		  PluginHook.lParam = (LPARAM)TextToSpeech.w_str();
		  if(PluginLink.CallService(AQQ_SYSTEM_SENDHOOK,(WPARAM)(&PluginHook),0)!=1)
		  {
			//Czytanie tekstu przez wbuduwany system
			PluginLink.CallService(AQQ_FUNCTION_SAY,(WPARAM)TextToSpeech.w_str(),0);
		  }
		}
	  }
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zaznaczenie kontaktu na liscie
int __stdcall OnContactSelected(WPARAM wParam, LPARAM lParam)
{
  if((!ForceUnloadExecuted)&&(hSettingsForm)&&(hSettingsForm->Visible))
  {
	//Pobranie danych dotyczacych kontatku
	TPluginContact ContactSelectedContact = *(PPluginContact)lParam;
	UnicodeString Nick = (wchar_t*)ContactSelectedContact.Nick;
	UnicodeString JID = (wchar_t*)ContactSelectedContact.JID;
	//Dodawanie kontatku na formie
	hSettingsForm->ContactEdit->Text = Nick + " (" + FrendlyFormatJID(JID) +")";
	hSettingsForm->ContactJIDEdit->Text = JID;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zaladowanie listy kontaktow
int __stdcall OnListReady(WPARAM wParam, LPARAM lParam)
{
  //Pobranie ID dla enumeracji kontaktów
  ReplyListID = GetTickCount();
  //Wywolanie enumeracji kontaktow
  PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);

  return 0;
}
//---------------------------------------------------------------------------

//Hook na enumeracje listy kontatkow
int __stdcall OnReplyList(WPARAM wParam, LPARAM lParam)
{
  //Sprawdzanie ID wywolania enumerqacji
  if((wParam==ReplyListID)&&(!ForceUnloadExecuted))
  {
	//Pobranie danych dotyczacych kontatku
	TPluginContact ReplyListContact = *(PPluginContact)lParam;
	UnicodeString JID = (wchar_t*)ReplyListContact.JID;
	UnicodeString Nick = (wchar_t*)ReplyListContact.Nick;
	//Zapisywanie nicku kontatku
	ContactList->Add(Nick);
	ContactsNickList->WriteString("Nick",JID,Nick);
	//Otwieranie pliku INI kontatku
	TIniFile *Ini = new TIniFile(GetContactsUserDir()+JID+".ini");
	//Pobieranie informacji o plci kontaktu
	UnicodeString Gender = Ini->ReadString("Buddy","Gender","");
	//Jezeli kontakt jest kobieta :)
	if(Gender=="RkVNQUxF")
	 //Dodawanie kontatku do listy kobiet
	 FemalesList->Add(JID);
	delete Ini;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na polaczenie sieci przy starcie AQQA
int __stdcall OnSetLastState(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie ilosci kont
  int UserIdxCount = PluginLink.CallService(AQQ_FUNCTION_GETUSEREXCOUNT,0,0);
  //Sprawdzanie stanu sieci
  for(int UserIdx=0;UserIdx<UserIdxCount;UserIdx++)
  {
    //Pobieranie stanu sieci
	TPluginStateChange PluginStateChange;
	PluginLink.CallService(AQQ_FUNCTION_GETNETWORKSTATE,(WPARAM)(&PluginStateChange),UserIdx);
	int NewState = PluginStateChange.NewState;
	//Connected
	if(NewState)
	{
	  //Blokowanie dzialania wtyczki
	  BlockFunction = true;
	  //Tworzenie timera do odblokowania dzialania wtyczki
	  SetTimer(hTimerFrm,TIMER_UNBLOCKFUNCTION,10000,(TIMERPROC)TimerFrmProc);
	  //Zakonczenie petli
	  UserIdx = UserIdxCount;
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na wyswietlanie chmurki informacyjnej
int __stdcall OnShowInfo(WPARAM wParam, LPARAM lParam)
{
  if(!ForceUnloadExecuted)
  {
	//Pobieranie danych dotyczacych chmurki informacyjnej
	TPluginShowInfo ShowInfo = *(PPluginShowInfo)lParam;
	//Pobranie tekstu chmurki
	UnicodeString Text = (wchar_t*)ShowInfo.Text;
	//Tekst chmurki zawiera nick kontatku
	if(ContactList->IndexOf(Text)!=-1)
	{
	  //Pobranie akcji chmurki
	  UnicodeString ActionID = (wchar_t*)ShowInfo.ActionID;
	  //Akcja chmurki jest "pusta"
	  if(ActionID.Pos("session")==1)
	  {
		//Blokowanie chmurki
		return 1;
	  }
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Notyfikacja zmiany stanu
int __stdcall OnStateChange(WPARAM wParam, LPARAM lParam)
{
  if(!ForceUnloadExecuted)
  {
	//Definicja niezbednych zmiennych
	TPluginStateChange StateChange = *(PPluginStateChange)lParam;
	int NewState = StateChange.NewState;
	bool Authorized = StateChange.Authorized;
	//Connecting
	if((!Authorized)&&(NewState))
	 //Ustawianie stanu stanu polaczenia sieci
	 NetworkConnecting = true;
	//Connected
	else if((NetworkConnecting)&&(Authorized)&&(NewState))
	{
	  //Blokowanie dzialania wtyczki
	  BlockFunction = true;
	  //Tworzenie timera do odblokowania dzialania wtyczki
	  KillTimer(hTimerFrm,TIMER_UNBLOCKFUNCTION);
	  SetTimer(hTimerFrm,TIMER_UNBLOCKFUNCTION,10000,(TIMERPROC)TimerFrmProc);
	  //Ustawianie stanu polaczenia sieci
	  NetworkConnecting = false;
	}
	//Disconnected
	else if((NetworkConnecting)&&(Authorized)&&(!NewState))
	 //Ustawianie stanu stanu polaczenia sieci
	 NetworkConnecting = false;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na pokazywanie popupmenu
int __stdcall OnSystemPopUp(WPARAM wParam, LPARAM lParam)
{
  TPluginPopUp PopUp = *(PPluginPopUp)lParam;
  //Pobieranie nazwy popupmenu
  UnicodeString PopUpName = (wchar_t*)PopUp.Name;
  //Popupmenu dostepne spod PPM na kontakcie w oknie kontaktow
  if(PopUpName=="muItem")
  {
	//Pobieranie danych kontatku
	TPluginContact SystemPopUContact = *(PPluginContact)wParam;
	//Pobieranie identyfikatora kontatku
	ContactJID = (wchar_t*)SystemPopUContact.JID;
	//Kontakt znajduje sie na liscie wyjatkow
	if(ExceptionsList->IndexOf(ContactJID)!=-1) ChangeFixStatusCloudFastItem(true);
	//Kontakt nie znajduje sie na liscie
	else ChangeFixStatusCloudFastItem(false);
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmianê kompozycji
int __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
  //Okno ustawien zostalo juz stworzone
  if(hSettingsForm)
  {
	//Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
	  //Pobieranie sciezki nowej aktywnej kompozycji
	  UnicodeString ThemeSkinDir = StringReplace((wchar_t*)lParam, "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
	  //Plik zaawansowanej stylizacji okien istnieje
	  if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	  {
		//Dane pliku zaawansowanej stylizacji okien
		ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
		hSettingsForm->sSkinManager->SkinDirectory = ThemeSkinDir;
		hSettingsForm->sSkinManager->SkinName = "Skin.asz";
		//Ustawianie animacji AlphaControls
		if(ChkThemeAnimateWindows()) hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 200;
		else hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 0;
		hSettingsForm->sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
		//Zmiana kolorystyki AlphaControls
		hSettingsForm->sSkinManager->HueOffset = GetHUE();
	    hSettingsForm->sSkinManager->Saturation = GetSaturation();
		//Aktywacja skorkowania AlphaControls
		hSettingsForm->sSkinManager->Active = true;
	  }
	  //Brak pliku zaawansowanej stylizacji okien
	  else hSettingsForm->sSkinManager->Active = false;
	}
	//Zaawansowana stylizacja okien wylaczona
	else hSettingsForm->sSkinManager->Active = false;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zamkniecie/otwarcie okien
int __stdcall OnWindowEvent(WPARAM wParam, LPARAM lParam)
{
  if(!ForceUnloadExecuted)
  {
	//Pobranie informacji o oknie i eventcie
	TPluginWindowEvent WindowEvent = *(PPluginWindowEvent)lParam;
	int Event = WindowEvent.WindowEvent;
	UnicodeString ClassName = (wchar_t*)WindowEvent.ClassName;
	//Zamkniecie wizytowki kontaktu
	if(((ClassName=="TfrmVCard")||(ClassName=="TInputQueryForm"))&&(Event==WINDOW_EVENT_CLOSE))
	{
	  //Resetowanie listy kontaktow
	  ContactList->Clear();
	  FemalesList->Clear();
	  ContactsNickList->EraseSection("Nick");
	  //Pobranie ID dla enumeracji kontaktów
	  ReplyListID = GetTickCount();
	  //Wywolanie enumeracji kontaktow
	  PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
	}
    //Otwarcie okna ustawien
	if((ClassName=="TfrmSettings")&&(Event==WINDOW_EVENT_CREATE))
	{
	  //Tworzenie timera
	  SetTimer(hTimerFrm,TIMER_CHKSETTINGS,500,(TIMERPROC)TimerFrmProc);
	}
	//Zamkniecie okna ustawien
	if((ClassName=="TfrmSettings")&&(Event==WINDOW_EVENT_CLOSE))
	{
	  //Zatrzymanie timera
	  KillTimer(hTimerFrm,TIMER_CHKSETTINGS);
	  //Sprawdzanie czy dzwiek statusu jest wlaczony
	  TStrings* IniList = new TStringList();
	  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
	  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	  Settings->SetStrings(IniList);
	  UnicodeString StatusSoundEnabled = Settings->ReadString("Sound","SoundStatusActive","1");
	  //Wylaczenie dzwieku statusu za usera
	  if(StrToBool(StatusSoundEnabled))
	  {
		//Nowe ustawienia
		TSaveSetup SaveSetup;
		SaveSetup.Section = L"Sound";
		SaveSetup.Ident = L"SoundStatusActive";
		SaveSetup.Value = L"0";
		//Zapis ustawien
		PluginLink.CallService(AQQ_FUNCTION_SAVESETUP,1,(LPARAM)(&SaveSetup));
		//Odswiezenie ustawien
		PluginLink.CallService(AQQ_FUNCTION_REFRESHSETUP,0,0);
		//Zapisanie pozniejszego przywrocenia
		RestoreStatusSound = true;
	  }
    }
  }

  return 0;
}
//---------------------------------------------------------------------------

//Zapisywanie zasobów
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
  TPluginTwoFlagParams PluginTwoFlagParams;
  PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
  PluginTwoFlagParams.Param1 = ResName;
  PluginTwoFlagParams.Param2 = ResType;
  PluginTwoFlagParams.Flag1 = (int)HInstance;
  PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
  if(FileExists(FileName))
  {
	UnicodeString Result;
	TFileStream *fs;

	fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
	try
	{
	  TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
	  try
	  {
	    Result = idmd5->HashStreamAsHex(fs);
	  }
	  __finally
	  {
	    delete idmd5;
	  }
    }
	__finally
    {
	  delete fs;
	}

	return Result;
  }
  else
   return 0;
}
//---------------------------------------------------------------------------

//Odczyt ustawien
void LoadSettings()
{
  //Odczyt ustawien
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\FixStatusCloud\\\\Settings.ini");
  ModeChk = Ini->ReadInteger("Settings","Mode",1);
  ExceptionsList->Clear();
  MultiExceptionsList->Clear();
  TStringList *pExceptionsList = new TStringList;
  Ini->ReadSection("Exceptions",pExceptionsList);
  int ExceptionsCount = pExceptionsList->Count;
  delete pExceptionsList;
  if(ExceptionsCount>0)
  {
	for(int Count=0;Count<ExceptionsCount;Count++)
	{
	  UnicodeString JID = Ini->ReadString("Exceptions","Item"+IntToStr(Count+1),"");
	  if(!JID.IsEmpty())
	  {
		ExceptionsList->Add(JID);
		MultiExceptionsList->Add(GetContactNick(JID));
	  }
	}
  }
  MultiExceptionsChk = Ini->ReadBool("Settings","MultiExceptions",false);
  PlaySoundChk = Ini->ReadBool("Settings","PlaySound",true);
  ShowStatusChk = Ini->ReadBool("Settings","ShowStatus",false);
  OnStatusChangedChk = Ini->ReadBool("Settings","OnStatusChanged",true);
  OnOfflineChk = Ini->ReadBool("Settings","OnOffline",false);
  OpenMsgChk = Ini->ReadBool("Settings","OpenMsg",true);
  GoogleTTSChk = Ini->ReadBool("Settings","GoogleTTS",false);
  CloudTimeOutVal = Ini->ReadInteger("Settings","CloudTimeOut",5);
  delete Ini;
}
//---------------------------------------------------------------------------

extern "C" int __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
  //Linkowanie wtyczki z komunikatorem
  PluginLink = *Link;
  //Folder z ustawieniami wtyczki
  if(!DirectoryExists(GetPluginUserDir() + "\\\\FixStatusCloud"))
   CreateDir(GetPluginUserDir() + "\\\\FixStatusCloud");
  //Wypakiwanie ikonki FixStatusCloud.dll.png
  //4C24D53551F5AEAFDC80CF0679B13871
  if(!DirectoryExists(GetPluginUserDir() + "\\\\Shared"))
   CreateDir(GetPluginUserDir() + "\\\\Shared");
  if(!FileExists(GetPluginUserDir() + "\\\\Shared\\\\FixStatusCloud.dll.png"))
   ExtractRes((GetPluginUserDir() + "\\\\Shared\\\\FixStatusCloud.dll.png").w_str(),L"SHARED",L"DATA");
  else if(MD5File(GetPluginUserDir() + "\\\\Shared\\\\FixStatusCloud.dll.png")!="4C24D53551F5AEAFDC80CF0679B13871")
   ExtractRes((GetPluginUserDir() + "\\\\Shared\\\\FixStatusCloud.dll.png").w_str(),L"SHARED",L"DATA");
  //Wypakiwanie ikonki FastAccess.png
  //0523012F0B431ECAEA1275D2C1DE6791
  if(!FileExists(GetPluginUserDir() + "\\\\FixStatusCloud\\\\FastAccess.png"))
   ExtractRes((GetPluginUserDir() + "\\\\FixStatusCloud\\\\FastAccess.png").w_str(),L"FASTACCESS",L"DATA");
  else if(MD5File(GetPluginUserDir() + "\\\\FixStatusCloud\\\\FastAccess.png")!="0523012F0B431ECAEA1275D2C1DE6791")
   ExtractRes((GetPluginUserDir() + "\\\\FixStatusCloud\\\\FastAccess.png").w_str(),L"FASTACCESS",L"DATA");
  //Przypisanie ikonki do FASTACCESS interfejsu AQQ
  FASTACCESS = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(GetPluginUserDir() + "\\\\FixStatusCloud\\\\FastAccess.png").w_str());
  //Hook na wylaczenie komunikatora poprzez usera
  PluginLink.HookEvent(AQQ_SYSTEM_BEFOREUNLOAD,OnBeforeUnload);
  //Hook na zmianê stanu kontaktu
  PluginLink.HookEvent(AQQ_CONTACTS_UPDATE,OnContactsUpdate);
  //Hook na zaznaczenie kontaktu na liscie
  PluginLink.HookEvent(AQQ_CONTACTS_BUDDY_SELECTED,OnContactSelected);
  //Hook na zaladowanie listy kontaktow
  PluginLink.HookEvent(AQQ_CONTACTS_LISTREADY,OnListReady);
  //Hook na enumeracje listy kontatkow
  PluginLink.HookEvent(AQQ_CONTACTS_REPLYLIST,OnReplyList);
  //Hook na polaczenie sieci przy starcie AQQ
  PluginLink.HookEvent(AQQ_SYSTEM_SETLASTSTATE,OnSetLastState);
  //Hook na wyswietlanie chmurki informacyjnej
  PluginLink.HookEvent(AQQ_FUNCTION_SHOWINFO,OnShowInfo);
  //Hook dla zmiany stanu
  PluginLink.HookEvent(AQQ_SYSTEM_STATECHANGE,OnStateChange);
  //Hook na pokazywanie popupmenu
  PluginLink.HookEvent(AQQ_SYSTEM_POPUP,OnSystemPopUp);
  //Hook na zmiane kompozycji
  PluginLink.HookEvent(AQQ_SYSTEM_THEMECHANGED, OnThemeChanged);
  //Hook na zamkniecie/otwarcie okien
  PluginLink.HookEvent(AQQ_SYSTEM_WINDOWEVENT,OnWindowEvent);
  //Tworzenie serwisu szybkiego wlaczenia/wylaczenia notyfikacji
  PluginLink.CreateServiceFunction(L"sFixStatusCloudFastItem",ServiceFixStatusCloudFastItem);
  //Tworzenie serwisu szybkiego dostepu do ustawien wtyczki
  PluginLink.CreateServiceFunction(L"sFixStatusCloudFastSettingsItem",ServiceFixStatusCloudFastSettingsItem);
  //Tworzenie elementu szybkiego wlaczenia/wylaczenia notyfikacji
  BuildFixStatusCloudFastItem();
  //Tworzenie interfejsu szybkiego dostepu do ustawien wtyczki
  BuildFixStatusCloudFastSettingsItem();
  //Sprawdzanie czy dzwiek statusu jest wlaczony
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  UnicodeString StatusSoundEnabled = Settings->ReadString("Sound","SoundStatusActive","1");
  //Wylaczenie dzwieku statusu za usera
  if(StrToBool(StatusSoundEnabled))
  {
	//Nowe ustawienia
	TSaveSetup SaveSetup;
	SaveSetup.Section = L"Sound";
	SaveSetup.Ident = L"SoundStatusActive";
	SaveSetup.Value = L"0";
	//Zapis ustawien
	PluginLink.CallService(AQQ_FUNCTION_SAVESETUP,1,(LPARAM)(&SaveSetup));
	//Odswiezenie ustawien
	PluginLink.CallService(AQQ_FUNCTION_REFRESHSETUP,0,0);
	//Zapisanie pozniejszego przywrocenia
	RestoreStatusSound = true;
  }
  //Wczytanie ustawien
  LoadSettings();
  //Wszystkie moduly zostaly zaladowane
  if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
  {
	//Pobranie ID dla enumeracji kontaktów
	ReplyListID = GetTickCount();
	//Wywolanie enumeracji kontaktow
	PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
  }
  //Rejestowanie klasy okna timera
  WNDCLASSEX wincl;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.style = 0;
  wincl.lpfnWndProc = TimerFrmProc;
  wincl.cbClsExtra = 0;
  wincl.cbWndExtra = 0;
  wincl.hInstance = HInstance;
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wincl.lpszMenuName = NULL;
  wincl.lpszClassName = L"TFixStatusCloudTimer";
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  RegisterClassEx(&wincl);
  //Tworzenie okna timera
  hTimerFrm = CreateWindowEx(0, L"TFixStatusCloudTimer", L"",	0, 0, 0, 0, 0, NULL, NULL, HInstance, NULL);

  return 0;
}
//---------------------------------------------------------------------------

//Wyladowanie wtyczki
extern "C" int __declspec(dllexport) __stdcall Unload()
{
  //Wyladowanie timera
  KillTimer(hTimerFrm,TIMER_UNBLOCKFUNCTION);
  //Usuwanie okna timera
  DestroyWindow(hTimerFrm);
  //Wyrejestowanie klasy okna timera
  UnregisterClass(L"TFixStatusCloudTimer",HInstance);
  //Usuwanie interwejsu szybkiego wlaczenia/wylaczenia notyfikacji
  DestroyFixStatusCloudFastItem();
  //Usuwanie serwisu szybkiego wlaczenia/wylaczenia notyfikacji
  PluginLink.DestroyServiceFunction(ServiceFixStatusCloudFastItem);
  //Usuwanie interwejsu szybkiego dostepu do ustawien wtyczki
  DestroyFixStatusCloudFastSettingsItem();
  //Usuwanie serwisu szybkiego dostepu do ustawien wtyczki
  PluginLink.DestroyServiceFunction(ServiceFixStatusCloudFastSettingsItem);
  //Wyladowanie wszystkich hookow
  PluginLink.UnhookEvent(OnBeforeUnload);
  PluginLink.UnhookEvent(OnContactsUpdate);
  PluginLink.UnhookEvent(OnContactSelected);
  PluginLink.UnhookEvent(OnListReady);
  PluginLink.UnhookEvent(OnReplyList);
  PluginLink.UnhookEvent(OnSetLastState);
  PluginLink.UnhookEvent(OnShowInfo);
  PluginLink.UnhookEvent(OnStateChange);
  PluginLink.UnhookEvent(OnSystemPopUp);
  PluginLink.UnhookEvent(OnThemeChanged);
  PluginLink.UnhookEvent(OnWindowEvent);
  //Wlaczanie wczesniej wylaczonego przez wtyczke dzwieku statusu
  if(RestoreStatusSound)
  {
	//Nowe ustawienia
	TSaveSetup SaveSetup;
	SaveSetup.Section = L"Sound";
	SaveSetup.Ident = L"SoundStatusActive";
	SaveSetup.Value = L"1";
	//Zapis ustawien
	PluginLink.CallService(AQQ_FUNCTION_SAVESETUP,1,(LPARAM)(&SaveSetup));
	//Odswiezenie ustawien
	PluginLink.CallService(AQQ_FUNCTION_REFRESHSETUP,0,0);
  }

  return 0;
}
//---------------------------------------------------------------------------

//Ustawienia wtyczki
extern "C" int __declspec(dllexport)__stdcall Settings()
{
  //Przypisanie uchwytu do formy ustawien
  if(!hSettingsForm)
  {
	Application->Handle = (HWND)SettingsForm;
	hSettingsForm = new TSettingsForm(Application);
  }
  //Pokaznie okna ustawien
  hSettingsForm->Show();

  return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" __declspec(dllexport) PPluginInfo __stdcall AQQPluginInfo(DWORD AQQVersion)
{
  PluginInfo.cbSize = sizeof(TPluginInfo);
  PluginInfo.ShortName = L"FixStatusCloud";
  PluginInfo.Version = PLUGIN_MAKE_VERSION(1,1,2,0);
  PluginInfo.Description = L"Poprawia funkcjonalnoœæ chmurki informacyjnej zmiany statusu kontaktu.";
  PluginInfo.Author = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.AuthorMail = L"kontakt@beherit.pl";
  PluginInfo.Copyright = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.Homepage = L"http://beherit.pl";

  return &PluginInfo;
}
//---------------------------------------------------------------------------
