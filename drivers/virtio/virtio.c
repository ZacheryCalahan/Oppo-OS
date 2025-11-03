#include "virtio.h"
#include "../../klib/stdlib.h"
#include "../../kernel/kernel.h"

uint32_t virtio_reg_read32(unsigned port, unsigned offset) {
    return *((volatile uint32_t *) ((VIRTIO_PADDR + (port * 0x1000)) + offset));
}

uint64_t virtio_reg_read64(unsigned port, unsigned offset) {
    return *((volatile uint64_t *)((VIRTIO_PADDR + (port * 0x1000)) + offset));
}

void virtio_reg_write32(unsigned port, unsigned offset, uint32_t value) {
    *((volatile uint32_t *) ((VIRTIO_PADDR + (port * 0x1000)) + offset)) = value;
}

void virtio_reg_fetch_and_or32(unsigned port, unsigned offset, uint32_t value) {
    virtio_reg_write32(port, offset, virtio_reg_read32(port, offset) | value);
}

struct virt_queue *virtq_init(unsigned port, unsigned index) {
    // Allocate a region for the virtqueue
    uint64_t virtq_paddr = (uint64_t) kalloc(align_up(sizeof(struct virt_queue), PAGE_SIZE) / PAGE_SIZE);
    if (!virtq_paddr) {
        PANIC("virtio: out of memory");
    }
    struct virt_queue *vq = (struct virt_queue *) virtq_paddr;
    vq->queue_index = index;
    vq->used_index = (volatile uint16_t *) &vq->used.idx;

    // Select the queue writing its index
    virtio_reg_write32(port, VIRTR_QUEUE_SEL_O, index);
    // Notify the device about the queue size
    virtio_reg_write32(port, VIRTR_QUEUE_NUM_O, VIRTQ_ENTRY_NUM);
    // Notify the device about the used alignment
    virtio_reg_write32(port, VIRTR_QUEUE_ALIGN_O, 0);
    // Write the physical number of the first page of the queue
    virtio_reg_write32(port, VIRTR_QUEUE_PFN_O, virtq_paddr);
    return vq;
}

void virtq_kick(unsigned port, struct virt_queue *vq, int desc_index) {
    vq->avail.ring[vq->avail.idx % VIRTQ_ENTRY_NUM] = desc_index;
    vq->avail.idx++;
    __sync_synchronize();
    virtio_reg_write32(port, VIRTR_QUEUE_NOTIFY_O, vq->queue_index);
    vq->last_used_index++;
}

int virtq_is_busy(struct virt_queue *vq) {
    return vq->last_used_index != *vq->used_index;
}


