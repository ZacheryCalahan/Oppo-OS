#ifndef BLKIO_H
#define BLKIO_H

#include <stdint.h>

#define SECTOR_SIZE         512
#define VIRTIO_DEVICE_BLK   2

struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
    uint8_t data[512];
    uint8_t status;
} __attribute__((packed));

void virtio_blk_init(void);

#endif