#ifndef SENS_HWSTATUS_H
#define SENS_HWSTATUS_H

// hardware status flags
#define HWstatus_core1 0x0002 // core1 is running
#define HWstatus_init  0x0001 // i2c devices initialized
#define HWstatus_wdrestart 0x0004 // restarted due to watchdog timeout
int get_hwstatus();
void update_hwstatus(int flag, bool set=true);
#endif // SENS_HWSTATUS_H