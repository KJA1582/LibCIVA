Remove-Item -r -fo .\dist

mkdir .\dist
mkdir .\dist\lib
mkdir .\dist\lib\debug
mkdir .\dist\lib\release

Set-Location .\out

# x64 Debug static
Remove-Item -r -fo .\x64
mkdir .\x64
Set-Location .\x64

cmake -DCMAKE_BUILD_TYPE="Debug" ..\..
msbuild .\libciva.vcxproj /property:Configuration="Debug" /m

Copy-Item .\Debug\libciva.idb ..\..\dist\lib\debug
Copy-Item .\Debug\libciva.lib ..\..\dist\lib\debug
Copy-Item .\Debug\libciva.pdb ..\..\dist\lib\debug

Set-Location ..\

# x64 Release static
Remove-Item -r -fo .\x64
mkdir .\x64
Set-Location .\x64

cmake -DCMAKE_BUILD_TYPE="Release" ..\..
msbuild .\libciva.vcxproj /property:Configuration="Release" /m

Copy-Item .\Release\libciva.lib ..\..\dist\lib\release

Set-Location ..\..\

# MSFS Debug
Remove-Item -r -fo .\out\MSFS

msbuild .\msfs.vcxproj /property:Platform=MSFS /property:Configuration="Debug" /m

Copy-Item .\out\MSFS\Debug\libciva.a .\dist\lib\debug

# MSFS Release
Remove-Item -r -fo .\out\MSFS

msbuild .\msfs.vcxproj /property:Platform=MSFS /property:Configuration="Release" /m

Copy-Item .\out\MSFS\Release\libciva.a .\dist\lib\release

# Include

Copy-Item .\include .\dist\include -Recurse