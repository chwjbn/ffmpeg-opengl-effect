basepath=$(cd `dirname $0`;pwd)
echo ${basepath}

pacman -S gcc make mingw-w64-i686-gcc--noconfirm
pacman -S gcc make mingw-w64-x86_64-gcc  --noconfirm
pacman -S mingw64/mingw-w64-x86_64-cmake  --noconfirm

pacman -S mingw-w64-x86_64-toolchain --noconfirm
pacman -S mingw-w64-i686-toolchain --noconfirm
pacman -S base-devel --noconfirm
pacman -S yasm nasm  gcc --noconfirm
pacman -S mingw64/mingw-w64-x86_64-SDL2 --noconfirm

pacman -S mingw32/mingw-w64-i686-glfw  --noconfirm
pacman -S mingw64/mingw-w64-x86_64-glfw --noconfirm
pacman -S mingw64/mingw-w64-x86_64-glfw-docs --noconfirm
pacman -S mingw-w64-x86_64-glew mingw32/mingw-w64-i686-glew  --noconfirm
pacman -S mingw64/mingw-w64-x86_64-glm  --noconfirm

pacman -S mingw64/mingw-w64-x86_64-libx264  --noconfirm
pacman -S mingw64/mingw-w64-x86_64-x264 --noconfirm
pacman -S mingw64/mingw-w64-x86_64-x265 --noconfirm

pacman -S mingw64/mingw-w64-x86_64-fdk-aac --noconfirm
pacman -S mingw64/mingw-w64-x86_64-libclc --noconfirm

pacman -S mingw64/mingw-w64-x86_64-libmfx --noconfirm
pacman -S mingw64/mingw-w64-x86_64-ffnvcodec-headers --noconfirm
pacman -S mingw64/mingw-w64-x86_64-fontconfig --noconfirm


pacman -S pkg-config --noconfirm

pacman -S autoconf --noconfirm
pacman -S automake --noconfirm


