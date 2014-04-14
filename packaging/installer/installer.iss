#define ExeVersion GetFileVersion(AddBackslash(SourcePath) + "Torsion.exe")

[Setup]
OutputBaseFilename=Torsion
AppName=Torsion
RestartIfNeededByRun=false
PrivilegesRequired=lowest
DefaultDirName={localappdata}\Torsion\
DisableProgramGroupPage=true
DisableDirPage=false
DisableReadyPage=false
DefaultGroupName=Torsion
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
AppID={{B700250B-D3E2-407F-A534-8818EB8E3D93}
AppVersion={#ExeVersion}
UninstallDisplayName=Torsion
Uninstallable=not IsPortableInstall
VersionInfoDescription=Torsion
VersionInfoProductName=Torsion
WizardImageFile=SetupModern11.bmp
ShowLanguageDialog=no
[Files]
Source: Torsion.exe; DestDir: {app}; DestName: Torsion.exe; Flags: replacesameversion
Source: ..\..\LICENSE; DestDir: {app}
Source: tor.exe; DestDir: {app}; Flags: replacesameversion uninsrestartdelete
Source: Qt\*; DestDir: {app}; Flags: recursesubdirs
Source: MSVCP110.DLL; DestDir: {app}
Source: MSVCR110.DLL; DestDir: {app}
[Icons]
Name: {group}\Torsion; Filename: {app}\Torsion.exe; WorkingDir: {app}; Comment: {cm:AppTitle}; Check: not IsPortableInstall
Name: {group}\{cm:UninstallShortcut}; Filename: {uninstallexe}; Check: not IsPortableInstall
[UninstallDelete]
Name: {app}\config; Type: filesandordirs
[Run]
Filename: {app}\Torsion.exe; WorkingDir: {app}; Description: {cm:RunShortcut}; Flags: postinstall nowait
[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl,..\..\translation\installer_en.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl,..\..\translation\installer_it.isl"

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
		DefaultPortableDir := ExtractFilePath(ExpandConstant('{srcexe}')) + 'Torsion';
		DefaultInstallDir := ExpandConstant('{localappdata}') + '\Torsion';

		if IsPortableInstall() then begin
			WizardForm.NextButton.Caption := CustomMessage('BtnExtract');
			WizardForm.SelectDirLabel.Caption := CustomMessage('ExtractDirText');
			WizardForm.PageDescriptionLabel.Caption := CustomMessage('ExtractDirDesc');
			if WizardForm.DirEdit.Text = DefaultInstallDir then
				WizardForm.DirEdit.Text := DefaultPortableDir;
		end else begin
			WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall);
			s := SetupMessage(msgSelectDirLabel3);
			StringChangeEx(s, '[name]', 'Torsion', True);
			WizardForm.SelectDirLabel.Caption := s;
			s := SetupMessage(msgSelectDirDesc);
			StringChangeEx(s, '[name]', 'Torsion', True);
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



