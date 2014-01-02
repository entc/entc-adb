set target=C:\"Program Files"\DevCommon\
set bconfig=Debug
set version=1.0.1

rem install into windows folder
copy %bconfig%\entc.dll  C:\Windows\System32\entc.dll

mkdir %target%
mkdir %target%include

mkdir %target%include\entc-%version%
mkdir %target%include\entc-%version%\system
mkdir %target%include\entc-%version%\tools
mkdir %target%include\entc-%version%\types
mkdir %target%include\entc-%version%\utils

copy system\*.h   %target%include\entc-%version%\system
copy tools\*.h    %target%include\entc-%version%\tools
copy types\*.h    %target%include\entc-%version%\types
copy utils\*.h    %target%include\entc-%version%\utils

mkdir %target%lib

copy %bconfig%\entc.lib                  %target%lib\entc.lib
copy %bconfig%\entc.exp                  %target%lib\entc.exp
