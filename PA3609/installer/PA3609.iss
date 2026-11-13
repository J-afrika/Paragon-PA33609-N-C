[Setup]
AppId={{7B1C2E64-5A3D-4F0B-9C11-3A9E6F0D2B47}
AppName=Paragon PA-3609 (VST3)
AppVersion=0.1.0
AppPublisher=Paragon Audio
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputBaseFilename=Paragon-PA3609-VST3-Setup
OutputDir=Output
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Uninstallable=yes
CreateUninstallRegKey=yes
UninstallDisplayName=Paragon PA-3609 (VST3)

[Files]
Source: "..\build\PA3609_artefacts\Release\VST3\Paragon PA-3609.vst3\*"; DestDir: "{commoncf64}\VST3\Paragon PA-3609.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\Paragon PA-3609.vst3"
