#include "rtc.h"
#include "../../klib/stdio.h"

#define RTC_ADDRESS 0x101000

void init_rtc() {
    uint32_t *rtc = (void *) RTC_ADDRESS;
    
    // Ensure device exists
    printf("RTC ID: %d\n", rtc[3]);
}