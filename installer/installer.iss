[Setup]
OutputBaseFilename=Torsion
AppName=Torsion IM
RestartIfNeededByRun=false
PrivilegesRequired=lowest
DefaultDirName={localappdata}\Torsion\
DisableProgramGroupPage=true
DisableDirPage=false
DisableReadyPage=true
DefaultGroupName=Torsion IM
AppID={{B700250B-D3E2-407F-A534-8818EB8E3D93}
AppVersion=1.0.0
UninstallDisplayName=Torsion IM
Uninstallable=not IsPortableInstall
VersionInfoDescription=Torsion - Anonymous IM
VersionInfoProductName=Torsion
WizardImageFile=SetupModern11.bmp
[Files]
Source: ..\release\Torsion.exe; DestDir: {app}; DestName: Torsion.exe; Flags: replacesameversion
Source: ..\COPYING; DestDir: {app}
Source: ..\README.txt; DestDir: {app}
Source: Tor\LICENSE; DestDir: {app}\Tor
Source: Tor\README.txt; DestDir: {app}\Tor
Source: Tor\tor.exe; DestDir: {app}\Tor; Flags: replacesameversion uninsrestartdelete
[Icons]
Name: {group}\Torsion; Filename: {app}\Torsion.exe; WorkingDir: {app}; Comment: Start Torsion IM; Check: not IsPortableInstall
Name: {group}\Uninstall Torsion IM; Filename: {uninstallexe}; Check: not IsPortableInstall
[Run]
Filename: {app}\Torsion.exe; WorkingDir: {app}; Description: Launch Torsion IM; Flags: postinstall nowait
[Messages]
WelcomeLabel2=This will install [name] on your computer.
[Dirs]
Name: {app}\Tor
[UninstallDelete]
Name: {app}\Tor; Type: filesandordirs
[CustomMessages]
PortableDesc=Do you want a portable installation?
PortableText=Torsion can be installed on your system, or extracted to a portable folder. The portable installation can be moved between computers or kept secure on an encrypted harddrive.
PortableTitle=Installation Mode
PortableOptInstall=Install (Recommended)
PortableOptExtract=Extract (Portable)
BtnExtract=Extract
ExtractDirText=Torsion will be extracted into the following folder
ExtractDirDesc=Where should Torsion be extracted?
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
			StringChangeEx(s, '[name]', 'Torsion IM', True);
			WizardForm.SelectDirLabel.Caption := s;
			s := SetupMessage(msgSelectDirDesc);
			StringChangeEx(s, '[name]', 'Torsion IM', True);
			WizardForm.PageDescriptionLabel.Caption := s;
			if WizardForm.DirEdit.Text = DefaultPortableDir then
				WizardForm.DirEdit.Text := DefaultInstallDir;
		end;
	end;
end;
