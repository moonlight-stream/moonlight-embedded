/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

snd_pcm_t *handle;

int nv_alsa_init(unsigned int channelCount, unsigned int sampleRate) {
	int rc;
	snd_pcm_hw_params_t *params;
	int dir;
	
	/* Open PCM device for playback. */
	if ((rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) != 0)
		return rc;
	
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	
	if ((rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) != 0)
                return rc;

	if ((rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) != 0)
                return rc;

	if ((rc = snd_pcm_hw_params_set_channels(handle, params, channelCount)) != 0)
                return rc;

	if ((rc = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir)) != 0)
                return rc;
	
	snd_pcm_uframes_t frames = 32;
	if ((rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir)) != 0)
		return rc;
	
	if ((rc = snd_pcm_hw_params(handle, params)) != 0)
		return rc;
}

int nv_alsa_play(const unsigned char* indata, int data_len) {
	int frames = data_len/4; /* 2 bytes/sample, 2 channels */
	int rc = snd_pcm_writei(handle, indata, frames);
	if (rc == -EPIPE) {
		/* EPIPE means underrun */
		fprintf(stderr, "underrun occurred\n");
		snd_pcm_prepare(handle);
	} else if (rc < 0) {
		fprintf(stderr,
			"error from writei: %s\n",
			snd_strerror(rc));
	} else if (rc != (int) frames) {
		fprintf(stderr,
			"short write, write %d frames\n", rc);
	}	
}

int nv_alsa_close(void) {
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
}
