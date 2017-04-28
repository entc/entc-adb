set bconfig=Release
set version=1.0.5
set target=%PROGRAMFILES%\quom\adbo\%version%

rem install into windows folder
copy core\%bconfig%\adbo.dll  C:\Windows\System32\adbo.dll

mkdir "%target%"
mkdir "%target%\include"

copy core\*.h   "%target%\include\"

mkdir "%target%\lib"

copy core\%bconfig%\adbo.lib                  "%target%\lib\adbo.lib"
copy core\%bconfig%\adbo.exp                  "%target%\lib\adbo.exp"
