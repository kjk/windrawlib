#include "backend-dwrite.h"


dummy_IDWriteTextFormat*
dwrite_create_text_format(const WCHAR* locale_name, const LOGFONTW* logfont,
                          dummy_DWRITE_FONT_METRICS* metrics)
{
    /* See https://github.com/Microsoft/Windows-classic-samples/blob/master/Samples/Win7Samples/multimedia/DirectWrite/RenderTest/TextHelpers.cpp */

    dummy_IDWriteTextFormat* tf = NULL;
    dummy_IDWriteGdiInterop* gdi_interop;
    dummy_IDWriteFont* font;
    dummy_IDWriteFontFamily* family;
    dummy_IDWriteLocalizedStrings* family_names;
    UINT32 family_name_buffer_size;
    WCHAR* family_name_buffer;
    float font_size;
    HRESULT hr;

    hr = dummy_IDWriteFactory_GetGdiInterop(dwrite_factory, &gdi_interop);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteFactory::GetGdiInterop() failed.");
        goto err_IDWriteFactory_GetGdiInterop;
    }

    hr = dummy_IDWriteGdiInterop_CreateFontFromLOGFONT(gdi_interop, logfont, &font);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteGdiInterop::CreateFontFromLOGFONT() failed.");
        goto err_IDWriteGdiInterop_CreateFontFromLOGFONT;
    }

    dummy_IDWriteFont_GetMetrics(font, metrics);

    hr = dummy_IDWriteFont_GetFontFamily(font, &family);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteFont::GetFontFamily() failed.");
        goto err_IDWriteFont_GetFontFamily;
    }

    hr = dummy_IDWriteFontFamily_GetFamilyNames(family, &family_names);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteFontFamily::GetFamilyNames() failed.");
        goto err_IDWriteFontFamily_GetFamilyNames;
    }

    hr = dummy_IDWriteLocalizedStrings_GetStringLength(family_names, 0, &family_name_buffer_size);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteLocalizedStrings::GetStringLength() failed.");
        goto err_IDWriteLocalizedStrings_GetStringLength;
    }

    family_name_buffer = (WCHAR*) _malloca(sizeof(WCHAR) * (family_name_buffer_size + 1));
    if(family_name_buffer == NULL) {
        WD_TRACE("dwrite_create_text_format: _malloca() failed.");
        goto err_malloca;
    }

    hr = dummy_IDWriteLocalizedStrings_GetString(family_names, 0,
            family_name_buffer, family_name_buffer_size + 1);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteLocalizedStrings::GetString() failed.");
        goto err_IDWriteLocalizedStrings_GetString;
    }

    if(logfont->lfHeight < 0) {
        font_size = (float) -logfont->lfHeight;
    } else if(logfont->lfHeight > 0) {
        font_size = (float)logfont->lfHeight * (float)metrics->designUnitsPerEm
                    / (float)(metrics->ascent + metrics->descent);
    } else {
        font_size = 12.0f;
    }

    hr = dummy_IDWriteFactory_CreateTextFormat(dwrite_factory, family_name_buffer,
            NULL, dummy_IDWriteFont_GetWeight(font), dummy_IDWriteFont_GetStyle(font),
            dummy_IDWriteFont_GetStretch(font), font_size, locale_name, &tf);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_format: "
                    "IDWriteFactory::CreateTextFormat() failed.");
        goto err_IDWriteFactory_CreateTextFormat;
    }

err_IDWriteFactory_CreateTextFormat:
err_IDWriteLocalizedStrings_GetString:
    _freea(family_name_buffer);
err_malloca:
err_IDWriteLocalizedStrings_GetStringLength:
    dummy_IDWriteLocalizedStrings_Release(family_names);
err_IDWriteFontFamily_GetFamilyNames:
    dummy_IDWriteFontFamily_Release(family);
err_IDWriteFont_GetFontFamily:
    dummy_IDWriteFont_Release(font);
err_IDWriteGdiInterop_CreateFontFromLOGFONT:
    dummy_IDWriteGdiInterop_Release(gdi_interop);
err_IDWriteFactory_GetGdiInterop:
    return tf;
}

dummy_IDWriteTextLayout*
dwrite_create_text_layout(dummy_IDWriteTextFormat* tf, const WD_RECT* rect,
                          const WCHAR* str, int len, DWORD flags)
{
    dummy_IDWriteTextLayout* layout;
    HRESULT hr;
    int tla;

    if(len < 0)
        len = wcslen(str);

    hr = dummy_IDWriteFactory_CreateTextLayout(dwrite_factory, str, len, tf,
                rect->x1 - rect->x0, rect->y1 - rect->y0, &layout);
    if(FAILED(hr)) {
        WD_TRACE_HR("dwrite_create_text_layout: "
                    "IDWriteFactory::CreateTextLayout() failed.");
        return NULL;
    }

    if(flags & WD_STR_RIGHTALIGN)
        tla = dummy_DWRITE_TEXT_ALIGNMENT_TRAILING;
    else if(flags & WD_STR_CENTERALIGN)
        tla = dummy_DWRITE_TEXT_ALIGNMENT_CENTER;
    else
        tla = dummy_DWRITE_TEXT_ALIGNMENT_LEADING;
    dummy_IDWriteTextLayout_SetTextAlignment(layout, tla);

    if(flags & WD_STR_BOTTOMALIGN)
        tla = dummy_DWRITE_PARAGRAPH_ALIGNMENT_FAR;
    else if(flags & WD_STR_MIDDLEALIGN)
        tla = dummy_DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    else
        tla = dummy_DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
    dummy_IDWriteTextLayout_SetParagraphAlignment(layout, tla);

    if(flags & WD_STR_NOWRAP)
        dummy_IDWriteTextLayout_SetWordWrapping(layout, dummy_DWRITE_WORD_WRAPPING_NO_WRAP);

    if((flags & WD_STR_ELLIPSISMASK) != 0) {
        static const dummy_DWRITE_TRIMMING trim_end = { dummy_DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
        static const dummy_DWRITE_TRIMMING trim_word = { dummy_DWRITE_TRIMMING_GRANULARITY_WORD, 0, 0 };
        static const dummy_DWRITE_TRIMMING trim_path = { dummy_DWRITE_TRIMMING_GRANULARITY_WORD, L'\\', 1 };

        const dummy_DWRITE_TRIMMING* trim_options = NULL;
        dummy_IDWriteInlineObject* trim_sign;

        hr = dummy_IDWriteFactory_CreateEllipsisTrimmingSign(dwrite_factory, tf, &trim_sign);
        if(FAILED(hr)) {
            WD_TRACE_HR("dwrite_create_text_layout: "
                        "IDWriteFactory::CreateEllipsisTrimmingSign() failed.");
            goto err_CreateEllipsisTrimmingSign;
        }

        switch(flags & WD_STR_ELLIPSISMASK) {
            case WD_STR_ENDELLIPSIS:    trim_options = &trim_end; break;
            case WD_STR_WORDELLIPSIS:   trim_options = &trim_word; break;
            case WD_STR_PATHELLIPSIS:   trim_options = &trim_path; break;
        }

        if(trim_options != NULL)
            dummy_IDWriteTextLayout_SetTrimming(layout, trim_options, trim_sign);

        dummy_IDWriteInlineObject_Release(trim_sign);
    }

err_CreateEllipsisTrimmingSign:
    return layout;
}
