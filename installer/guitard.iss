[Setup]
AppName=GuitarD
AppContact=t.kozel@protonmail.com
AppCopyright=Copyright (C) 2020 Tobias Kozel
AppPublisher=TobiasKozel
AppPublisherURL=https://github.com/TobiasKozel/GuitarD
AppSupportURL=https://github.com/TobiasKozel/GuitarD/issues
AppVersion=0.1.0
VersionInfoVersion=0.1.0
DefaultDirName={pf}\GuitarD
DefaultGroupName=GuitarD
Compression=lzma2
SolidCompression=yes
OutputDir=.\
ArchitecturesInstallIn64BitMode=x64
OutputBaseFilename=GuitarD Installer
LicenseFile=license.rtf
SetupLogging=yes
ShowComponentSizes=no
; WizardImageFile=installer_bg-win.bmp
; WizardSmallImageFile=installer_icon-win.bmp

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Messages]
WelcomeLabel1=Welcome to the GuitarD installer
SetupWindowTitle=GuitarD installer
SelectDirLabel3=The standalone application and supporting files will be installed in the following folder.
SelectDirBrowseLabel=To continue, click Next. If you would like to select a different folder (not recommended), click Browse.

[Components]
Name: "app"; Description: "Standalone application (.exe)"; Types: full custom;
;Name: "vst2_32"; Description: "32-bit VST2 Plugin (.dll)"; Types: full custom;
;Name: "vst2_64"; Description: "64-bit VST2 Plugin (.dll)"; Types: full custom; Check: Is64BitInstallMode;
;Name: "vst3_32"; Description: "32-bit VST3 Plugin (.vst3)"; Types: full custom;
Name: "vst3_64"; Description: "64-bit VST3 Plugin (.vst3)"; Types: full custom; Check: Is64BitInstallMode;
;Name: "aax_32"; Description: "32-bit AAX Plugin (.aaxplugin)"; Types: full custom;
;Name: "aax_64"; Description: "64-bit AAX Plugin (.aaxplugin)"; Types: full custom; Check: Is64BitInstallMode;
Name: "manual"; Description: "User guide"; Types: full custom; Flags: fixed

[Dirs] 
;Name: "{cf32}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Attribs: readonly; Components:aax_32; 
;Name: "{cf64}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Attribs: readonly; Check: Is64BitInstallMode; Components:aax_64; 
;Name: "{cf32}\VST3\GuitarD.vst3\"; Attribs: readonly; Components:vst3_32; 
Name: "{cf64}\VST3\GuitarD.vst3\"; Attribs: readonly; Check: Is64BitInstallMode; Components:vst3_64; 

[Files]
;Source: "..\build-win\GuitarD_Win32.exe"; DestDir: "{app}"; Check: not Is64BitInstallMode; Components:app; Flags: ignoreversion;
Source: "..\build-win\GuitarD_x64.exe"; DestDir: "{app}"; Check: Is64BitInstallMode; Components:app; Flags: ignoreversion;

;Source: "..\build-win\GuitarD_Win32.dll"; DestDir: {code:GetVST2Dir_32}; Check: not Is64BitInstallMode; Components:vst2_32; Flags: ignoreversion;
;Source: "..\build-win\GuitarD_Win32.dll"; DestDir: {code:GetVST2Dir_32}; Check: Is64BitInstallMode; Components:vst2_32; Flags: ignoreversion;
;Source: "..\build-win\GuitarD_x64.dll"; DestDir: {code:GetVST2Dir_64}; Check: Is64BitInstallMode; Components:vst2_64; Flags: ignoreversion;

;Source: "..\build-win\GuitarD.vst3\*.*"; Excludes: "\Contents\x86_64\*,*.pdb,*.exp,*.lib,*.ilk,*.ico,*.ini"; DestDir: "{cf32}\VST3\GuitarD.vst3\"; Components:vst3_32; Flags: ignoreversion recursesubdirs;
;Source: "..\build-win\GuitarD.vst3\Desktop.ini"; DestDir: "{cf32}\VST3\GuitarD.vst3\"; Components:vst3_32; Flags: overwritereadonly ignoreversion; Attribs: hidden system;
;Source: "..\build-win\GuitarD.vst3\PlugIn.ico"; DestDir: "{cf32}\VST3\GuitarD.vst3\"; Components:vst3_32; Flags: overwritereadonly ignoreversion; Attribs: hidden system;

Source: "..\build-win\GuitarD.vst3\*.*"; Excludes: "\Contents\x86\*,*.pdb,*.exp,*.lib,*.ilk,*.ico,*.ini"; DestDir: "{cf64}\VST3\GuitarD.vst3\"; Check: Is64BitInstallMode; Components:vst3_64; Flags: ignoreversion recursesubdirs;
Source: "..\build-win\GuitarD.vst3\Desktop.ini"; DestDir: "{cf64}\VST3\GuitarD.vst3\"; Check: Is64BitInstallMode; Components:vst3_64; Flags: overwritereadonly ignoreversion; Attribs: hidden system;
Source: "..\build-win\GuitarD.vst3\guitard.ico"; DestDir: "{cf64}\VST3\GuitarD.vst3\"; Check: Is64BitInstallMode; Components:vst3_64; Flags: overwritereadonly ignoreversion; Attribs: hidden system;

; Source: "..\build-win\aax\bin\GuitarD.aaxplugin\*.*"; Excludes: "\Contents\x64\*,*.pdb,*.exp,*.lib,*.ilk,*.ico,*.ini"; DestDir: "{cf32}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Components:aax_32; Flags: ignoreversion recursesubdirs;
; Source: "..\build-win\aax\bin\GuitarD.aaxplugin\Desktop.ini"; DestDir: "{cf32}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Components:aax_32; Flags: overwritereadonly ignoreversion; Attribs: hidden system;
; Source: "..\build-win\aax\bin\GuitarD.aaxplugin\PlugIn.ico"; DestDir: "{cf32}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Components:aax_32; Flags: overwritereadonly ignoreversion; Attribs: hidden system;

;Source: "..\build-win\GuitarD.aaxplugin\*.*"; Excludes: "\Contents\Win32\*,*.pdb,*.exp,*.lib,*.ilk,*.ico,*.ini"; DestDir: "{cf64}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Check: Is64BitInstallMode; Components:aax_64; Flags: ignoreversion recursesubdirs;
;Source: "..\build-win\GuitarD.aaxplugin\Desktop.ini"; DestDir: "{cf64}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Check: Is64BitInstallMode; Components:aax_64; Flags: overwritereadonly ignoreversion; Attribs: hidden system;
;Source: "..\build-win\GuitarD.aaxplugin\PlugIn.ico"; DestDir: "{cf64}\Avid\Audio\Plug-Ins\GuitarD.aaxplugin\"; Check: Is64BitInstallMode; Components:aax_64; Flags: overwritereadonly ignoreversion; Attribs: hidden system;

Source: "..\manual\guitard manual.pdf"; DestDir: "{app}"; Flags: isreadme
Source: "license.rtf"; DestDir: "{app}"
;Source: "changelog.txt"; DestDir: "{app}"
;Source: "readme-win.rtf"; DestDir: "{app}"; DestName: "readme.rtf"; Flags: isreadme
;Source: "license.rtf"; DestDir: "{app}"; DestName: "license.rtf"; Flags: isreadme

[Icons]
Name: "{group}\GuitarD"; Filename: "{app}\GuitarD_x64.exe"
Name: "{group}\User guide"; Filename: "{app}\guitard manual.pdf"
Name: "{group}\Changelog"; Filename: "{app}\changelog.txt"
;Name: "{group}\readme"; Filename: "{app}\readme.rtf"
Name: "{group}\Uninstall GuitarD"; Filename: "{app}\unins000.exe"

[Code]
var
  OkToCopyLog : Boolean;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssDone then
    OkToCopyLog := True;
end;

procedure DeinitializeSetup();
begin
  if OkToCopyLog then
    FileCopy (ExpandConstant ('{log}'), ExpandConstant ('{app}\InstallationLogFile.log'), FALSE);
  RestartReplace (ExpandConstant ('{log}'), '');
end;

[UninstallDelete]
Type: files; Name: "{app}\InstallationLogFile.log"