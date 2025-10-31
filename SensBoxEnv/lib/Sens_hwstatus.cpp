#include "Sens_hwstatus.h"
long HWstatus=0; // hardware status
void update_hwstatus(int flag, bool set) {
    if (set) {
        HWstatus |= flag;
    } else {
        HWstatus &= ~flag;
    }
}   

int get_hwstatus() {
    return  HWstatus;
}