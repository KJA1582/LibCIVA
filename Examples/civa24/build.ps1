param(
  [ValidateSet('Debug', 'Release')]
  [string]
  $Build = "Debug",

  [switch]
  $Clean,

  [switch]
  $StackAnalysis
)

Write-Host "Building for $Build"
Write-Host "Cleaning: $Clean"
Write-Host "Stack Analysis: $StackAnalysis"

Write-Warning "Check settings" -WarningAction Inquire

# Lib
Set-Location ..\..\libciva
mkdir .\out
# Clean
if ($Clean) {
  Set-Location .\out
  Remove-Item -r -fo .\MSFS
  Remove-Item -r -fo .\MSFS2024
  Set-Location ..\
}
# Build
msbuild .\msfs.vcxproj /property:Configuration="$Build" /property:Platform=MSFS /m
msbuild .\msfs2024.vcxproj /property:Configuration="$Build" /property:Platform=MSFS /m

# Example
Set-Location ..\Examples\civa24
# Clean
if ($Clean) {
  Remove-Item -r -fo .\out
}
# Build
msbuild .\civa24.sln /property:Configuration="$Build" /property:Platform=MSFS /m

# Stack analysis
if ($StackAnalysis) {
  Set-Location ..\..\
  node .\stack-analysis.js
  Set-Location .\Examples\civa24
}