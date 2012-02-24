; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "OpenNIC Setup"
#define MyAppVersion "0.1"
#define MyAppPublisher "The OpenNIC Project"
#define MyAppURL "http://www.opennicproject.org"
#define MyAppExeName "OpenNIC.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{1D9BA5F4-E722-4670-804D-4BED7B4A26DA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\OpenNIC
DefaultGroupName=OpenNIC
LicenseFile=C:\Documents and Settings\Mike\My Documents\OpenNIC\COPYING
OutputDir=C:\Documents and Settings\Mike\My Documents
OutputBaseFilename=OpenNIC Setup
SetupIconFile=C:\Documents and Settings\Mike\My Documents\OpenNIC\images\opennic.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1
;Name: "startup\common"; Description: "For all users"; GroupDescription: "{cm:AdditionalIcons}"; Flags: exclusive

[Files]
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC-build-desktop-Qt_4_7_4_for_Desktop_-_MinGW_4_4__Qt_SDK__Debug\debug\OpenNIC.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtSDK\Desktop\Qt\4.7.4\mingw\lib\QtCored4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtSDK\Desktop\Qt\4.7.4\mingw\lib\QtGuid4.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\bindevt.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\dig.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libbind9.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libdns.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libisc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libisccc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\libisccfg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\dig\install\liblwres.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Mike\My Documents\OpenNIC\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

Source: "C:\QtSDK\mingw\bin\mingwm10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtSDK\mingw\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent

