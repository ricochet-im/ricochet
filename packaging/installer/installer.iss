#define ExeVersion GetFileVersion(AddBackslash(SourcePath) + "ricochet.exe")

[Setup]
OutputBaseFilename=Ricochet
AppName=Ricochet
RestartIfNeededByRun=false
PrivilegesRequired=lowest
DefaultDirName={localappdata}\Ricochet\
DisableProgramGroupPage=true
DisableDirPage=false
DisableReadyPage=false
DefaultGroupName=Ricochet
AppID={{B700250B-D3E2-407F-A534-8818EB8E3D93}
AppVersion={#ExeVersion}
UninstallDisplayName=Ricochet
Uninstallable=not IsPortableInstall
VersionInfoDescription=Ricochet
VersionInfoProductName=Ricochet
WizardImageFile=SetupModern11.bmp
ShowLanguageDialog=no
SetupIconFile=ricochet.ico
[Files]
Source: ricochet.exe; DestDir: {app}; DestName: ricochet.exe; Flags: replacesameversion
Source: LICENSE; DestDir: {app}
Source: tor.exe; DestDir: {app}; Flags: replacesameversion uninsrestartdelete
Source: Qt\*; DestDir: {app}; Flags: recursesubdirs
[Icons]
Name: {group}\Ricochet; Filename: {app}\ricochet.exe; WorkingDir: {app}; Comment: {cm:AppTitle}; Check: not IsPortableInstall
Name: {group}\{cm:UninstallShortcut}; Filename: {uninstallexe}; Check: not IsPortableInstall
[UninstallDelete]
Name: {app}\config; Type: filesandordirs
[Run]
Filename: {app}\ricochet.exe; WorkingDir: {app}; Description: {cm:RunShortcut}; Flags: postinstall nowait
[Languages]
Name: "bg"; MessagesFile: "translation\inno\Bulgarian.isl,translation\installer_bg.isl"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl,translation\installer_cs.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl,translation\installer_da.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl,translation\installer_de.isl"
Name: "en"; MessagesFile: "compiler:Default.isl,translation\installer_en.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl,translation\installer_es.isl"
Name: "et_EE"; MessagesFile: "translation\inno\Estonian.isl,translation\installer_et_EE.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl,translation\installer_fi.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl,translation\installer_fr.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl,translation\installer_ja.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl,translation\installer_it.isl"
Name: "it_IT"; MessagesFile: "compiler:Languages\Italian.isl,translation\installer_it_IT.isl"
Name: "nb"; MessagesFile: "compiler:Languages\Norwegian.isl,translation\installer_nb.isl"
Name: "nl_NL"; MessagesFile: "compiler:Languages\Dutch.isl,translation\installer_nl_NL.isl"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl,translation\installer_pt_BR.isl"
Name: "pt_PT"; MessagesFile: "compiler:Languages\Portuguese.isl,translation\installer_pt_PT.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl,translation\installer_ru.isl"
Name: "sv"; MessagesFile: "translation\inno\Swedish.isl,translation\installer_sv.isl"
Name: "sq"; MessagesFile: "translation\inno\Albanian.isl,translation\installer_sq.isl"
Name: "tr"; MessagesFile: "compiler:Languages\Turkish.isl,translation\installer_tr.isl"
Name: "uk"; MessagesFile: "compiler:Languages\Ukrainian.isl,translation\installer_uk.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl,translation\installer_pl.isl"
Name: "sl"; MessagesFile: "compiler:Languages\Slovenian.isl,translation\installer_sl.isl"
Name: "zh"; MessagesFile: "translation\inno\ChineseSimplified.isl,translation\installer_zh.isl"
Name: "zh_HK"; MessagesFile: "translation\inno\ChineseSimplified.isl,translation\installer_zh_HK.isl"

[Code]
// http://www.vincenzo.net/isxkb/index.php?title=Obtaining_the_application's_version
// http://www.vincenzo.net/isxkb/index.php?title=Uninstall_user_files

var
	PortablePage: TInputOptionWizardPage;

procedure InitializeWizard;
begin
	PortablePage := CreateInputOptionPage(wpWelcome, CustomMessage('PortableTitle'), CustomMessage('PortableDesc'),
		CustomMessage('PortableText') + #13#13, True, False);
	PortablePage.Add(CustomMessage('PortableOptInstall'));
	PortablePage.Add(CustomMessage('PortableOptExtract'));
	PortablePage.Values[0] := True;
end;

function IsPortableInstall(): Boolean;
begin
	Result := PortablePage.Values[1];
end;

procedure CurPageChanged(CurPageID: Integer);
var
	s: String;
	DefaultPortableDir: String;
	DefaultInstallDir: String;
begin
	if CurPageID = wpSelectDir then begin
		DefaultPortableDir := ExtractFilePath(ExpandConstant('{srcexe}')) + 'Ricochet';
		DefaultInstallDir := ExpandConstant('{localappdata}') + '\Ricochet';

		if IsPortableInstall() then begin
			WizardForm.NextButton.Caption := CustomMessage('BtnExtract');
			WizardForm.SelectDirLabel.Caption := CustomMessage('ExtractDirText');
			WizardForm.PageDescriptionLabel.Caption := CustomMessage('ExtractDirDesc');
			if WizardForm.DirEdit.Text = DefaultInstallDir then
				WizardForm.DirEdit.Text := DefaultPortableDir;
		end else begin
			WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall);
			s := SetupMessage(msgSelectDirLabel3);
			StringChangeEx(s, '[name]', 'Ricochet', True);
			WizardForm.SelectDirLabel.Caption := s;
			s := SetupMessage(msgSelectDirDesc);
			StringChangeEx(s, '[name]', 'Ricochet', True);
			WizardForm.PageDescriptionLabel.Caption := s;
			if WizardForm.DirEdit.Text = DefaultPortableDir then
				WizardForm.DirEdit.Text := DefaultInstallDir;
		end;
	end;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
	if (PageID = wpSelectDir) and (not IsPortableInstall()) then
		Result := True
	else if (PageID = wpReady) and (IsPortableInstall()) then
	    Result := True
	else
	    Result := False;
end;



