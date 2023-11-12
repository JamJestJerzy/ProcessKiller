# Function to extract version from main.cpp
function Get-Version {
    $filePath = "main.cpp"  # Replace with the actual path to your main.cpp file
    $content = Get-Content -Path $filePath -Raw

    # Use regex to extract version
    $versionPattern = 'std::string VERSION = "(.+?)";'
    $versionMatch = [regex]::Match($content, $versionPattern)

    if ($versionMatch.Success) {
        return $versionMatch.Groups[1].Value
    } else {
        Write-Host "Error: Version not found in main.cpp. Please check the file."
        exit 1
    }
}

# Function to build the C++ program
function Build-Program {
    param (
        [string]$version
    )

    $outputFileName = ".\ProcessKiller-$version.exe"
    $compileCommand = "g++ -o $outputFileName main.cpp -I.\libcurl\include -L. -lcurl -static-libgcc -static-libstdc++ -static -lpthread -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lwinmm -lmingw32 -lmingwex -lmsvcrt -lmsvcr100 -lversion -lstdc++fs -lws2_32 -lwinhttp"

    Invoke-Expression $compileCommand

    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build successful. Output file: $outputFileName"
    } else {
        Write-Host "Build failed. Please check the compilation command and errors."
        exit 1
    }
}

# Get the version from main.cpp
$version = Get-Version

# Build the C++ program
Build-Program -version $version