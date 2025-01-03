set ExternalDir=%1
set TargetDir=%2

xcopy /y /d /i "%ExternalDir%FFTW\libfftw3f-3.dll" "%TargetDir%"
