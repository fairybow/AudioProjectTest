set ExternalDir=%1
set TargetDir=%2

xcopy /y /d /i "%ExternalDir%FFTW-win\libfftw3f-3.dll" "%TargetDir%"
