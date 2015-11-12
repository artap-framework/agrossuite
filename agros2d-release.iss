[Setup]

#define AppName "Agros2D"
#define AppVersion GetFileVersion("agros2d.exe")
#define AppDate GetFileDateTimeString("agros2d.exe", "mm/dd/yyyy", "/", ":") 
#define OutputBaseFileName "Agros2D" + "_" + AppVersion + "." + GetDateTimeString('yyyymmdd', '', '')

AppName={#AppName}
AppVerName={#AppName} {#AppVersion} ({#AppDate})
OutputBaseFilename={#OutputBaseFileName}
DefaultGroupName=Agros2D
LicenseFile=COPYING

AppId=Agros2D
AppPublisher=hpfem.org
AppCopyright=hpfem.org
AppPublisherURL=http://www.agros2d.org/
AppMutex=Agros2D
OutputDir=setup
DefaultDirName={pf}\Agros2D
UninstallDisplayIcon={app}\images\agros2d.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=resources_source\images\setup\SetupModern.bmp
WizardSmallImageFile=resources_source\images\setup\SetupModernSmall.bmp

[Languages]

[Files]
Source: agros2d_util.dll; DestDir: {app}; DestName: agros2d_util.dll
Source: agros2d_pythonlab_library.dll; DestDir: {app}; DestName: agros2d_pythonlab_library.dll
Source: agros_pythonlab.exe; DestDir: {app}; DestName: PythonLab.exe
Source: agros2d_library.dll; DestDir: {app}; DestName: agros2d_library.dll
Source: libs\*.dll; DestDir: {app}\libs
Source: agros2d.exe; DestDir: {app}; DestName: Agros2D.exe
;Source: agros2d_solver.exe; DestDir: {app}; DestName: Solver.exe
;Source: agros2d_remote.exe; DestDir: {app}; DestName: Remote.exe
;Source: solver_external.exe; DestDir: {app}; DestName: Solver_External.exe
Source: resources\images\agros2d.ico; DestDir: {app}; DestName: Agros2D.ico
Source: resources\*; DestDir: {app}/resources; Flags: recursesubdirs
Source: ..\agros2d-win32-new-release\*; DestDir: {app}; Flags: recursesubdirs
; VC++ 2013 runtime
Source: "..\vcredist_x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Icons]
Name: {group}\Agros2D; Filename: {app}\Agros2D.exe; WorkingDir: {app}
Name: {group}\Web pages; Filename: {app}\Agros2D.url
Name: {group}\COPYING; Filename: {app}\COPYING
Name: {group}\Uninstall; Filename: {uninstallexe}
Name: {commondesktop}\Agros2D; Filename: {app}\Agros2D.exe; WorkingDir: {app}; Tasks: desktopicon

[Tasks]
Name: desktopicon; Description: Create icon on desktop

[Run]
Filename: "{tmp}\vcredist_x86.exe"; Check: VCRedistNeedsInstall

[Registry]
; a2d
Root: HKCR; SubKey: .a2d; ValueType: string; ValueData: Agros2D.Data; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros2D.Data; ValueType: string; ValueData: Agros2D data file; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros2D.Data\Shell\Open\Command; ValueType: string; ValueData: """{app}\Agros2D.exe"" -p ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Agros2D.Data\DefaultIcon; ValueType: string; ValueData: {app}\Agros2D.ico; Flags: uninsdeletevalue

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

  VC_2013_REDIST_X86 = '{13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}';
  VC_2013_REDIST_x64 = '{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}';

function MsiQueryProductState(szProduct: string): INSTALLSTATE; external 'MsiQueryProductState{#AW}@msi.dll stdcall';

function VCVersionInstalled(const ProductID: string): Boolean;
begin
  Result := MsiQueryProductState(ProductID) = INSTALLSTATE_DEFAULT;
end;

function VCRedistNeedsInstall: Boolean;
begin
  // here the Result must be True when you need to install your VCRedist
  // or False when you don't need to, so now it's upon you how you build
  // this statement, the following won't install your VC redist only when
  // the Visual C++ 2013 Redist (x86) and Visual C++ 2010 SP1 Redist(x86)
  // are installed for the current user
  Result := not (VCVersionInstalled(VC_2013_REDIST_X86));
end;

