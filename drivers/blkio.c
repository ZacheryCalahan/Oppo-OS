#include "headers/blkio.h"
#include "headers/virtio.h"
#include "../klib/headers/stdio.h"
#include "../klib/headers/stdlib.h"

struct virt_queue *blk_request_vq;
struct virtio_blk_req *blk_req;
uint64_t blk_req_paddr;
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
    printf("virtio-blk: capacity is $d bytes\n", blk_capacity);

    // Allocate a region to store requests to the device.
    // blk_req_paddr = kaligned_alloc() continue writing here!

}