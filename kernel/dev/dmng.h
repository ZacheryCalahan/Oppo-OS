#ifndef DMNG_H
#define DMNG_H

#include <stdint.h>

enum device_interface {
    PCI,
    USB,
    MMIO,
};

enum device_class {
    DEV_CLASS_BLOCK,
    DEV_CLASS_CHAR,
    DEV_CLASS_NET,
};

typedef struct device_t {
    const char *name;                               // Human readable name of the device.
    const uint32_t dev_id;                          // Unique ID of the device.

    const enum device_class dev_class;              // The type of device, such as block, net, char, etc.
    const enum device_interface dev_interface;      // The bus interface, such as USB, PCI, or MMIO.

    uint32_t (*init_func)();                        // Init function.
    uint32_t (*remove_dev)(struct device_t *dev);   // Remove function
    uint16_t interrupt_number;                      // IRQ number

    uint32_t (*read)(struct device_t *dev, void *buf, uint32_t size, uint64_t offset);  // Read from the device
    uint32_t (*write)(struct device_t *dev, void *buf, uint32_t size, uint64_t offset); // Write to the device
    uint32_t (*io_ctl)(struct device_t *dev, uint32_t cmd, void *arg);                  // Control misc io

    uint64_t io_base;                               // MMIO or port base address
    uint64_t io_size;                               // Size of MMIO/Port memory
};

#endif