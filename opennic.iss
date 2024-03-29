; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "OpenNIC Wizard"
#define MyAppServiceName "OpenNIC Service"
#define MyAppVersion "0.6.beta1"
#define MyAppPublisher "The OpenNIC Project"
#define MyAppURL "http://github.com/8bitgeek/OpenNIC-Wizard"
#define MyAppExeName "opennicwizui.exe"
#define MyAppServiceName "opennicwizd.exe"

; My Windows 64-bit configuration
#define MyOutputDir           "C:\cygwin64\home\mike.sharkey\"
#define MyOpenNICSource       "C:\cygwin64\home\mike.sharkey\development\github\OpenNIC-Wizard"
#define MyOpenNICServerBuild  "C:\cygwin64\home\mike.sharkey\development\github\OpenNIC-Wizard\server\debug"
#define MyOpenNICClientBuild  "C:\cygwin64\home\mike.sharkey\development\github\OpenNIC-Wizard\client\debug"
#define MyQtLib               "C:\Qt\5.15.0\mingw81_64\bin"
#define MyMingwBin            "C:\Qt\5.15.0\mingw81_64\bin"
#define MyQtPlugins           "C:\Qt\5.15.0\mingw81_64\plugins"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C2F9E08D-39B2-4475-8CC3-1D08D8FD5FF5}}
PrivilegesRequired=admin
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\OpenNIC
DefaultGroupName=OpenNIC
LicenseFile={#MyOpenNICSource}\COPYING
OutputDir={#MyOutputDir}
OutputBaseFilename=opennicwiz-{#MyAppVersion}
WizardImageFile={#MyOpenNICSource}\installer.bmp
SetupIconFile={#MyOpenNICSource}\client\images\opennic.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Files]
Source: {#MyOpenNICClientBuild}\opennicwizui.exe; DestDir: {app}; Flags: ignoreversion; Permissions: admins-full; MinVersion: 0,5.1.2600;
Source: {#MyOpenNICServerBuild}\opennicwizd.exe; DestDir: {app}; Flags: ignoreversion; Permissions: admins-full; MinVersion: 0,5.1.2600;
Source: {#MyQtLib}\Qt5Core.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtLib}\Qt5Gui.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtLib}\Qt5Widgets.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtLib}\Qt5Network.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtLib}\Qt5Script.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtLib}\Qt5Script.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyQtPlugins}\platforms\qwindows.dll; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: {#MyQtPlugins}\platforms\qdirect2d.dll; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: {#MyQtPlugins}\platforms\qminimal.dll; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: {#MyQtPlugins}\platforms\qoffscreen.dll; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: {#MyQtPlugins}\platforms\qwebgl.dll; DestDir: "{app}\platforms"; Flags: ignoreversion

Source: {#MyOpenNICSource}\server\dig\install\bindevt.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\dig.exe; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libbind9.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libdns.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libeay32.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libisc.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libisccc.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\libisccfg.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\server\dig\install\liblwres.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\unins000.exe.manifest; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\COPYING; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\README.md; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyOpenNICSource}\etc\bootstrap.t1; DestDir: {app}; Flags: ignoreversion; 
Source: {#MyOpenNICSource}\etc\bootstrap.domains; DestDir: {app}; Flags: ignoreversion; 
Source: {#MyOpenNICSource}\server\opennicwizd.exe.manifest; DestDir: {app}; Flags: ignoreversion; 

Source: {#MyMingwBin}\libgcc_s_seh-1.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyMingwBin}\libwinpthread-1.dll; DestDir: "{app}"; Flags: ignoreversion
Source: {#MyMingwBin}\libstdc++-6.dll; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commonstartup}\OpenNIC Wizard"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppServiceName}"; Parameters: "-install"; WorkingDir: "{app}"; Flags:  runascurrentuser; Description: "{cm:LaunchProgram,{#StringChange(MyAppServiceName, "&", "&&")}}"; StatusMsg: "Installing OpenNIC Service...";
Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Flags: nowait runasoriginaluser; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; StatusMsg: "Starting OpenNIC Task Tray Applet...";
FileName: "{sys}\sc.exe"; Parameters: "config OpenNICWizard start= auto"; Flags: runascurrentuser; StatusMsg: "Configuring OpenNIC Service...";
Filename: "{sys}\sc.exe"; Parameters: "start OpenNICWizard"; Flags: runascurrentuser; StatusMsg: "Starting OpenNIC Service...";
Filename: "http://grep.geek"; Description: "Open http://grep.geek in a web browser"; Flags: shellexec postinstall runasoriginaluser

[UninstallRun]
Filename: "{app}\{#MyAppServiceName}"; WorkingDir: "{app}"; Parameters: "-terminate";
Filename: "{app}\{#MyAppServiceName}"; WorkingDir: "{app}"; Parameters: "-uninstall";
Filename: "{sys}\sc.exe"; Parameters: "stop OpenNICWizard";
Filename: "{sys}\sc.exe"; Parameters: "delete OpenNICWizard";

[Code]
function InitializeSetup(): Boolean;
begin
  Log('InitializeSetup() called');
  Result := True;
  if not IsAdminInstallMode then
  begin
    MsgBox('This installer must be run as administrator',mbCriticalError,MB_OK);
    Result := False;
  end;
end;
