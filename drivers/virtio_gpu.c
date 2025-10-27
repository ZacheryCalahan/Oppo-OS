#include "headers/virtio_gpu.h"
#include "headers/virtio.h"
#include "../klib/headers/stdio.h"
#include "../klib/headers/stdlib.h"
#include "../kernel/headers/kernel.h"

#define GPU_DEFAULT_WIDTH 640
#define GPU_DEFAULT_HEIGHT 480
#define port 1 // MMIO location, 0 indexed from 0x1000.

struct virt_queue *gpu_request_vq;
struct virtio_gpu_ctrl_hdr *gpu_req;
uint32_t gpu_req_paddr;

struct gpu_pixel_rgba *framebuffer;

void bounce() {
    // Bounce animation
	struct gpu_pixel_rgba black = {
			.a = 255,
			.r = 0,
			.g = 0,
			.b = 0
	};

	int x = 0, y = 0; // Position
	int is_left = 0, is_down = 1; // State
	int w = 64, h = 64; // Size

	while (1) {
		clear_screen();

		if (is_left) {
			x -= 1;
		} else {
			x += 1;
		}

		if (is_down) {
			y += 1;
		} else {
			y -= 1;
		}

		if (y == 0 || y == 480 - h) {
			is_down = !is_down;
		}

		if (x == 0 || x == 640 - w) {
			is_left = !is_left;
		}

		draw_rect(framebuffer, x, y, w, h, 0, black);
        flush();

		for (int i = 0; i < 200000; i++) {
			// Delay
		}
	}
}

void *virtio_gpu_init(void) {
    if (virtio_reg_read32(port, VIRTR_MAGIC_O) != VIRTIO_MAGIC_NUMBER) {
        PANIC("virtio: invalid magic value!");
    }
    if (virtio_reg_read32(port, VIRTR_VERSION_O) != 1) {
        PANIC("virtio: invalid version");
    }
    if (virtio_reg_read32(port, VIRTR_DEV_ID_O) != VIRTIO_DEVICE_GPU) {
        PANIC("virtio: invalid device id");
    }

    // Reset device
    virtio_reg_write32(port, VIRTR_DEV_STATUS_O, 0);
    // Set ACK status bit
    virtio_reg_fetch_and_or32(port, VIRTR_DEV_STATUS_O, VIRTIO_STATUS_ACKNOWLEDGE);
    // Set DRIVER status bit
    virtio_reg_fetch_and_or32(port, VIRTR_DEV_STATUS_O, VIRTIO_STATUS_DRIVER);
    // Set FEATURES bit
    virtio_reg_fetch_and_or32(port, VIRTR_DEV_STATUS_O, VIRTIO_STATUS_FEATURES_OK);

    gpu_request_vq = virtq_init(port, 0);
    // Set the DRIVER_OK status bit
    virtio_reg_write32(port, VIRTR_DEV_STATUS_O, VIRTIO_STATUS_DRIVER_OK);

    // Allocate a region to store requests to the device.
    gpu_req_paddr = (uint32_t) kalloc(align_up(sizeof(*gpu_req), PAGE_SIZE) / PAGE_SIZE);
    if (!gpu_req_paddr) {
        PANIC("virtio-gpu: out of memory");
    }
    gpu_req = (struct virtio_gpu_ctrl_hdr *) gpu_req_paddr;

    // Configure scanout.
    struct virtio_gpu_resource_create_2d create2dCmd = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .ring_idx = 0,
        },
        .resource_id = 1,
        .format = VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM,
        .width = GPU_DEFAULT_WIDTH,
        .height = GPU_DEFAULT_HEIGHT
    };

    struct virt_queue *vq = gpu_request_vq;
    uint64_t *resp = (uint64_t *) &create2dCmd + sizeof(create2dCmd);

    vq->buffers[0].addr = (uint64_t) &create2dCmd;
    vq->buffers[0].len = sizeof(create2dCmd);
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;

    vq->buffers[1].addr = (uint64_t) resp;
    vq->buffers[1].len = sizeof(uint64_t);
    vq->buffers[1].flags = VIRTQ_DESC_F_WRITE;

    virtq_kick(port, vq, 0);

    while (virtq_is_busy(vq)) {}
    
    if (*resp != VIRTIO_GPU_RESP_OK_NODATA) {
        printf("virtio_gpu: warn: invalid resource: 0x%x\n", *resp);
    }

    // Set buffer
    framebuffer = kalloc(align_up(sizeof(struct gpu_pixel_rgba) * GPU_DEFAULT_HEIGHT * GPU_DEFAULT_WIDTH, PAGE_SIZE) / PAGE_SIZE);

    struct virtio_gpu_resource_attach_backing attachBacking = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .ring_idx = 0,
        },
        .resource_id = 1,
        .nr_entries = 1
    };

    struct virtio_gpu_mem_entry mem_entry = {
        .addr = (uint64_t) framebuffer,
        .length = GPU_DEFAULT_HEIGHT * GPU_DEFAULT_WIDTH * 4,
        .padding = 0
    };

    resp = (uint64_t *) &attachBacking + sizeof(attachBacking);
    
    vq->buffers[0].addr = (uint64_t) &attachBacking;
    vq->buffers[0].len = sizeof(attachBacking);
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;

    vq->buffers[1].addr = (uint64_t) &mem_entry;
    vq->buffers[1].len = sizeof(mem_entry);
    vq->buffers[1].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[1].next = 2;

    vq->buffers[2].addr = (uint64_t) resp;
    vq->buffers[2].len = sizeof(uint64_t);
    vq->buffers[2].flags = VIRTQ_DESC_F_WRITE;

    virtq_kick(port, vq, 0);

    while (virtq_is_busy(vq)) {}

    if (*resp != VIRTIO_GPU_RESP_OK_NODATA) {
        printf("virtio_gpu: warn: invalid backing: 0x%x\n", *resp);
    }

    // Link the framebuffer to a display scanout.
    struct virtio_gpu_set_scanout setScanOut = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_SET_SCANOUT,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .ring_idx = 0,
        },
        .r = {
            .h = GPU_DEFAULT_HEIGHT,
            .w = GPU_DEFAULT_WIDTH,
            .x = 0,
            .y = 0
        },
        .resource_id = 1,
        .scanout_id = 0
    };

    resp = (uint64_t *) &setScanOut + sizeof(setScanOut);

    vq->buffers[0].addr = (uint64_t) &setScanOut;
    vq->buffers[0].len = sizeof(setScanOut);
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;

    vq->buffers[1].addr = (uint64_t) resp;
    vq->buffers[1].len = sizeof(uint64_t);
    vq->buffers[1].flags = VIRTQ_DESC_F_WRITE;

    virtq_kick(port, vq, 0);

    while (virtq_is_busy(vq)) {}
    
    if (*resp != VIRTIO_GPU_RESP_OK_NODATA) {
        printf("virtio_gpu: warn: invalid scan out: 0x%x\n", *resp);
    }

    printf("virtio_gpu: initialized.\n");

    bounce();

    return 0;
}

void clear_screen() {
    struct gpu_pixel_rgba white = {
        .r = 255,
        .a = 255,
        .g = 255,
        .b = 255
    };

    for (int i = 0; i < GPU_DEFAULT_HEIGHT * GPU_DEFAULT_WIDTH; i++) {
        framebuffer[i] = white;
    }
}

static struct gpu_pixel_rgba hex_to_rgba(uint64_t color) {
    struct gpu_pixel_rgba rgba;
    memcpy(&rgba, &color, 4);
    return rgba;
}

void set_pixel(struct gpu_pixel_rgba *fb, uint32_t x, uint32_t y, struct gpu_pixel_rgba color) {
    if (x < GPU_DEFAULT_WIDTH && y < GPU_DEFAULT_HEIGHT) {
        framebuffer[y * GPU_DEFAULT_WIDTH + x] = color;
    }
}

void fill_rect(struct gpu_pixel_rgba *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct gpu_pixel_rgba color) {
    for (uint32_t row = y; row < (y + h); row++) {
        for (uint32_t col = x; col < (x + w); col++) {
            set_pixel(fb, col, row, color);
        }
    }
}

void stroke_rect(struct gpu_pixel_rgba *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct gpu_pixel_rgba color, uint32_t size) {
    fill_rect(fb, x, y, w, size, color);
    fill_rect(fb, x, y + h, w, size, color);
    fill_rect(fb, x, y, size, h, color);
    fill_rect(fb, x + w, y, size, h + size, color);
}

void draw_rect(struct gpu_pixel_rgba *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t size, struct gpu_pixel_rgba color) {
    if (size == 0) {
        fill_rect(fb, x, y, w, h, color);
        return;
    }

    stroke_rect(fb, x, y, w, h, color, size);
}

void flush() {
    uint64_t *resp; // Location of the response
    struct virt_queue *vq = gpu_request_vq;

    // Move buffer to the GPU
    struct virtio_gpu_transfer_to_host_2d transfer = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .ring_idx = 0,
        },
        .r = {
            .h = GPU_DEFAULT_HEIGHT,
            .w = GPU_DEFAULT_WIDTH,
            .x = 0,
            .y = 0
        },
        .offset = 0,
        .resource_id = 1,
        .padding = 0
    };
    resp = (uint64_t *) &transfer;

    vq->buffers[0].addr = (uint64_t) &transfer;
    vq->buffers[0].len = sizeof(transfer);
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;
    vq->buffers[1].addr = (uint64_t) framebuffer;
    vq->buffers[1].len = sizeof(framebuffer);
    vq->buffers[1].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[1].next = 2;
    vq->buffers[2].addr = (uint64_t) resp;
    vq->buffers[2].len = sizeof(uint64_t);
    vq->buffers[2].flags = VIRTQ_DESC_F_WRITE;
    virtq_kick(port, vq, 0);
    while (virtq_is_busy(vq)) {}
    if (*resp != VIRTIO_GPU_RESP_OK_NODATA) {
        printf("virtio_gpu: warn: invalid response: 0x%x\n", *resp);
    }

    // Flush
    struct virtio_gpu_resource_flush flush = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_RESOURCE_FLUSH,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .ring_idx = 0,
        },
        .r = {
            .h = GPU_DEFAULT_HEIGHT,
            .w = GPU_DEFAULT_WIDTH,
            .x = 0,
            .y = 0
        },
        .resource_id = 1,
        .padding = 0
    };
    resp = (uint64_t *) &flush + sizeof(flush);

    vq->buffers[0].addr = (uint64_t) &flush;
    vq->buffers[0].len = sizeof(flush);
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;

    vq->buffers[1].addr = (uint64_t) resp;
    vq->buffers[1].len = sizeof(uint64_t);
    vq->buffers[1].flags = VIRTQ_DESC_F_WRITE;

    virtq_kick(port, vq, 0);

    while (virtq_is_busy(vq)) {}
    
    if (*resp != VIRTIO_GPU_RESP_OK_NODATA) {
        printf("virtio_gpu: warn: invalid response: 0x%x\n", *resp);
    }
}







