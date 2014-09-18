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
[Files]
Source: ricochet.exe; DestDir: {app}; DestName: ricochet.exe; Flags: replacesameversion
Source: ..\..\LICENSE; DestDir: {app}
Source: tor.exe; DestDir: {app}; Flags: replacesameversion uninsrestartdelete
Source: Qt\*; DestDir: {app}; Flags: recursesubdirs
Source: MSVCP120.DLL; DestDir: {app}
Source: MSVCR120.DLL; DestDir: {app}
[Icons]
Name: {group}\Ricochet; Filename: {app}\ricochet.exe; WorkingDir: {app}; Comment: {cm:AppTitle}; Check: not IsPortableInstall
Name: {group}\{cm:UninstallShortcut}; Filename: {uninstallexe}; Check: not IsPortableInstall
[UninstallDelete]
Name: {app}\config; Type: filesandordirs
[Run]
Filename: {app}\ricochet.exe; WorkingDir: {app}; Description: {cm:RunShortcut}; Flags: postinstall nowait
[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl,..\..\translation\installer_en.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl,..\..\translation\installer_it.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl,..\..\translation\installer_es.isl"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl,..\..\translation\installer_pt_BR.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl,..\..\translation\installer_da.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl,..\..\translation\installer_de.isl"

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



