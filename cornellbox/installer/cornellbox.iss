; ============================================================================
; cornellbox.iss  ·  Instalador para Windows (Inno Setup 6)
; Genera "CornellBox-Setup.exe": copia el programa, los shaders y los DLL,
; crea accesos directos en el menu Inicio y un desinstalador.
;
; Antes de compilar este script:
;   1) En MSYS2 MinGW x64:  cd cornellbox && mingw32-make dist
;      (deja el binario, los shaders y los DLL en cornellbox\dist\cornellbox\)
;   2) Abre este .iss con Inno Setup y pulsa Compilar (o:  iscc cornellbox.iss)
; Ver guia completa en LEEME_INSTALADOR.md.
; ============================================================================

#define MyAppName "Caja de Cornell - Modelos de Iluminacion"
#define MyAppShortName "CornellBox"
#define MyAppVersion "1.0"
#define MyAppPublisher "Steveen Maque Espinoza"
#define MyAppExeName "cornellbox.exe"

; Carpeta con el binario, shaders y DLL (resultado de "mingw32-make dist").
; Se puede sobreescribir al compilar:  iscc /DSourceDir="ruta" cornellbox.iss
#ifndef SourceDir
  #define SourceDir "..\dist\cornellbox"
#endif

[Setup]
AppId={{A5F3C2E1-7B4D-4E9A-9C1F-3D8E6B2A4F70}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppShortName}
DefaultGroupName={#MyAppShortName}
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
OutputDir=Output
OutputBaseFilename=CornellBox-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Licencia mostrada en el asistente (se incluye en "mingw32-make dist").
LicenseFile={#SourceDir}\LICENSE

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Ejecutable
Source: "{#SourceDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; Todos los DLL (GLFW, GLEW y runtime de MinGW)
Source: "{#SourceDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
; Shaders (el programa los busca junto al ejecutable)
Source: "{#SourceDir}\shaders\*"; DestDir: "{app}\shaders"; Flags: ignoreversion recursesubdirs createallsubdirs
; Documentacion
Source: "{#SourceDir}\README.md"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#SourceDir}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\Desinstalar {#MyAppShortName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppShortName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppShortName}}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent
