# Function to extract version from main.cpp
function Get-Version
{
    $filePath = "src/main.cpp"
    $content = Get-Content -Path $filePath -Raw

    # Use regex to extract version
    $versionPattern = 'std::string VERSION = "(.+?)";'
    $versionMatch = [regex]::Match($content, $versionPattern)

    if ($versionMatch.Success)
    {
        return $versionMatch.Groups[1].Value
    }
    else
    {
        Write-Host "Error: Version not found in main.cpp. Please check the file."
        exit 1
    }
}

# Function to build the C++ program
function Build-Program
{
    $outputFolder = "builds"
    if (-not(Test-Path $outputFolder))
    {
        New-Item -ItemType Directory -Path $outputFolder | Out-Null
    }

    # Get all .cpp files in the src/ directory
    $cppFiles = Get-ChildItem -Path "src\" -Filter "*.cpp" | ForEach-Object { $_.FullName }

    $cppFileList = $cppFiles -join " "
    $version = Get-Version
    $outputFileName = Join-Path $outputFolder "ProcessKiller-$version.exe"
    $compileCommand = "g++ -o $outputFileName $cppFileList -I.\libcurl\include -L. -lcurl -static-libgcc -static-libstdc++ -static -lpthread -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lwinmm -lmingw32 -lmingwex -lmsvcrt -lmsvcr100 -lversion -lstdc++fs -lws2_32 -lwinhttp -Wcpp -w"

    Invoke-Expression $compileCommand

    if ($LASTEXITCODE -eq 0)
    {
        Write-Host "Build successful. Output file: $outputFileName"
    }
    else
    {
        Write-Host "Build failed. Please check the compilation command and errors."
        exit 1
    }
}

# Build the C++ program
Build-Program
