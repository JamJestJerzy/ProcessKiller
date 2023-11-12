param (
    [string]$text
)

# Function to extract version from main.cpp
function Get-Version {
    $filePath = "src/main.cpp"  # Replace with the actual path to your main.cpp file
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

# Check if a parameter is provided as the commit message
if (-not $text) {
    Write-Host "Error: Please provide a commit message."
    exit 1
}

# Get the version from main.cpp
$version = Get-Version

# Execute Git commands
git add --all
git commit -m ("[{0}] {1}" -f $version, $text)
git push origin main