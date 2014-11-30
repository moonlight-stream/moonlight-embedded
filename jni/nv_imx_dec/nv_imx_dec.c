#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libv4l2.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/fb.h>
#include <linux/ioctl.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include "vpu_io.h"
#include "vpu_lib.h"

#define STREAM_BUF_SIZE      0x200000
#define PS_SAVE_SIZE         0x080000

#define MODE420              1
#define MODE422              2
#define MODE224              3

struct mxcfb_gbl_alpha {
      int enable;
      int alpha;
};
#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)


struct v4l_buf {
	void *start;
	off_t offset;
	size_t length;	
};

vpu_mem_desc mem_desc = {0};
vpu_mem_desc ps_mem_desc = {0};
vpu_mem_desc slice_mem_desc = {0};

DecHandle handle = {0};
DecParam decparam = {0};
int fd;

int initialized = 0, decoding = 0;
int loop_id = 0;

int threshold;
int queued_count = 0;
int ncount = 0;
int disp_clr_index = 0;

FrameBuffer *fb;
struct v4l2_buffer dbuf;

int nv_imx_init(void) {
	struct mxcfb_gbl_alpha alpha;

	dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	dbuf.memory = V4L2_MEMORY_MMAP;
	
	int fd_fb = open("/dev/fb0", O_RDWR, 0);

	if (fd_fb < 0) {
		return -1;
	}
	alpha.alpha = 0;
	alpha.enable = 1;
	if (ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &alpha) < 0) {
		return -2;
	}
	close(fd_fb);
	
	if (vpu_Init(NULL) != RETCODE_SUCCESS)
		return -3;

	mem_desc.size = STREAM_BUF_SIZE;
	if (IOGetPhyMem(&mem_desc)) {
		return -4;
	}

	if (IOGetVirtMem(&mem_desc) <= 0) {
		return -5;
	}

	ps_mem_desc.size = PS_SAVE_SIZE;
	if (IOGetPhyMem(&ps_mem_desc)) {
		return -6;
	}

	DecOpenParam oparam = {0};	
	oparam.bitstreamFormat = STD_AVC;
	oparam.bitstreamBuffer = mem_desc.phy_addr;
	oparam.bitstreamBufferSize = STREAM_BUF_SIZE;
	oparam.pBitStream = (Uint8 *) mem_desc.virt_uaddr;
	oparam.reorderEnable = 1;
	oparam.mp4DeblkEnable = 0;
	oparam.chromaInterleave = 0;
	oparam.avcExtension = oparam.mp4Class = 0;
	oparam.mjpg_thumbNailDecEnable = 0;
	oparam.mapType = LINEAR_FRAME_MAP;
	oparam.tiled2LinearEnable = 0;
	oparam.bitstreamMode = 1;

	oparam.psSaveBuffer = ps_mem_desc.phy_addr;
	oparam.psSaveBufferSize = PS_SAVE_SIZE;

	if (vpu_DecOpen(&handle, &oparam) != RETCODE_SUCCESS) {
		return -7;
	}

	decparam.dispReorderBuf = 0;
	decparam.skipframeMode = 0;
	decparam.skipframeNum = 0;
	decparam.iframeSearchEnable = 0;
	
	return 0;
}

int nv_imx_decode(const unsigned char* indata, int data_len, int last) {
	//Write information
	Uint32 target_addr, space;
	PhysicalAddress pa_read_ptr, pa_write_ptr;
	RetCode ret;
	int room;
	size_t read;
	//Initialize
	DecInitialInfo initinfo = {0};
	DecBufInfo bufinfo;
	//Display using v4l
	int type;
	struct timeval tv;

	if (vpu_DecGetBitstreamBuffer(handle, &pa_read_ptr, &pa_write_ptr, &space) != RETCODE_SUCCESS) {
		return -1;
	}
		
	target_addr = mem_desc.virt_uaddr + (pa_write_ptr - mem_desc.phy_addr);

	if ( (target_addr + data_len) > mem_desc.virt_uaddr + STREAM_BUF_SIZE) {
		room = mem_desc.virt_uaddr + STREAM_BUF_SIZE - target_addr;
		memcpy((void *)target_addr, indata, room);
		memcpy((void *)mem_desc.virt_uaddr, indata + room, data_len - room);
	} else {
		memcpy((void *)target_addr, indata, data_len);
	}
	vpu_DecUpdateBitstreamBuffer(handle, data_len);	
	
	if (last) {
		if (!initialized) {
			initialized = 1;
			
			vpu_DecSetEscSeqInit(handle, 1);
			if (vpu_DecGetInitialInfo(handle, &initinfo) != RETCODE_SUCCESS) {
				return -2;
			}
			vpu_DecSetEscSeqInit(handle, 0);
			
			int regfbcount = initinfo.minFrameBufferCount + 2;
			threshold = regfbcount - initinfo.minFrameBufferCount;
			int picWidth = ((initinfo.picWidth + 15) & ~15);
			int picHeight = ((initinfo.picHeight + 15) & ~15);
			int stride = picWidth;

			int phy_slicebuf_size = initinfo.worstSliceSize * 1024;
			
			slice_mem_desc.size = phy_slicebuf_size;
			if (IOGetPhyMem(&slice_mem_desc)) {
				return -3;
			}	

			fb = calloc(regfbcount, sizeof(FrameBuffer));
			if (fb == NULL) {
				return -4;
			}

			char v4l_device[16], node[8];
			sprintf(node, "%d", 17);
			strcpy(v4l_device, "/dev/video");
			strcat(v4l_device, node);
			fd = open(v4l_device, O_RDWR, 0);
			struct v4l2_format fmt = {0};
			fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;	
			fmt.fmt.pix.width = picWidth;
			fmt.fmt.pix.height = picHeight;
			fmt.fmt.pix.bytesperline = picWidth;
			fmt.fmt.pix.field = V4L2_FIELD_ANY;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
			if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
				return -5;
			}

			if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
				return -6;
			}

			struct v4l2_requestbuffers reqbuf = {0};
			reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			reqbuf.memory = V4L2_MEMORY_MMAP;
			reqbuf.count = regfbcount;

			struct v4l_buf* buffers[regfbcount];

			if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
				return -7;
			}

			if (reqbuf.count < regfbcount) {
				return -8;
			}
			
			int i;
			for (i = 0; i < regfbcount; i++) {
				struct v4l2_buffer buffer = {0};
				struct v4l_buf *buf;

				buf = calloc(1, sizeof(struct v4l_buf));
				if (buf == NULL) {
					return -9;
				}

				buffers[i] = buf;

				buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
				buffer.memory = V4L2_MEMORY_MMAP;
				buffer.index = i;

				if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
					return -10;
				}
				buf->start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, buffer.m.offset);

				/*
				* Workaround for new V4L interface change, this change
				* will be removed after V4L driver is updated for this.
				* Need to call QUERYBUF ioctl again after mmap.
				*/
				if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
					return -11;
				}

				buf->offset = buffer.m.offset;
				buf->length = buffer.length;

				if (buf->start == MAP_FAILED) {
					return -12;
				}

			}
			
			int img_size = stride * picHeight;
			vpu_mem_desc *mvcol_md = NULL;
			
			int mjpg_fmt = MODE420;
			int divX = (mjpg_fmt == MODE420 || mjpg_fmt == MODE422) ? 2 : 1;
			int divY = (mjpg_fmt == MODE420 || mjpg_fmt == MODE224) ? 2 : 1;

			mvcol_md = calloc(regfbcount, sizeof(vpu_mem_desc));

			for (i = 0; i < regfbcount; i++) {
				fb[i].myIndex = i;
				fb[i].bufY = buffers[i]->offset;
				fb[i].bufCb = fb[i].bufY + img_size;
				fb[i].bufCr = fb[i].bufCb + (img_size / divX / divY);
						
				/* allocate MvCol buffer here */
				memset(&mvcol_md[i], 0, sizeof(vpu_mem_desc));
				mvcol_md[i].size = img_size / divX / divY;
				if (IOGetPhyMem(&mvcol_md[i])) {
					return -13;
				}
				fb[i].bufMvCol = mvcol_md[i].phy_addr;
			}
			
			bufinfo.avcSliceBufInfo.bufferBase = slice_mem_desc.phy_addr;
			bufinfo.avcSliceBufInfo.bufferSize = phy_slicebuf_size;
			
			bufinfo.maxDecFrmInfo.maxMbX = stride / 16;
			bufinfo.maxDecFrmInfo.maxMbY = picHeight / 16;
			bufinfo.maxDecFrmInfo.maxMbNum = stride * picHeight / 256;
			
			int delay = -1;
			vpu_DecGiveCommand(handle, DEC_SET_FRAME_DELAY, &delay);

			if (vpu_DecRegisterFrameBuffer(handle, fb, regfbcount, stride, &bufinfo) != RETCODE_SUCCESS) {
				return -14;
			}
		}
		
		if (!decoding) {
			if (vpu_DecStartOneFrame(handle, &decparam) != RETCODE_SUCCESS) {
				return -1;
			}
			decoding = 15;
		}
	}
	
	while (vpu_IsBusy()) {
		if (loop_id > 50) {
			vpu_SWReset(handle, 0);
			return -16;
		}
		vpu_WaitForInt(100);
		loop_id++;
	}
	loop_id = 0;

	if (decoding) {
		decoding = 0;

		DecOutputInfo outinfo = {0};
		if (vpu_DecGetOutputInfo(handle, &outinfo) != RETCODE_SUCCESS) {
			return -17;
		}

		if (outinfo.decodingSuccess & 0x10) {
			return 0;
		} else if (outinfo.notSufficientPsBuffer) {
			return -19;
		} else if (outinfo.notSufficientSliceBuffer) {
			return -20;
		}

		if (outinfo.indexFrameDisplay >= 0)	{
			gettimeofday(&tv, 0);
			dbuf.timestamp.tv_sec = tv.tv_sec;
			dbuf.timestamp.tv_usec = tv.tv_usec;
			dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			dbuf.memory = V4L2_MEMORY_MMAP;

			dbuf.index = outinfo.indexFrameDisplay;
			if (ioctl(fd, VIDIOC_QUERYBUF, &dbuf) < 0) {
				return -21;
			}

			dbuf.index = outinfo.indexFrameDisplay;
			dbuf.field =  V4L2_FIELD_NONE;
			if (ioctl(fd, VIDIOC_QBUF, &dbuf) < 0) {
				return -22;
			}

			if (ncount == 1) {
				type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
				if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
					return -23;
				}
			}

			ncount++;
			queued_count++;

			if (queued_count > threshold) {
				dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
				dbuf.memory = V4L2_MEMORY_MMAP;
				if (ioctl(fd, VIDIOC_DQBUF, &dbuf) < 0) {
					return -24;
				} else
					queued_count--;
			} else
				dbuf.index = -1;
			
			if (disp_clr_index >= 0)
				vpu_DecClrDispFlag(handle, disp_clr_index);
			
			disp_clr_index = outinfo.indexFrameDisplay;
		} else if (outinfo.indexFrameDisplay == -1)
			return -25;
	}
	
	return 0;
}

void nv_imx_destroy(void) {
	IOFreePhyMem(&ps_mem_desc);
	IOFreePhyMem(&slice_mem_desc);
	
	IOFreeVirtMem(&mem_desc);
	IOFreePhyMem(&mem_desc);
	vpu_UnInit();
}
