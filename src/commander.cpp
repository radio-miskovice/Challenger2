#include "commander.h"

void Commander::activate() { 
    status.active = 1 ; 
}
void Commander::deactivate() { status.active = 0 ;}
bool Commander::isActive() { return !!(status.active); }

Commander commander ;