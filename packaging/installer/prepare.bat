if exist "Qt\" rd /q /s Qt
windeployqt --qmldir ..\..\src\ui\qml --dir Qt ricochet.exe
if exist "Qt\qmltooling" rd /q /s Qt\qmltooling
copy /Y ..\..\icons\ricochet.ico ricochet.ico
copy /Y ..\..\LICENSE LICENSE
if not exist "translation\" mkdir translation
copy /Y ..\..\translation\installer_*.isl translation\
xcopy /Y ..\..\translation\inno translation\inno\
