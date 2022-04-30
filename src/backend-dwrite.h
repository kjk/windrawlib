#include "misc.h"
#include "dummy/dwrite.h"

extern dummy_IDWriteFactory* dwrite_factory;

typedef struct dwrite_font_tag dwrite_font_t;
struct dwrite_font_tag {
    dummy_IDWriteTextFormat* tf;
    dummy_DWRITE_FONT_METRICS metrics;
};

dummy_IDWriteTextFormat* dwrite_create_text_format(const WCHAR* locale_name,
            const LOGFONTW* logfont, dummy_DWRITE_FONT_METRICS* metrics);

dummy_IDWriteTextLayout* dwrite_create_text_layout(dummy_IDWriteTextFormat* tf,
            const WD_RECT* rect, const WCHAR* str, int len, DWORD flags);

