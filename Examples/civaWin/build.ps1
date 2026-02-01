param(
  [ValidateSet('Debug', 'Release')]
  [string]
  $Build = "Debug",

  [switch]
  $Clean
)

Write-Host "Building for $Build"
Write-Host "Cleaning: $Clean"

Write-Warning "Check settings" -WarningAction Inquire

# Lib
Set-Location ..\..\libciva
mkdir .\out
Set-Location .\out
# Clean
if ($Clean) {
  Remove-Item -r -fo .\Win32
}
# Build
mkdir .\Win32
Set-Location .\Win32
cmake -A Win32 -DCMAKE_BUILD_TYPE="$Build" ..\..
msbuild .\libciva.vcxproj /property:Configuration="$Build" /m

# Example
Set-Location ..\..\..\Examples\civaWin

# Terminate here if lib build failed
if ($LASTEXITCODE -gt 0) {
  exit
}

# Clean
if ($Clean) {
  Remove-Item -r -fo .\out
}
# Build
mkdir .\out
Set-Location .\out
cmake -A Win32 -DCMAKE_BUILD_TYPE="$Build" ..
msbuild .\civaWin.vcxproj /property:Configuration="$Build" /m
Set-Location ..\
