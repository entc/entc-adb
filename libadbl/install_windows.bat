set bconfig=Release
set version=1.3.0
set target=%PROGRAMFILES%\quom\adbl\%version%

rem install into windows folder
copy core\%bconfig%\adbl.dll  C:\Windows\System32\adbl.dll

mkdir C:\Windows\System32\adbl-%version%\
copy adbl_sqlite\%bconfig%\adbl_sqlite3.dll C:\Windows\System32\adbl-%version%\

mkdir "%target%"
mkdir "%target%\include"

copy core\*.h   "%target%\include\"

mkdir "%target%\lib"

copy core\%bconfig%\adbl.lib                  "%target%\lib\adbl.lib"
copy core\%bconfig%\adbl.exp                  "%target%\lib\adbl.exp"
