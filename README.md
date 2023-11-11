# ProcessKiller
Kills configured proccesses right after they started
# Usage
Run `.exe` file as every other program. On first run it'll generate config file.<br>
Write to this file filenames of processes you want to kill. 1 filename for line<br>
> Example:
>
> ```properties
> # Comment or smth
> Never.exe
> Gonna.exe
> Give.exe
> You.exe
> Up.exe
> Never.exe
> Gonna.exe
> Let.exe
> You.exe
> Down.exe
> ```
# Compilation
> - Clone this repository (```git clone https://github.com/JamJestJerzy/ProcessKiller.git```)<br>
> - `cd` into cloned folder
> - Run command<br> ```g++ -o ProcessKiller .\main.cpp -static-libgcc -static-libstdc++ -static -lpthread -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lwinmm -lmingw32 -lmingwex -lmsvcrt -lmsvcr100 -lversion -lstdc++fs```
> - Compiled binary should be in current dir with name `ProcessKiller.exe`
## License
© All Rights Reserved