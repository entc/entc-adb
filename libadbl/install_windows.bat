set target=C:\"Program Files"\DevCommon\
set bconfig=Debug
set version=1.0.1

rem install into windows folder
copy core\%bconfig%\adbl.dll  C:\Windows\System32\adbl.dll

mkdir C:\Windows\System32\adbl-%version%\
copy adbl_sqlite\%bconfig%\adbl_sqlite3.dll C:\Windows\System32\adbl-%version%\

mkdir %target%
mkdir %target%include

mkdir %target%include\adbl-%version%

copy core\*.h   %target%include\adbl-%version%\

mkdir %target%lib

copy core\%bconfig%\adbl.lib                  %target%lib\adbl.lib
copy core\%bconfig%\adbl.exp                  %target%lib\adbl.exp
