# Microsoft Developer Studio Project File - Name="morphnt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=morphnt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "morphnt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "morphnt.mak" CFG="morphnt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "morphnt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "morphnt - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/c/morphnt", SAJBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "morphnt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "morphnt_"
# PROP BASE Intermediate_Dir "morphnt_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Winrel"
# PROP Intermediate_Dir "Winrel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /GX /O2 /I "aucntrl" /I "morphdll" /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "aucntrl" /i "morphdll" /i "include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib ctl3d32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "morphnt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "morphnt0"
# PROP BASE Intermediate_Dir "morphnt0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Windebug"
# PROP Intermediate_Dir "Windebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /Gm /GX /ZI /Od /I "aucntrl" /I "morphdll" /I "include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "aucntrl" /i "morphdll" /i "include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib ctl3d32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "morphnt - Win32 Release"
# Name "morphnt - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "c;rc"
# Begin Source File

SOURCE=.\BMPCTRL.C
# End Source File
# Begin Source File

SOURCE=.\DLGUTILS.C
# End Source File
# Begin Source File

SOURCE=.\FILEHPS.C
# End Source File
# Begin Source File

SOURCE=.\FILEINT.C
# End Source File
# Begin Source File

SOURCE=.\FILEMM.C
# End Source File
# Begin Source File

SOURCE=.\FILEPM.C
# End Source File
# Begin Source File

SOURCE=.\FILEPRS.C
# End Source File
# Begin Source File

SOURCE=.\FILES.C
# End Source File
# Begin Source File

SOURCE=.\FILETUNE.C
# End Source File
# Begin Source File

SOURCE=.\FUNCGEN.C
# End Source File
# Begin Source File

SOURCE=.\HYPER1.C
# End Source File
# Begin Source File

SOURCE=.\HYPERS.C
# End Source File
# Begin Source File

SOURCE=.\MAIN.C
# End Source File
# Begin Source File

SOURCE=.\MASTER.C
# End Source File
# Begin Source File

SOURCE=.\MDATA.C
# End Source File
# Begin Source File

SOURCE=.\MIDI.C
# End Source File
# Begin Source File

SOURCE=.\MIDIFILE.C
# End Source File
# Begin Source File

SOURCE=.\MIDIMAP1.C
# End Source File
# Begin Source File

SOURCE=.\MIDIMAPS.C
# End Source File
# Begin Source File

SOURCE=.\MIDIRX.C
# End Source File
# Begin Source File

SOURCE=.\MIDITX.C
# End Source File
# Begin Source File

SOURCE=.\MORPHEUS.RC
# End Source File
# Begin Source File

SOURCE=.\MORPHMBX.C
# End Source File
# Begin Source File

SOURCE=.\OPTIONS.C
# End Source File
# Begin Source File

SOURCE=.\PATCHBNO.C
# End Source File
# Begin Source File

SOURCE=.\PATCHBRT.C
# End Source File
# Begin Source File

SOURCE=.\PIANO.C
# End Source File
# Begin Source File

SOURCE=.\PRESET2.C
# End Source File
# Begin Source File

SOURCE=.\PRESET3.C
# End Source File
# Begin Source File

SOURCE=.\PRESET4.C
# End Source File
# Begin Source File

SOURCE=.\PRESETS.C
# End Source File
# Begin Source File

SOURCE=.\PRINIT.C
# End Source File
# Begin Source File

SOURCE=.\PROGMAP.C
# End Source File
# Begin Source File

SOURCE=.\SYSEX.C
# End Source File
# Begin Source File

SOURCE=.\TUNING.C
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\MIDIDEFS.H
# End Source File
# Begin Source File

SOURCE=.\MORFILE.H
# End Source File
# Begin Source File

SOURCE=.\MORPHEUS.H
# End Source File
# Begin Source File

SOURCE=.\RESOURCE.H
# End Source File
# Begin Source File

SOURCE=.\VERSION.H
# End Source File
# Begin Source File

SOURCE=.\WDLG_I.H
# End Source File
# End Group
# Begin Source File

SOURCE=.\APP.ICO
# End Source File
# Begin Source File

SOURCE=.\APP2.ICO
# End Source File
# Begin Source File

SOURCE=.\EMULOGO.BMP
# End Source File
# Begin Source File

SOURCE=.\HYPER.ICO
# End Source File
# Begin Source File

SOURCE=.\LIST.ICO
# End Source File
# Begin Source File

SOURCE=.\MASTER.ICO
# End Source File
# Begin Source File

SOURCE=.\MIDIMAP.ICO
# End Source File
# Begin Source File

SOURCE=.\NOPATCH.ICO
# End Source File
# Begin Source File

SOURCE=.\PIANO.ICO
# End Source File
# Begin Source File

SOURCE=.\PRESET.ICO
# End Source File
# Begin Source File

SOURCE=.\PROGMAN.ICO
# End Source File
# Begin Source File

SOURCE=.\RTPATCH.ICO
# End Source File
# Begin Source File

SOURCE=.\TUNING.ICO
# End Source File
# End Target
# End Project
