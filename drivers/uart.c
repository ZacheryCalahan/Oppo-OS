#include "headers/uart.h"

void uart_init() {
    // Disable interrupts so we can set up uart settings
    uart_write(INTERRUPT_ENABLE_REGISTER, 0x00);

    // Enter baud rate mode
    uart_write(LINE_CONTROL_REGISTER, LINE_CONTROL_REGISTER_BAUD_LATCH);

    // Set baud rate to 38.4K
    uart_write(0, 0x01);
    uart_write(1, 0x00);

    // Leave baud rate mode
    uart_write(LINE_CONTROL_REGISTER, LINE_CONTROL_REGISTER_EIGHT_BITS_NO_PARITY);

    // Enable and clear FIFO
    uart_write(
        FIFO_CONTROL_REGISTER,
        FIFO_CONTROL_REGISTER_ENABLE |
        FIFO_CONTROL_REGISTER_CLEAR_TX |
        FIFO_CONTROL_REGISTER_CLEAR_RX
    );

    // Re-enable interrupts
    uart_write(INTERRUPT_ENABLE_REGISTER,
        INTERRUPT_ENABLE_REGISTER_RX_ENABLE |
        INTERRUPT_ENABLE_REGISTER_TX_ENABLE
    );
}

void uart_transmit(const char c) {
    uart_write(0, c);
}

char uart_receive() {
    return uart_read(0);
}