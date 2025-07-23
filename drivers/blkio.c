/*
    NOTE: This assumes only one blk_io device at a time!
*/

#include "headers/blkio.h"
#include "headers/virtio.h"
#include "../klib/headers/stdio.h"
#include "../klib/headers/stdlib.h"
#include "../kernel/headers/kernel.h"

struct virt_queue *blk_request_vq;
struct virtio_blk_req *blk_req;
uint32_t blk_req_paddr; // May also be wrong because riscv32/riscv64
uint64_t blk_capacity;

void virtio_blk_init(void) {
    if (virtio_reg_read32(VIRTR_MAGIC_O) != VIRTIO_MAGIC_NUMBER) {
        PANIC("virtio: invalid magic value!");
    }
    if (virtio_reg_read32(VIRTR_VERSION_O) != 1) {
        PANIC("virtio: invalid version");
    }
    if (virtio_reg_read32(VIRTR_DEV_ID_O) != VIRTIO_DEVICE_BLK) {
        PANIC("virtio: invalid device id");
    }

    // Reset device
    virtio_reg_write32(VIRTR_DEV_STATUS_O, 0);
    // Set ACK status bit
    virtio_reg_fetch_and_or32(VIRTR_DEV_STATUS_O, VIRTIO_STATUS_ACKNOWLEDGE);
    // Set DRIVER status bit
    virtio_reg_fetch_and_or32(VIRTR_DEV_STATUS_O, VIRTIO_STATUS_DRIVER);
    // Set FEATURES bit
    virtio_reg_fetch_and_or32(VIRTR_DEV_STATUS_O, VIRTIO_STATUS_FEATURES_OK);
    
    // BLK device setup!
    blk_request_vq = virtq_init(0);
    // Set the DRIVER_OK status bit
    virtio_reg_write32(VIRTR_DEV_STATUS_O, VIRTIO_STATUS_DRIVER_OK);

    // Get disk capacity
    blk_capacity = virtio_reg_read64(VIRTR_CFG_O + 0) * SECTOR_SIZE;
    printf("virtio-blk: capacity is %d bytes\n", blk_capacity);

    // Allocate a region to store requests to the device.
    blk_req_paddr = (uint32_t) kalloc(align_up(sizeof(*blk_req), PAGE_SIZE) / PAGE_SIZE);
    if (!blk_req_paddr) {
        PANIC("virtio-blk: out of memory");
    }
    blk_req = (struct virtio_blk_req *) blk_req_paddr;
}

void read_write_disk(void *buf, unsigned sector, int is_write) {
    if (sector >= blk_capacity / SECTOR_SIZE) {
        PANIC("virtio-blk: tried to read/write sector=%d, but capacity is %d", sector, blk_capacity / SECTOR_SIZE);
        return;
    }

    // Build request
    blk_req->sector = sector;
    blk_req->type = is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    if (is_write) {
        memcpy(blk_req->data, buf, SECTOR_SIZE);
    }

    // Build virtqueue descriptors
    struct virt_queue *vq = blk_request_vq;
    vq->buffers[0].addr = blk_req_paddr;
    vq->buffers[0].len = sizeof(uint32_t) * 2 + sizeof(uint64_t); // sizeof(type, reserved, sector) in virtio_blk_req
    vq->buffers[0].flags = VIRTQ_DESC_F_NEXT;
    vq->buffers[0].next = 1;

    vq->buffers[1].addr = blk_req_paddr + offsetof(struct virtio_blk_req, data);
    vq->buffers[1].len = SECTOR_SIZE;
    vq->buffers[1].flags = VIRTQ_DESC_F_NEXT | (is_write ? 0 : VIRTQ_DESC_F_WRITE);
    vq->buffers[1].next = 2;

    vq->buffers[2].addr = blk_req_paddr + offsetof(struct virtio_blk_req, status);
    vq->buffers[2].len = sizeof(uint8_t);
    vq->buffers[2].flags = VIRTQ_DESC_F_WRITE;

    // Notify device of request
    virtq_kick(vq, 0);

    // Wait unitl the device finishes processing.
    while (virtq_is_busy(vq)) {}

    if (blk_req->status != 0) {
        printf("virtio-blk: warn: failed to r/w sector=%d status%d\n", sector, blk_req->status);
        return;
    }

    if (!is_write) {
        memcpy(buf, blk_req->data, SECTOR_SIZE);
    }
}
