#include "misc.h"
#include "backend-d2d.h"
#include "backend-dwrite.h"
#include "backend-wic.h"
#include "backend-gdix.h"
#include "lock.h"

int
wdBackend(void)
{
    if(d2d_enabled()) {
        return WD_BACKEND_D2D;
    } 
    
    if(gdix_enabled()) {
        return WD_BACKEND_GDIPLUS;
    }
    
  return -1;
}
