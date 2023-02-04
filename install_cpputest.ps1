param(
    [switch] $clone = $false
)

if ($clone) {
    git clone https://github.com/cpputest/cpputest.git
}
if (-not (Test-Path -Path "$PSScriptRoot/cpputest")) {
    Write-Host "Error: folder $PSScriptRoot\cpputest does not exist."
    Exit
}
cd $PSScriptRoot/cpputest/build
cmake ..
make
make install
cd ../src/CppUTest
cp -r C:\cygwin64\usr\local\lib\* .
cd $PSScriptRoot
