#include "misc.h"
#include <wincodec.h>

extern IWICImagingFactory* wic_factory;
extern const GUID wic_pixel_format;

int wic_init(void);
void wic_fini(void);
