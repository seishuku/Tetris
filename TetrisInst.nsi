!include "MUI2.nsh"

Name "NitroGL HDR Tetris"

OutFile "HDRTetrisInst.exe"

InstallDir "$PROGRAMFILES\NitroGL HDR Tetris"
InstallDirRegKey HKCU "Software\NitroGL HDR Tetris" ""

RequestExecutionLevel user

Var StartMenuFolder

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\NitroGL HDR Tetris"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Program Files" SecProgram
  SetOutPath "$INSTDIR"

  File "blossom.scn"
  File "blossom.tga"
  File "blossom_s.tga"
  File "blur_f.glsl"
  File "blur_v.glsl"
  File "border.3ds"
  File "composite_f.glsl"
  File "composite_v.glsl"
  File "drop.wav"
  File "fcube.3ds"
  File "galileo.scn"
  File "galileo.tga"
  File "galileo_s.tga"
  File "generic_v.glsl"
  File "glass_f.glsl"
  File "grace.scn"
  File "grace.tga"
  File "grace_s.tga"
  File "levelup.wav"
  File "License.txt"
  File "lighting_f.glsl"
  File "line.wav"
  File "line4.wav"
  File "milkglass_f.glsl"
  File "move.wav"
  File "Readme.txt"
  File "rnl.scn"
  File "rnl.tga"
  File "rnl_s.tga"
  File "rotate.wav"
  File "scube.3ds"
  File "skybox_f.glsl"
  File "skybox_v.glsl"
  File "sphere.3ds"
  File "stpeters.scn"
  File "stpeters.tga"
  File "stpeters_s.tga"
  File "tetris.exe"
  File "uffizi.scn"
  File "uffizi.tga"
  File "uffizi_s.tga"

  ExecShell open "Readme.txt"

  WriteRegStr HKCU "Software\NitroGLHDRTetris" "" $INSTDIR

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Tetris.lnk" "$INSTDIR\Tetris.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Readme.lnk" "$INSTDIR\Readme.txt"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Source Code" SecSource
  SetOutPath "$INSTDIR"
  File "3ds.c"
  File "3ds.h"
  File "dds.c"
  File "engine.c"
  File "font.c"
  File "font.h"
  File "fontdata.h"
  File "image.c"
  File "image.h"
  File "math.c"
  File "math.h"
  File "opengl.c"
  File "opengl.h"
  File "tetris.c"
  File "tetris.h"
  File "tetris.sln"
  File "tetris.vcxproj"
  File "tetris.vcxproj.filters"
  File "tga.c"
  SetOutPath "$INSTDIR\Tremor"
  File "Tremor\backends.h"
  File "Tremor\bitwise.c"
  File "Tremor\block.c"
  File "Tremor\codebook.c"
  File "Tremor\codebook.h"
  File "Tremor\codec_internal.h"
  File "Tremor\config_types.h"
  File "Tremor\floor0.c"
  File "Tremor\floor1.c"
  File "Tremor\framing.c"
  File "Tremor\info.c"
  File "Tremor\ivorbiscodec.h"
  File "Tremor\ivorbisfile.h"
  File "Tremor\lsp_lookup.h"
  File "Tremor\mapping0.c"
  File "Tremor\mdct.c"
  File "Tremor\mdct.h"
  File "Tremor\mdct_lookup.h"
  File "Tremor\misc.h"
  File "Tremor\ogg.h"
  File "Tremor\os.h"
  File "Tremor\os_types.h"
  File "Tremor\registry.c"
  File "Tremor\registry.h"
  File "Tremor\res012.c"
  File "Tremor\sharedbook.c"
  File "Tremor\synthesis.c"
  File "Tremor\vorbisfile.c"
  File "Tremor\window.c"
  File "Tremor\window.h"
  File "Tremor\window_lookup.h"

  WriteRegStr HKCU "Software\NitroGLHDRTetris" "" $INSTDIR

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Source Code.lnk" "$INSTDIR"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "OpenAL Runtime Install" SecOpenAL
  SetOutPath "$TEMP"

  File "oalinst.exe"
  Exec '$TEMP\oalinst.exe'
SectionEnd

LangString DESC_SecProgram ${LANG_ENGLISH} "The main program"
LangString DESC_SecSource ${LANG_ENGLISH} "The program source code"
LangString DESC_SecOpenAL ${LANG_ENGLISH} "OpenAL sound libaray runtime"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSource} $(DESC_SecSource)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenAL} $(DESC_SecOpenAL)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  Delete "$INSTDIR\blossom.scn"
  Delete "$INSTDIR\blossom.tga"
  Delete "$INSTDIR\blossom_s.tga"
  Delete "$INSTDIR\blur_f.glsl"
  Delete "$INSTDIR\blur_v.glsl"
  Delete "$INSTDIR\border.3ds"
  Delete "$INSTDIR\composite_f.glsl"
  Delete "$INSTDIR\composite_v.glsl"
  Delete "$INSTDIR\cube.3ds"
  Delete "$INSTDIR\drop.wav"
  Delete "$INSTDIR\font_f.glsl"
  Delete "$INSTDIR\font_v.glsl"
  Delete "$INSTDIR\galileo.scn"
  Delete "$INSTDIR\galileo.tga"
  Delete "$INSTDIR\galileo_s.tga"
  Delete "$INSTDIR\generic_v.glsl"
  Delete "$INSTDIR\grace.scn"
  Delete "$INSTDIR\grace.tga"
  Delete "$INSTDIR\grace_s.tga"
  Delete "$INSTDIR\levelup.wav"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\lighting_f.glsl"
  Delete "$INSTDIR\line.wav"
  Delete "$INSTDIR\line4.wav"
  Delete "$INSTDIR\move.wav"
  Delete "$INSTDIR\Readme.txt"
  Delete "$INSTDIR\reflect_f.glsl"
  Delete "$INSTDIR\rnl.scn"
  Delete "$INSTDIR\rnl.tga"
  Delete "$INSTDIR\rnl_s.tga"
  Delete "$INSTDIR\rotate.wav"
  Delete "$INSTDIR\skybox_f.glsl"
  Delete "$INSTDIR\skybox_v.glsl"
  Delete "$INSTDIR\sphere.3ds"
  Delete "$INSTDIR\stpeters.scn"
  Delete "$INSTDIR\stpeters.tga"
  Delete "$INSTDIR\stpeters_s.tga"
  Delete "$INSTDIR\tetris.exe"
  Delete "$INSTDIR\uffizi.scn"
  Delete "$INSTDIR\uffizi.tga"
  Delete "$INSTDIR\uffizi_s.tga"
  Delete "$INSTDIR\3ds.c"
  Delete "$INSTDIR\3ds.h"
  Delete "$INSTDIR\dds.c"
  Delete "$INSTDIR\engine.c"
  Delete "$INSTDIR\font.c"
  Delete "$INSTDIR\font.h"
  Delete "$INSTDIR\fontdata.h"
  Delete "$INSTDIR\image.c"
  Delete "$INSTDIR\image.h"
  Delete "$INSTDIR\math.c"
  Delete "$INSTDIR\math.h"
  Delete "$INSTDIR\opengl.c"
  Delete "$INSTDIR\opengl.h"
  Delete "$INSTDIR\tetris.c"
  Delete "$INSTDIR\tetris.h"
  Delete "$INSTDIR\tetris.sln"
  Delete "$INSTDIR\tetris.vcxproj"
  Delete "$INSTDIR\tetris.vcxproj.filters"
  Delete "$INSTDIR\tga.c"
  Delete "$INSTDIR\Tremor\backends.h"
  Delete "$INSTDIR\Tremor\bitwise.c"
  Delete "$INSTDIR\Tremor\block.c"
  Delete "$INSTDIR\Tremor\codebook.c"
  Delete "$INSTDIR\Tremor\codebook.h"
  Delete "$INSTDIR\Tremor\codec_internal.h"
  Delete "$INSTDIR\Tremor\config_types.h"
  Delete "$INSTDIR\Tremor\floor0.c"
  Delete "$INSTDIR\Tremor\floor1.c"
  Delete "$INSTDIR\Tremor\framing.c"
  Delete "$INSTDIR\Tremor\info.c"
  Delete "$INSTDIR\Tremor\ivorbiscodec.h"
  Delete "$INSTDIR\Tremor\ivorbisfile.h"
  Delete "$INSTDIR\Tremor\lsp_lookup.h"
  Delete "$INSTDIR\Tremor\mapping0.c"
  Delete "$INSTDIR\Tremor\mdct.c"
  Delete "$INSTDIR\Tremor\mdct.h"
  Delete "$INSTDIR\Tremor\mdct_lookup.h"
  Delete "$INSTDIR\Tremor\misc.h"
  Delete "$INSTDIR\Tremor\ogg.h"
  Delete "$INSTDIR\Tremor\os.h"
  Delete "$INSTDIR\Tremor\os_types.h"
  Delete "$INSTDIR\Tremor\registry.c"
  Delete "$INSTDIR\Tremor\registry.h"
  Delete "$INSTDIR\Tremor\res012.c"
  Delete "$INSTDIR\Tremor\sharedbook.c"
  Delete "$INSTDIR\Tremor\synthesis.c"
  Delete "$INSTDIR\Tremor\vorbisfile.c"
  Delete "$INSTDIR\Tremor\window.c"
  Delete "$INSTDIR\Tremor\window.h"
  Delete "$INSTDIR\Tremor\window_lookup.h"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"
  RMDir "$INSTDIR\Tremor"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Tetris.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Readme.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\NitroGLHDRTetris"
SectionEnd