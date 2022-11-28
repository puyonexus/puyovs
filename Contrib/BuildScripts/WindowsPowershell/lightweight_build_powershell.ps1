# Windows PowerShell script to build PuyoVS from source.
# Copyright (C) 2022 Dohyeon Jeon
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
echo "  - 7z"
echo "  - cmake"
echo "  - CMake Build Environment"
echo ""
echo "Also, please run this script with Administrator privillige."
## other programs are handlable through ps1 script
pause
echo ""

if ( [IntPtr]::Size -eq 4 ) {
    $SYSARCH = 'Win32'
    $MATARCH = 'x86'
}
if ( [IntPtr]::Size -eq 8 ) {
    $SYSARCH = 'x64'
    $MATARCH = 'x64'
}

$GitHubPath = ($MyInvocation.MyCommand).Path
for ($i = 0; $i -le 2; $i++) {
	$temp = Split-Path $GitHubPath
    $GitHubPath = $temp
}

$QtPath = "C:\"

### Phase 1. Skipped. Installing Visual Studio / BuildTools is skipped on Lightweight script.

### Phase 2. Download Qt SDK
clear
echo "Now this will download Qt SDK, required to build Puyo VS."
## Download from GitHub...
Invoke-WebRequest -Uri "https://github.com/puyonexus/qt-sdk-builder/releases/download/v2/qt-windows-5.15.2-$MATARCH-msvc-static-release.7z" -OutFile "$QtPath\qt.7z"

### Phase 3. Extract Qt SDK
echo "Extracting..."
echo "If error occurs, please install 7z and run the script from the beginning."
## Use 7z to extract qt.7z
7z x "$QtPath\qt.7z" -o"$QtPath\Qt\"
## ..and add to path
#### Quote: https://techexpert.tips/powershell/powershell-edit-path-variable/
$TEMPQT = "$QtPath\Qt\Qt-5.15.2\bin"
$OLDPATH = [System.Environment]::GetEnvironmentVariable('PATH','machine')
$NEWPATH = "$OLDPATH;$TEMPQT"
[Environment]::SetEnvironmentVariable('PATH', "$NEWPATH", 'MACHINE')
#### Cleanup
$TEMPQT = ""
$OLDPATH = ""
$NEWPATH = ""

### Phase 4. Make CMake Build Directory
clear
echo "Now this will configure PuyoVS via CMake."
## Relate 
cd $GitHubPath
mkdir .\.build
## Init CMake
cmake -B .build
## Build PuyoVS
cmake --build .build --parallel 8 --config RelWithDebInfo
## Install Assets
cmake --install .build --config RelWithDebInfo
echo "Build finished. Please check file at '$GitHubPath\.build\Client\RelWithDebInfo\PuyoVS.exe'."
pause

### Phase 6. Finalizing
cls
echo "Build finished. Cleaning up..."
del "$QtPath\qt" -Recurse -Force
del "$QtPath\qt.7z"
pause
