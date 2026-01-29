# Build lib
cd ..\..
cd .\libciva
mkdir .\out
cd .\out
rm -r -fo .\Win32
mkdir .\Win32
cd .\Win32
cmake -A Win32 ..\..
msbuild .\libciva.sln
cd ..\..\..

# Build example
cd .\Examples\civaWin
rm -r -fo .\out
mkdir .\out
cd .\out
cmake -A Win32 ..
msbuild .\civaWin.sln
cd ..
