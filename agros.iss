[Setup]

#define AppName "Agros Suite"
#define AppVersion GetFileVersion("agros.exe")
#define AppDate GetFileDateTimeString("agros.exe", "mm/dd/yyyy", "/", ":") 
; #define OutputBaseFileName "AgrosSuite" + "_" + AppVersion + "." + GetDateTimeString('yyyymmdd', '', '')
#define OutputBaseFileName "AgrosSuite"

AppName={#AppName}
AppVerName={#AppName} {#AppVersion} ({#AppDate})
OutputBaseFilename={#OutputBaseFileName}
DefaultGroupName=Agros Suite
LicenseFile=COPYING

AppId=Agros Suite
AppPublisher=hpfem.org
AppCopyright=hpfem.org
AppPublisherURL=http://www.agros2d.org/
AppMutex=Agros Suite
OutputDir=setup
DefaultDirName={pf}\Agros Suite
UninstallDisplayIcon={app}\images\agros.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=resources_source\images\setup\SetupModern.bmp
WizardSmallImageFile=resources_source\images\setup\SetupModernSmall.bmp

[Languages]

[Files]
Source: agros_library.dll; DestDir: {app}; DestName: agros_library.dll
Source: libs\*.dll; DestDir: {app}/libs
Source: agros.exe; DestDir: {app}; DestName: Agros.exe
Source: solver_plugin_MUMPS.dll; DestDir: {app}; DestName: solver_plugin_MUMPS.dll
Source: resources\images\agros.ico; DestDir: {app}; DestName: Agros.ico
Source: resources\*; DestDir: {app}/resources; Flags: recursesubdirs

Source: ..\install\*; DestDir: {app}; Flags: recursesubdirs
; VC++ 2013 runtime
Source: "..\install\VC_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Icons]
Name: {group}\Agros; Filename: {app}\Agros.exe; WorkingDir: {app}
Name: {group}\Web pages; Filename: {app}\Agros2D.url
Name: {group}\COPYING; Filename: {app}\COPYING
Name: {group}\Uninstall; Filename: {uninstallexe}
Name: {commondesktop}\Agros; Filename: {app}\Agros.exe; WorkingDir: {app}; Tasks: desktopicon

[Tasks]
Name: desktopicon; Description: Create icon on desktop


[Registry]
; a2d
Root: HKCR; SubKey: .ags; ValueType: string; ValueData: Agros.Data; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros.Data; ValueType: string; ValueData: Agros data file; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros.Data\Shell\Open\Command; ValueType: string; ValueData: """{app}\Agros.exe"" -p ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Agros.Data\DefaultIcon; ValueType: string; ValueData: {app}\Agros.ico; Flags: uninsdeletevalue

[INI]
Filename: {app}\Agros2D.url; Section: InternetShortcut; Key: URL; String: http://www.agros2d.org/

[UninstallDelete]
Type: files; Name: {app}\Agros2D.url

[Code]
#IFDEF UNICODE
  #DEFINE AW "W"
#ELSE
  #DEFINE AW "A"
#ENDIF
type
  INSTALLSTATE = Longint;
const
  INSTALLSTATE_INVALIDARG = -2;  // An invalid parameter was passed to the function.
  INSTALLSTATE_UNKNOWN = -1;     // The product is neither advertised or installed.
  INSTALLSTATE_ADVERTISED = 1;   // The product is advertised but not installed.
  INSTALLSTATE_ABSENT = 2;       // The product is installed for a different user.
  INSTALLSTATE_DEFAULT = 5;      // The product is installed for the current user.


function MsiQueryProductState(szProduct: string): INSTALLSTATE; external 'MsiQueryProductState{#AW}@msi.dll stdcall';

function VCVersionInstalled(const ProductID: string): Boolean;
begin
  Result := MsiQueryProductState(ProductID) = INSTALLSTATE_DEFAULT;
end;

[UninstallRun]
Filename: "VC_redist.x64.exe"; Parameters: "/uninstall /q /norestart"; StatusMsg: "Odinstalace Visual C++ Redistributable x64..."; Flags: waituntilterminated
