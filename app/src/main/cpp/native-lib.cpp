#include <jni.h>
#include <string>
#include "stdio.h"

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}

#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"OpenSLESDemo",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"OpenSLESDemo",FORMAT,##__VA_ARGS__);

SLObjectItf slEngineObjectItf = NULL;
SLEngineItf slEngineItf = NULL;
SLObjectItf outputObjectItf = NULL;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
SLObjectItf playerObject = NULL;
SLPlayItf pcmPlayerPlay = NULL;
SLVolumeItf pcmVolumeItf = NULL;
SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

FILE *pcmFile;
uint8_t *out_buffer;
void *buffer;

void getPcmData(void **pcm) {
    while (!feof(pcmFile)) {
        fread(out_buffer, 44100 * 2 * 2, 1, pcmFile);
        if (out_buffer == NULL) {
            LOGI("read end");
            break;
        } else {
            LOGI("reading");
        }
        *pcm = out_buffer;
        break;
    }
}

void pcmBufferCallback(SLAndroidSimpleBufferQueueItf bf, void *context) {
    getPcmData(&buffer);
    if (NULL != buffer) {
        (*pcmBufferQueue)->Enqueue(pcmBufferQueue, buffer, 44100 * 2 * 2);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ray_openslesdemo_MainActivity_playPCM(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    pcmFile = fopen(url, "r");
    if (pcmFile == NULL) {
        LOGE("can't open %s", url);
        return;
    }
    out_buffer = (uint8_t *) (malloc(44100 * 2 * 2));
    //1.创建引擎对象
    slCreateEngine(&slEngineObjectItf, 0, 0, 0, 0, 0);
    (*slEngineObjectItf)->Realize(slEngineObjectItf, SL_BOOLEAN_FALSE);
    (*slEngineObjectItf)->GetInterface(slEngineObjectItf, SL_IID_ENGINE, &slEngineItf);

    const SLInterfaceID pInterfaceIds[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean pInterfaceRequireds[1] = {SL_BOOLEAN_FALSE};
    //2.创建混音器
    (*slEngineItf)->CreateOutputMix(slEngineItf, &outputObjectItf, 1, pInterfaceIds,
                                    pInterfaceRequireds);
    (*outputObjectItf)->Realize(outputObjectItf, SL_BOOLEAN_FALSE);
    (*outputObjectItf)->GetInterface(outputObjectItf, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);

    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,
                                                                      &reverbSettings);
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputObjectItf};
    //3.创建播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//数据格式
            2,//声道格式
            SL_SAMPLINGRATE_44_1, //采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource dataSource = {&android_queue, &pcm};
    SLDataSink dataSink = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean reqs[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*slEngineItf)->CreateAudioPlayer(slEngineItf, &playerObject, &dataSource, &dataSink, 3, ids,
                                      reqs);
    //初始化播放器
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    //得到接口
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &pcmPlayerPlay);

    //4创建缓冲区和回调函数
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallback, NULL);
    //获取音量接口
    (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &pcmVolumeItf);
    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    pcmBufferCallback(pcmBufferQueue, NULL);

//    env->ReleaseStringUTFChars(url_, url);
}
