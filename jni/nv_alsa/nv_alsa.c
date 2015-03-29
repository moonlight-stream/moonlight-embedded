/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

#define CHECK_RETURN(f) if ((rc = f) != 0) return rc;

snd_pcm_t *handle;
unsigned int channels;

int nv_alsa_init(unsigned int channelCount, unsigned int sampleRate, unsigned char* device) {
	int rc;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_uframes_t period_size = 240 * channelCount * 2;
	snd_pcm_uframes_t buffer_size = 12 * period_size;

	channels = channelCount;

	/* Open PCM device for playback. */
	CHECK_RETURN(snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK))

	/* Set hardware parameters */
	CHECK_RETURN(snd_pcm_hw_params_malloc(&hw_params));
	CHECK_RETURN(snd_pcm_hw_params_any(handle, hw_params));
	CHECK_RETURN(snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
	CHECK_RETURN(snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE));
	CHECK_RETURN(snd_pcm_hw_params_set_rate_near(handle, hw_params, &sampleRate, NULL));
	CHECK_RETURN(snd_pcm_hw_params_set_channels(handle, hw_params, channelCount));
	CHECK_RETURN(snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffer_size));
	CHECK_RETURN(snd_pcm_hw_params_set_period_size_near(handle, hw_params, &period_size, NULL));
	CHECK_RETURN(snd_pcm_hw_params(handle, hw_params));
	snd_pcm_hw_params_free(hw_params);

	/* Set software parameters */
	CHECK_RETURN(snd_pcm_sw_params_malloc(&sw_params));
	CHECK_RETURN(snd_pcm_sw_params_current(handle, sw_params));
	CHECK_RETURN(snd_pcm_sw_params_set_start_threshold(handle, sw_params, buffer_size - period_size));
	CHECK_RETURN(snd_pcm_sw_params_set_avail_min(handle, sw_params, period_size));
	CHECK_RETURN(snd_pcm_sw_params(handle, sw_params));
	snd_pcm_sw_params_free(sw_params);

	CHECK_RETURN(snd_pcm_prepare(handle));

	return 0;
}

int nv_alsa_play(const unsigned char* indata, int data_len) {
	int frames = data_len / (2 * channels); /* 2 bytes/sample */
	int rc = snd_pcm_writei(handle, indata, frames);
	if (rc == -EPIPE)
		snd_pcm_recover(handle, rc, 1);
	
	return rc;
}

int nv_alsa_close(void) {
	if (handle != NULL) {
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	}
}
