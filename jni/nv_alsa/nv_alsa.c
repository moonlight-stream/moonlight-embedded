/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

snd_pcm_t *handle;

int nv_alsa_init(unsigned int channelCount, unsigned int sampleRate, unsigned char* device) {
	int rc;
	
	/* Open PCM device for playback. */
	if ((rc = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) != 0)
		return rc;
	
	if ((rc = snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, channelCount, sampleRate, 1, 50000)) != 0) //50ms latency
		return rc;
}

int nv_alsa_play(const unsigned char* indata, int data_len) {
	int frames = data_len/4; /* 2 bytes/sample, 2 channels */
	int rc = snd_pcm_writei(handle, indata, frames);
	if (rc == -EPIPE)
		snd_pcm_prepare(handle);
	
	return rc;
}

int nv_alsa_close(void) {
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
}
