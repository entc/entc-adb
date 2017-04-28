set bconfig=Release
set version=1.2.0
set target=%PROGRAMFILES%\quom\entc\%version%

rem install into windows folder
copy %bconfig%\entc.dll  C:\Windows\System32\entc.dll

mkdir "%target%"

mkdir "%target%\include\"
mkdir "%target%\include\system"
mkdir "%target%\include\tools"
mkdir "%target%\include\types"
mkdir "%target%\include\utils"

copy system\*.h   "%target%\include\system\"
copy tools\*.h    "%target%\include\tools\"
copy types\*.h    "%target%\include\types\"
copy utils\*.h    "%target%\include\utils\"

mkdir "%target%\lib"

copy %bconfig%\entc.lib                  "%target%\lib\entc.lib"
copy %bconfig%\entc.exp                  "%target%\lib\entc.exp"
