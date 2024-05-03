#!/bin/sh
#进入执行脚本所在目录
basepath=$(cd `dirname $0`;pwd)
echo ${basepath}

#进入ffmpeg源码目录
cd ${basepath}/src  # 根据路径名称自行修改
pwd


#指定PKG_CONFIG_PATH变量，告知编译器x264库的路径
echo ${PKG_CONFIG_PATH}


make clean

#配置编译参数
./configure \
  --prefix=${basepath}/out \
  --arch=x86_64 \
  --enable-cross-compile \
  --pkg-config-flags=--static \
  --enable-gpl \
  --enable-libfdk_aac \
  --enable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvpx \
  --enable-encoder=libvpx_vp8 --enable-encoder=libvpx_vp9 --enable-decoder=vp8 --enable-decoder=vp9 --enable-parser=vp8 --enable-parser=vp9 \
  --enable-libx264 \
  --enable-libx265 \
  --enable-decoder=h264 \
  --enable-decoder=hevc \
  --enable-libass \
  --enable-libfreetype \
  --enable-fontconfig \
  --enable-libfribidi \
  --enable-libwebp \
  --enable-demuxer=dash \
  --enable-libxml2 \
  --enable-nonfree \
  --enable-opengl \
  --enable-ffnvcodec --enable-postproc --enable-nvdec --enable-nvenc --enable-cuda --enable-cuvid --enable-encoder=h264_nvenc --enable-decoder=h264_cuvid \
  --enable-libvpl --enable-encoder=h264_qsv --enable-decoder=h264_qsv \
  --disable-doc  \
  --disable-htmlpages  \
  --disable-manpages  \
  --disable-podpages  \
  --disable-txtpages \
  --extra-cflags="-static" \
  --extra-ldflags="-Bstatic -lm -lz -llzma -lpthread" \
  --extra-ldflags='-lopengl32 -lglew32 -lglfw3 -DGLEW_STATIC' \
  --extra-libs=-lpthread \
  --extra-libs=-lm \
  --extra-libs=-lopengl32 \
  --extra-libs=-lglew32 \
  --extra-libs=-lglfw3 \
  --enable-filter=vernus
 
#开始8线程编译
make -j8
#将编译后的文件拷贝到--prefix参数配置目录
make install