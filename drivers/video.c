#include "headers/video.h"
#include "headers/virtio.h"
#include "../kernel/stdlib/headers/stdlib.h"
#include "../kernel/stdlib/headers/stdio.h"

#define SUPPORTED_FEATURES 0x0
#define VIRTIO_BASE_ADDRESS_MMIO 0x10001000

int32_t virtio_gpu_init_mmio() {

    // Find the device by enumeration.
    volatile uint32_t * gpu_base;

    for (int i = 0; i < 8; i++) {
        uint32_t* curr_addr = (volatile uint32_t *)(VIRTIO_BASE_ADDRESS_MMIO + (i * 0x1000));
        printf("Checking address 0x%x for virtio-gpu. ", curr_addr);

        if (curr_addr[VIRTR_MAGIC_O] != VIRTIO_MAGIC_NUMBER) {
            // This is not a valid virtio device.
            printf("Invalid magic number found: 0x%x\n", curr_addr[VIRTR_MAGIC_O]);
            continue;
        }

        if (curr_addr[VIRTR_VERSION_O] == VIRTIO_VERSION) {
            // This is not a valid virtio version.
            printf("Invalid Version found: 0x%x\n", curr_addr[VIRTR_VERSION_O]);
            continue;
        }

        if (curr_addr[VIRTR_DEV_ID_O] != (uint32_t) 16) {
            // Device isn't of type GPU.
            printf("Invalid Device ID found: 0x%x\n", curr_addr[VIRTR_DEV_ID_O]);
            continue;
        }
        
        // Device found, set as current GPU device.
        gpu_base = curr_addr;
    }

    if (gpu_base == NULL) {
        return -1;
    }

    // Tell the GPU we found it
    gpu_base[VIRTR_DEV_STATUS_O] |= VIRTIO_STATUS_ACKNOWLEDGE;
    gpu_base[VIRTR_DEV_STATUS_O] |= VIRTIO_STATUS_DRIVER;

    // Start configuration
    gpu_base[VIRTR_DEV_STATUS_O] = 0x0; // Resets the device

    gpu_base[VIRTR_DEV_FEATURES_SEL_O] = 0x1; // Must write before reading features.
    uint64_t dev_features = gpu_base[VIRTR_DEV_FEATURES_O];
    if (dev_features != 0) {
        gpu_base[VIRTR_DRV_FEATURES_SEL_0] = 0x1; // Tell device we're handling features the driver supports
        gpu_base[VIRTR_DRV_FEATURES_O] = SUPPORTED_FEATURES;
    }
    
    gpu_base[VIRTR_DEV_STATUS_O] |= VIRTIO_STATUS_FEATURES_OK; // We're done changing features

    // Ensure that the device agrees with our features.
    if (!(gpu_base[VIRTR_DEV_STATUS_O] & VIRTIO_STATUS_FEATURES_OK) == VIRTIO_STATUS_FEATURES_OK) {
        // Device doesn't agree with our features, device is unusable.
        return -4;
    }

    // Setup virtual queues.

    // As described in 4.2.3.2 in Virtio documentation ver 1.2.
    gpu_base[VIRTR_QUEUE_SEL_O] = 0;
    if (gpu_base[VIRTR_QUEUE_READY_O] != 0x0) {
        // Queue is already in use.
        return -5;
    }

    uint64_t dev_max_queue_size = gpu_base[VIRTR_QUEUE_NUM_MAX_O]; // Get the max queue size.
    if  (dev_max_queue_size == 0) {
        // Queue is not available.
        return -6;
    }

    // Allocate queues and reset their memory.
    virt_queue *cmd_queue = kaligned_alloc(4096, sizeof(virt_queue));
    virt_queue *cursor_queue = kaligned_alloc(4096, sizeof(virt_queue));
    memset(cmd_queue, 0, sizeof(virt_queue));
    memset(cursor_queue, 0, sizeof(virt_queue));

    // Notify device of queue size
    gpu_base[VIRTR_QUEUE_NUM_O] = VIRTQ_QUEUE_SIZE;

    /* 
    *  Tell device where the data is for the given queue (control).
    *  These registers are technically high-low, BUT both systems are little endian, so just write the full 64-bit word. 
    */
   gpu_base[VIRTR_QUEUE_DESC_O] = (uint64_t) &cmd_queue -> buffers;
   gpu_base[VIRTR_QUEUE_DRV_O] =  (uint64_t) &cmd_queue -> available.ring;
   gpu_base[VIRTR_QUEUE_DEV_O] =  (uint64_t) &cmd_queue -> used.ring;

   // Tell device we're ready!
   gpu_base[VIRTR_QUEUE_READY_O] = 0x1;

   return 0;
}

int32_t virtio_gpu_init_pci() {
    

    return 0;
}


