set target=C:\"Program Files"\DevCommon\
set bconfig=Debug
set version=1.0.1

rem install into windows folder
copy core\%bconfig%\adbo.dll  C:\Windows\System32\adbo.dll

mkdir %target%
mkdir %target%include

mkdir %target%include\adbo-%version%

copy core\*.h   %target%include\adbo-%version%\

mkdir %target%lib

copy core\%bconfig%\adbo.lib                  %target%lib\adbo.lib
copy core\%bconfig%\adbo.exp                  %target%lib\adbo.exp
