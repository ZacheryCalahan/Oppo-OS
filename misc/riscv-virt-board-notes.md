# What is this?
This is a markdown file that is for reference when building the OS. This contains information pertinent to the current task in development.

# Virt board features
- 1 generic PCIe host bridge
- fw_cfg device to get info from QEMU. This appears to be at address 0x0000000010100000. This is via MMIO.
- dtb that is passed to kernel.