if exist "Qt\" rd /q /s Qt
windeployqt --qmldir ..\..\src\ui\qml --dir Qt ricochet.exe
if exist "Qt\qmltooling" rd /q /s Qt\qmltooling
