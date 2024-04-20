# ffmpeg-opengl-effect
### ffmpeg effect with opengl,支持ffmpeg7和opengl3.3 core模式的视频滤镜特效,演示抖音灵魂出窍特效
##### 编译说明
- 编译环境:msys2\ffmpeg7,环境准备build-env.sh脚本,编译参数参考build-ffmpeg.sh脚本
- 复制vf_vernus.c到libavfilter目录下
- 修改libavfilter目录下Makefile,在  OBJS-$(CONFIG_ZSCALE_FILTER)                 += vf_zscale.o  下面一行添加 OBJS-$(CONFIG_VERNUS_FILTER)                 += vf_vernus.o
- 修改libavfilter目录下allfilters.c文件,在extern const AVFilter ff_vf_zscale;下面一行添加extern const AVFilter ff_vf_vernus;

##### 使用说明
- 使用示例:.\ffplay.exe .\test\test.mp4 -vf "vernus=name=soul:start=2:duration=999"
###### 参数说明
- 过滤器名称: vernus
- name: 效果名称,如soul,效果是抖音的灵魂出窍,effect目录下soul.vert\soul.frag文件,分别对应顶点着色器\片元着色器代码,具体编写方法参考OpenGL说明文档,目前支持两个变量输入iPlayTime(播放时间),iTexture0(当前帧图片数据,RGBA格式)
- start: 效果开始时间,相对于开始播放,单位秒
- duration: 效果持续时间,单位秒
