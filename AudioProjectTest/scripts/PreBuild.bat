:: Properties > Build Events > ...
:: call "$(ProjectDir)scripts\PreBuild.bat" "$(ProjectDir)external\" "$(TargetDir)"

set ExternalDir=%1
set TargetDir=%2

xcopy /y /d /i "%ExternalDir%FFTW\libfftw3-3.dll" "%TargetDir%"
xcopy /y /d /i "%ExternalDir%LibSndFile\bin\sndfile.dll" "%TargetDir%"
