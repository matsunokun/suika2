/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * OpenAL 1.1 sound module for Emscripten
 *
 * [Changes]
 *  2021-06-27 作成
 */

#include "suika.h"

#include <AL/al.h>
#include <AL/alc.h>

/*
 * サンプリングレートとバッファのサンプル数
 */
#define SAMPLING_RATE	(44100)
#define SAMPLES		(SAMPLING_RATE/4)

/*
 * バッファ数
 */
#define BUFFER_COUNT	(4)

/*
 * サウンドデバイス
 */
static ALCdevice *device;

/*
 * サウンドコンテキスト
 */
static ALCcontext *context;

/*
 * バッファ
 */
static ALuint buffer[MIXER_STREAMS][BUFFER_COUNT];

/*
 * ソース
 */
static ALuint source[MIXER_STREAMS];

/*
 * PCMストリーム
 */
static struct wave *stream[MIXER_STREAMS];

/*
 * サンプルの一時格納場所
 */
static uint32_t tmp_buf[SAMPLES];

/*
 * サウンドの初期化を行う
 */
bool init_openal(void)
{
	ALenum error;
	int i;

	/* デバイスを開く */
	device = alcOpenDevice(NULL);

	/* コンテキストを作成する */
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	alGetError();

	/* バッファを作成する */
	for (i = 0; i < MIXER_STREAMS; i++)
		alGenBuffers(BUFFER_COUNT, buffer[i]);

	/* ソースを作成する */
	alGenSources(MIXER_STREAMS, source);
	for (i = 0; i < MIXER_STREAMS; i++) {
		alSourcef(source[i], AL_GAIN, 1);
		alSource3f(source[i], AL_POSITION, 0, 0, 0);
	}

	return true;
}

/*
 * サウンドの終了処理を行う
 */
void cleanup_openal(void)
{
	/* TODO */
}

/*
 * サウンドを再生を開始する
 */
bool play_sound(int n, struct wave *w)
{
	ALuint buf[BUFFER_COUNT];
	int i, samples;

	/* 再生中であれば停止する */
	if (stream[n] != NULL) {
		alSourceStop(source[n]);
		alSourceUnqueueBuffers(source[n], BUFFER_COUNT, buf);
	}

	/* ストリームを保持する */
	stream[n] = w;

	/* バッファを埋める */
	for (i = 0; i < BUFFER_COUNT; i++) {
		samples = get_wave_samples(stream[n], tmp_buf, SAMPLES);
		alBufferData(buffer[n][i], AL_FORMAT_STEREO16, tmp_buf,
			     samples * sizeof(uint32_t), SAMPLING_RATE);
	}

	alSourceQueueBuffers(source[n], BUFFER_COUNT, buffer[n]);
	alSourcePlay(source[n]);

	return true;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int n)
{
	ALuint buf[BUFFER_COUNT];

	/* 再生中であれば停止する */
	if (stream[n] != NULL) {
		alSourceStop(source[n]);
		alSourceUnqueueBuffers(source[n], BUFFER_COUNT, buf);
		stream[n] = NULL;
	}

	return true;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int n, float vol)
{
	alSourcef(source[n], AL_GAIN, vol);

	return true;
}

/*
 * サウンドのバッファを埋める
 */
void fill_sound_buffer(void)
{
	ALuint buf;
	int n, processed, samples;

	for (n = 0; n < MIXER_STREAMS; n++) {
		/* 処理済みのバッファ数を取得する */
		alGetSourcei(source[n], AL_BUFFERS_PROCESSED, &processed);

		/* 処理済みのバッファについて */
		while (processed-- > 0) {
			/* バッファをアンキューする */
			alSourceUnqueueBuffers(source[n], 1, &buf);

			/* サンプルを取得する */
			samples = get_wave_samples(stream[n], tmp_buf,
						   SAMPLES);

			/* バッファに書き込む */
			alBufferData(buf, AL_FORMAT_STEREO16, tmp_buf,
				     samples * sizeof(uint32_t),
				     SAMPLING_RATE);

			/* バッファをキューする */
			alSourceQueueBuffers(source[n], 1, &buf);
		}
	}
}

/*
 * サウンドを一時停止する
 */
void pause_sound(void)
{
	int i;
	for (i = 0; i < MIXER_STREAMS; i++)
		if (stream[i] != NULL)
			alSourceStop(source[i]);
}

/* サウンドを再開する */
void resume_sound(void)
{
	int i;
	for (i = 0; i < MIXER_STREAMS; i++)
		if (stream[i] != NULL)
			alSourcePlay(source[i]);
}