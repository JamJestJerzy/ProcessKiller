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
> - Make sure you have c compiler (`g++`) installed
> - Run command ```.\build.ps1``` in Windows PowerShell
> - Compiled binary should be in ```builds``` dir with name `ProcessKiller-{current version}.exe`
## License
© All Rights Reserved