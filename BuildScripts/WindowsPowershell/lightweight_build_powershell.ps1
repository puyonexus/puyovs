echo "Puyo VS Build Script"
echo ""
echo "DO NOT USE THIS AS ARM64 BUILD: THE BUILD WILL CRASH!!!!!"
echo ""
echo ""

## Tried to implement 7z auto-install, but I didn't.
## If you want to implement that, you can.
echo "Before start, you SHOULD install these programs manually."
echo "If installed, press ENTER to continue."
echo ""
echo "Programs:"
echo "  - Git"
echo "  - 7z"
echo "  - CMake"
echo "  - Also, you should have setted up VC++ build environment."
echo "  If not, please use the common one."
echo ""
echo "Also, please run this script with Administrator privillige."
## other programs are handlable through ps1 script
pause
echo ""

## [IntPtr]::Size is 4 on x86 and 8 on x64/ARM64
if ( [IntPtr]::Size -eq 4 ) {
    $SYSARCH = 'Win32'
    $MATARCH = 'x86'
}
if ( [IntPtr]::Size -eq 8 ) {
    $SYSARCH = 'x64'
    $MATARCH = 'x64'
}

$GITHUB_PATH = "$env:USERPROFILE\GitHub\PuyoVS"
del "$GITHUB_PATH" -Recurse -Force

### Phase 1. Install winget if not installed
## This phase is skipped since it's lightweight:
## the lightweight build script is used if you have installed Visual C++ build environment.

### Phase 2. Download Qt SDK
clear
echo "Now this will download Qt SDK, required to build Puyo VS."
## Download from GitHub...
Invoke-WebRequest -Uri "https://github.com/puyonexus/qt-sdk-builder/releases/download/v2/qt-windows-5.15.2-$MATARCH-msvc-static-release.7z" -OutFile "C:\qt.7z"

### Phase 3. Extract Qt SDK
echo "Extracting..."
echo "If error occurs, please install 7z and run the script from the beginning."
## Use 7z to extract qt.7z
7z x C:\qt.7z -oC:\Qt\
## ..and add to path
#### Quote: https://techexpert.tips/powershell/powershell-edit-path-variable/
$TEMPQT = "C:\Qt\Qt-5.15.2\bin"
$OLDPATH = [System.Environment]::GetEnvironmentVariable('PATH','machine')
$NEWPATH = "$OLDPATH;$TEMP_CMAKE;$TEMPQT"
[Environment]::SetEnvironmentVariable('PATH', "$NEWPATH", 'MACHINE')
#### Cleanup
$TEMPQT = ""
$OLDPATH = ""
$NEWPATH = ""

### Phase 4. Git Clone
clear
echo "Now this will clone Puyo VS repository."
echo "Ignore the git error."
git clone --recursive "https://github.com/puyonexus/puyovs.git" "$GITHUB_PATH"
cd "$GITHUB_PATH"

### Phase 5. Make CMake Build Directory
clear
echo "Now this will configure PuyoVS via CMake."
## Init CMake
cmake -B .build
## Build PuyoVS
cmake --build .build --parallel 8 --config RelWithDebInfo
## Install Assets
cmake --install .build --config RelWithDebInfo
pause

### Phase 6. Finalizing
cls
echo "Build finished. Cleaning up..."
$GITHUB_PATH\.build\Client\RelWithDebInfo\PuyoVS.exe
del C:\qt -Recurse -Force
del C:\qt.7z
pause
