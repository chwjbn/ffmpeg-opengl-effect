
#include <stdio.h>

#include <io.h>
#include <process.h>

#include <math.h>

#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "video.h"

#include "libswscale/swscale.h"

#define GLEW_STATIC 

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif



static int str_last_index_of(const char* str, char c) 
{
	int lastIndex = -1;
	int i;

	for (i = 0; str[i] != '\0'; i++) 
	{
		if (str[i] == c) {
			lastIndex = i;
		}
	}

	return lastIndex;
}

static int str_sub_str(const char* str, char* sub_str, int from_index, int to_index)
{
	int nRet = -1;

	int i,j;

	j = 0;

	for (i = 0; str[i] != '\0'; i++) 
	{
		if (i>=from_index&&i<=to_index)
		{
			sub_str[j] = str[i];
			j++;
		}
	}

	nRet = 0;


	return nRet;
}

static int app_base_dir(char* path)
{
	int nRet = -1;


	char* appDirPath[_MAX_PATH] = { 0 };

	char* appFilePath[_MAX_PATH] = { 0 };
	sprintf(appFilePath, "%s", _pgmptr);

	int lastIndex = str_last_index_of(appFilePath, PATH_SEPARATOR);

	if (lastIndex < 0)
	{
		if (getcwd(appDirPath, sizeof(appDirPath)))
		{
			strcpy(appDirPath, "./");
		}

		strcpy(path, appDirPath);

		nRet = 0;

		return nRet;
	}

	nRet = str_sub_str(appFilePath, appDirPath, 0, lastIndex);

	if (nRet>=0)
	{
		strcpy(path, appDirPath);

		nRet = 0;
	}


	return nRet;
}


typedef struct {
	const AVClass* class;

	char* mMixName;
	int64_t         mRenderStart;
	int64_t         mRenderDuration;


	GLuint          mVetexShader;
	GLuint          mFragmentshader;
	GLuint          mProgram;


	GLFWwindow* mWindow;
	GLuint          mVAO;
	GLuint          mVBO;
	GLuint          mEBO;
	GLuint          mFrameTexture;

	GLuint         mCodeVarPlayTime;
	GLuint         mCodeVarTexture0;

} VernusContext;


void glfwOnError(int error, const char* description);


#define OFFSET(x) offsetof(VernusContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM

static const AVOption vernus_options[] = {
	{"name",  "required, effect name.",
						OFFSET(mMixName),
						AV_OPT_TYPE_STRING,
						{.str = NULL},
						CHAR_MIN,
						CHAR_MAX,
						FLAGS},
	{"start", "optional, effect start time in seconds.",
			  OFFSET(mRenderStart),
			  AV_OPT_TYPE_INT64,
			  {.i64 = 0.},
			  0,
			  INT64_MAX,
			  FLAGS},
	{"duration",  "optional, effect duration time in seconds.",
				  OFFSET(mRenderDuration),
				  AV_OPT_TYPE_INT64,
				  {.i64 = 0.},
				  0,
				  INT64_MAX,
				  FLAGS},
	{NULL}
};


AVFILTER_DEFINE_CLASS(vernus);


//顶点数据
static const float gVertices[] = {
	// top left
	-1.0, 1.0, 0.0, // position
	1.0, 0.0, 0.0, // Color
	0.0, 1.0, // texture coordinates

	// top right
	1.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	1.0, 1.0,

	// bottom right
	1.0, -1.0, 0.0,
	0.0, 0.0, 1.0,
	1.0, 0.0,

	// bottom left
	-1.0, -1.0, 0.0,
	1.0, 1.0, 1.0,
	0.0, 0.0,
};


//索引数据
static const unsigned int gIndices[] = {
	// rectangle
	0, 1, 2, // top triangle
	0, 2, 3, // bottom triangle
};

static int initGLData(AVFilterContext* ctx)
{
	int nRet = -1;

	VernusContext* vernusCtx = ctx->priv;

	glGenVertexArrays(1, &vernusCtx->mVAO);


	//绑定顶点开始操作
	glBindVertexArray(vernusCtx->mVAO);

	glGenBuffers(1, &vernusCtx->mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vernusCtx->mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gVertices), gVertices, GL_STATIC_DRAW);


	glGenBuffers(1, &vernusCtx->mEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vernusCtx->mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), gIndices, GL_STATIC_DRAW);

	//连接顶点数据
	int offset = 0;
	int stride = 3 * sizeof(float) + 3 * sizeof(float) + 2 * sizeof(float);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);

	glEnableVertexAttribArray(0);
	offset += 3 * sizeof(float);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(1);
	offset += 3 * sizeof(float);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(2);
	offset += 2 * sizeof(float);

	//解绑顶点
	glBindVertexArray(0);


	//创建纹理并且初始化
	glGenTextures(1, &vernusCtx->mFrameTexture);

	//绑定纹理到TEXTURE0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, vernusCtx->mFrameTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	nRet = 0;

	return nRet;

}

static int readShaderCode(AVFilterContext* ctx, const char* filePath, char* fileContent)
{
	int nRet = -1;

	FILE* fileHandle = fopen(filePath, "rb");
	if (!fileHandle)
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: readShaderCode=[%s] faild\n", filePath);
		nRet = -101;
		return nRet;
	}

	fseek(fileHandle, 0, SEEK_END);
	long fileSize = ftell(fileHandle);
	fseek(fileHandle, 0, SEEK_SET);


	av_log(ctx, AV_LOG_INFO, "vf_vernus: readShaderCode=[%s] fileSize=[%ld]\n", filePath, fileSize);

	if (fileSize > 0)
	{
		char* fileData = malloc(fileSize + 1);
		memset(fileData, 0, fileSize + 1);
		size_t readSize = fread(fileData, sizeof(char), fileSize, fileHandle);

		av_log(ctx, AV_LOG_INFO, "vf_vernus: readShaderCode=[%s] readSize=[%lld]\n", filePath, readSize);


		nRet = -102;
		if (readSize > 0)
		{
			memcpy(fileContent, fileData, fileSize);
			nRet = 0;
		}

		free(fileData);
	}

	fclose(fileHandle);

	nRet = 0;

	return nRet;
}

static GLuint buildShader(AVFilterContext* ctx, const GLchar* shaderCode, GLenum type)
{
	GLuint ret = 0;

	GLuint shader = glCreateShader(type);

	if (!shader || !glIsShader(shader))
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildShader glCreateShader faild\n");
		return ret;
	}

	GLint status;

	glShaderSource(shader, 1, &shaderCode, 0);
	glCompileShader(shader);


	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE)
	{
		ret = shader;
		return ret;
	}

	int nInfoLogLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &nInfoLogLength);

	if (nInfoLogLength > 0)
	{
		char* sInfoMessage = (char*)malloc(nInfoLogLength);

		glGetShaderInfoLog(shader, nInfoLogLength, NULL, sInfoMessage);

		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildShader glCompileShader error:[%s]\n", sInfoMessage);

		free(sInfoMessage);

		return ret;
	}

	av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildShader glCompileShader faild\n");

	return ret;
}



static int buildProgram(AVFilterContext* ctx)
{
	int nRet = -1;

	VernusContext* vernusCtx = ctx->priv;

	if (!vernusCtx->mMixName)
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildProgram error:[mMixName is null]\n");
		nRet = -101;
		return nRet;
	}

	av_log(ctx, AV_LOG_INFO, "vf_vernus: buildProgram mMixName=[%s]\n", vernusCtx->mMixName);


	char workDir[_MAX_PATH] = { 0 };
	if (app_base_dir(workDir)<0)
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildProgram error:[app_base_dir faild]\n");
		nRet = -102;
		return nRet;
	}

	av_log(ctx, AV_LOG_INFO, "vf_vernus: buildProgram workDir=[%s]\n",workDir);

	char xPathSplit[2] = { 0 };
	xPathSplit[0] = PATH_SEPARATOR;


	char vertexCodeFilePath[1024] = { 0 };
	char fragmentCodeFilePath[1024] = { 0 };

	strcat(vertexCodeFilePath, workDir);
	strcat(fragmentCodeFilePath, workDir);

	strcat(vertexCodeFilePath, "effect");
	strcat(fragmentCodeFilePath, "effect");

	strcat(vertexCodeFilePath, xPathSplit);
	strcat(fragmentCodeFilePath, xPathSplit);

	strcat(vertexCodeFilePath, vernusCtx->mMixName);
	strcat(fragmentCodeFilePath, vernusCtx->mMixName);

	strcat(vertexCodeFilePath, ".vert");
	strcat(fragmentCodeFilePath, ".frag");


	av_log(ctx, AV_LOG_INFO, "vf_vernus: buildProgram vertex=[%s]\n", vertexCodeFilePath);
	av_log(ctx, AV_LOG_INFO, "vf_vernus: buildProgram fragment=[%s]\n", fragmentCodeFilePath);


	char* sVertexCode = malloc(65530);
	memset(sVertexCode, 0, 65530);

	nRet = readShaderCode(ctx, vertexCodeFilePath, sVertexCode);
	if (nRet < 0)
	{
		free(sVertexCode);
		return nRet;
	}

	vernusCtx->mVetexShader = buildShader(ctx, sVertexCode, GL_VERTEX_SHADER);

	free(sVertexCode);

	if (vernusCtx->mVetexShader <= 0)
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildShader vertex faild\n");

		nRet = -111;
		return nRet;
	}



	char* sFragmentCode = malloc(65530);
	memset(sFragmentCode, 0, 65530);

	nRet = readShaderCode(ctx, fragmentCodeFilePath, sFragmentCode);
	if (nRet < 0)
	{
		free(sFragmentCode);
		return nRet;
	}

	av_log(ctx, AV_LOG_INFO, "vf_vernus:buildProgram buildShader vertex success\n");

	vernusCtx->mFragmentshader = buildShader(ctx, sFragmentCode, GL_FRAGMENT_SHADER);

	free(sFragmentCode);

	if (vernusCtx->mFragmentshader <= 0)
	{
		av_log(ctx, AV_LOG_ERROR, "vf_vernus: buildShader fragment faild\n");

		nRet = -111;
		return nRet;
	}


	av_log(ctx, AV_LOG_INFO, "vf_vernus:buildProgram buildShader fragment success\n");


	vernusCtx->mProgram = glCreateProgram();

	glAttachShader(vernusCtx->mProgram, vernusCtx->mVetexShader);
	glAttachShader(vernusCtx->mProgram, vernusCtx->mFragmentshader);
	glLinkProgram(vernusCtx->mProgram);

	GLint status = 0;
	glGetProgramiv(vernusCtx->mProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		nRet = -112;
		return nRet;
	}

	glUseProgram(vernusCtx->mProgram);

	vernusCtx->mCodeVarPlayTime = glGetUniformLocation(vernusCtx->mProgram, "iPlayTime");
	vernusCtx->mCodeVarTexture0 = glGetUniformLocation(vernusCtx->mProgram, "iTexture0");

	glUniform1f(vernusCtx->mCodeVarPlayTime, 0.0f);


	nRet = 0;


	return nRet;
}


static int processFrameFormatChange(AVFilterLink* inlink, AVFrame* src, AVFrame* dst)
{
	int nRet = -1;

	struct SwsContext* swsCtx = sws_getContext(
		src->width,
		src->height,
		src->format,
		dst->width,
		dst->height,
		dst->format,
		SWS_BICUBIC,
		NULL, NULL, NULL);

	if (!swsCtx)
	{
		nRet = -101;
		return nRet;
	}

	int nRes = sws_scale(swsCtx, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
	if (nRes >= 0)
	{
		nRet = 0;
	}

	sws_freeContext(swsCtx);

	return nRet;
}


static int processFrameRGBA(AVFilterLink* inlink, AVFrame* src)
{
	int nRet = -1;


	AVFilterContext* filterCtx = inlink->dst;
	VernusContext* vernusCtx = filterCtx->priv;

	glViewport(0, 0, src->width, src->height);

	glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(vernusCtx->mProgram);


	//传递时间
	double playTime = glfwGetTime();
	glUniform1f(vernusCtx->mCodeVarPlayTime, playTime);


	//传递纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, vernusCtx->mFrameTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, src->width, src->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, src->data[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	glUniform1i(vernusCtx->mCodeVarTexture0, 0);

	//绘制顶点
	glBindVertexArray(vernusCtx->mVAO);


	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	glReadPixels(0, 0, src->width, src->height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)src->data[0]);

	glfwSwapBuffers(vernusCtx->mWindow);

	glBindVertexArray(0);


	nRet = 0;

	return nRet;
}

static int inputConfig(AVFilterLink* inlink)
{

	int nRet = -1;

	AVFilterContext* filterCtx = inlink->dst;
	VernusContext* vernusCtx = filterCtx->priv;

	av_log(filterCtx, AV_LOG_INFO, "vf_vernus:inputConfig begin\n");

	if (glfwInit() != GLFW_TRUE) {
		av_log(filterCtx, AV_LOG_ERROR, "vf_vernus:inputConfig glfw init faild\n");
		nRet = -101;
		return nRet;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DECORATED, 0);
	glfwWindowHint(GLFW_VISIBLE, 0);

	vernusCtx->mWindow = glfwCreateWindow(inlink->w, inlink->h, "VernusFilter", NULL, NULL);

	glfwMakeContextCurrent(vernusCtx->mWindow);

	if (glewInit() != GLEW_OK) {
		av_log(filterCtx, AV_LOG_ERROR, "vf_vernus:inputConfig glew init faild\n");
		nRet = -111;
		return nRet;
	}

	glViewport(0, 0, inlink->w, inlink->h);
	glClearColor(0, 0, 0, 1);

	glEnable(GL_FRAMEBUFFER_SRGB);  //gamma校正


	nRet = initGLData(filterCtx);
	if (nRet < 0)
	{
		av_log(filterCtx, AV_LOG_ERROR, "vf_vernus:inputConfig initData faild\n");
		return nRet;
	}
	av_log(filterCtx, AV_LOG_INFO, "vf_vernus:inputConfig initData success\n");


	nRet = buildProgram(filterCtx);
	if (nRet < 0)
	{
		av_log(filterCtx, AV_LOG_ERROR, "vf_vernus:inputConfig buildProgram faild\n");
		return nRet;
	}
	av_log(filterCtx, AV_LOG_INFO, "vf_vernus:inputConfig buildProgram success\n");

	nRet = 0;

	av_log(filterCtx, AV_LOG_INFO, "vf_vernus:inputConfig end\n");

	return nRet;

}




static int inputFilterFrame(AVFilterLink* inlink, AVFrame* in)
{
	int nRet = -1;

	AVFilterContext* ctx = inlink->dst;
	AVFilterLink* outlink = ctx->outputs[0];

	VernusContext* vernusCtx = ctx->priv;

	double currentTimeSec = outlink->current_pts * av_q2d(outlink->time_base);
	double processStartTimeSec = vernusCtx->mRenderStart;
	double processEndTimeSec = vernusCtx->mRenderStart + vernusCtx->mRenderDuration;

	av_log(ctx, AV_LOG_INFO, "vf_vernus: inputFilterFrame currentTimeSec=[%lf] processStartTimeSec=[%lf] processEndTimeSec=[%lf]\n", currentTimeSec, processStartTimeSec, processEndTimeSec);

	//不在时间范围内,不处理
	if (currentTimeSec<processStartTimeSec || currentTimeSec>processEndTimeSec)
	{
		return ff_filter_frame(outlink, in);
	}

	AVFrame* outFrame = av_frame_alloc();
	if (!outFrame)
	{
		return ff_filter_frame(outlink, in);
	}

	outFrame->width = in->width;
	outFrame->height = in->height;
	outFrame->format = AV_PIX_FMT_RGBA;

	int outFrameBuffSize = av_image_get_buffer_size(outFrame->format, outFrame->width, outFrame->height, 1);
	void* outFrameBuff = av_malloc(outFrameBuffSize);
	av_image_fill_arrays(&outFrame->data, outFrame->linesize, outFrameBuff, outFrame->format, outFrame->width, outFrame->height, 1);


	int swsRet = processFrameFormatChange(inlink, in, outFrame);
	if (swsRet >= 0)
	{

		av_log(ctx, AV_LOG_INFO, "vf_vernus: inputFilterFrame processFrameFormatChange currentTimeSec=[%lf] in->out success\n", currentTimeSec);

		swsRet = processFrameRGBA(inlink, outFrame);

		if (swsRet >= 0)
		{
			swsRet = processFrameFormatChange(inlink, outFrame, in);

			if (swsRet >= 0)
			{
				av_log(ctx, AV_LOG_INFO, "vf_vernus: inputFilterFrame processFrameFormatChange out->in success\n");
			}

		}

	}


	av_free(outFrameBuff);
	av_frame_free(&outFrame);


	return ff_filter_frame(outlink, in);
}




static av_cold int init(AVFilterContext* ctx)
{
	VernusContext* vernusCtx = ctx->priv;

	glfwSetErrorCallback(glfwOnError);
	return 0;
}


void glfwOnError(int error, const char* description)
{
	av_log(0, AV_LOG_ERROR, "vf_vernus: glfw error #[%d]:\n[%s]\n", error, description);
}


static av_cold void uninit(AVFilterContext* ctx)
{
	VernusContext* vernusCtx = ctx->priv;

	if (!vernusCtx)
	{
		return;
	}

	//清理顶点数据
	if (vernusCtx->mVAO)
	{
		glDeleteVertexArrays(1, &vernusCtx->mVAO);
		vernusCtx->mVAO = 0;
	}

	if (vernusCtx->mVBO)
	{
		glDeleteBuffers(1, &vernusCtx->mVBO);
		vernusCtx->mVBO = 0;
	}


	if (vernusCtx->mEBO)
	{
		glDeleteBuffers(1, &vernusCtx->mEBO);
		vernusCtx->mEBO = 0;
	}


	//清理着色器和程序
	if (vernusCtx->mVetexShader)
	{
		glDeleteShader(vernusCtx->mVetexShader);
		vernusCtx->mVetexShader = 0;
	}

	if (vernusCtx->mFragmentshader)
	{
		glDeleteShader(vernusCtx->mFragmentshader);
		vernusCtx->mFragmentshader = 0;
	}

	if (vernusCtx->mProgram)
	{
		glDeleteProgram(vernusCtx->mProgram);
		vernusCtx->mProgram = 0;
	}

	if (vernusCtx->mFrameTexture)
	{
		GLuint xTextures[] = { vernusCtx->mFrameTexture };
		glDeleteTextures(1, xTextures);
	}


	//清理窗口
	if (vernusCtx->mWindow)
	{
		glfwDestroyWindow(vernusCtx->mWindow);
		vernusCtx->mWindow = NULL;
	}

}

static const AVFilterPad vernusInputs[] = {
	{
		.name = "default",
		.type = AVMEDIA_TYPE_VIDEO,
		.config_props = inputConfig,
		.filter_frame = inputFilterFrame
	}
};



AVFilter ff_vf_vernus = {
	.name = "vernus",
	.description = NULL_IF_CONFIG_SMALL("vernus effect using ffmpeg filter"),
	.priv_size = sizeof(VernusContext),
	.priv_class = &vernus_class,
	.init = init,
	.uninit = uninit,
	.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
	FILTER_INPUTS(vernusInputs),
	FILTER_OUTPUTS(ff_video_default_filterpad),
};