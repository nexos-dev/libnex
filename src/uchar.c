// Borrowed from musl. musl's license may be found at https://github.com/bminor/musl/blob/master/COPYRIGHT

#include <errno.h>
#include <uchar.h>
#include <wchar.h>

size_t c16rtomb (char* restrict s, char16_t c16, mbstate_t* restrict ps)
{
    static unsigned internal_state;
    if (!ps)
        ps = (void*) &internal_state;
    unsigned* x = (unsigned*) ps;
    wchar_t wc;

    if (!s)
    {
        if (*x)
            goto ilseq;
        return 1;
    }

    if (!*x && c16 - 0xd800u < 0x400)
    {
        *x = c16 - 0xd7c0 << 10;
        return 0;
    }

    if (*x)
    {
        if (c16 - 0xdc00u >= 0x400)
            goto ilseq;
        else
            wc = *x + c16 - 0xdc00;
        *x = 0;
    }
    else
    {
        wc = c16;
    }
    return wcrtomb (s, wc, 0);

ilseq:
    *x = 0;
    errno = EILSEQ;
    return -1;
}

size_t c32rtomb (char* restrict s, char32_t c32, mbstate_t* restrict ps)
{
    return wcrtomb (s, c32, ps);
}

size_t mbrtoc32 (char32_t* restrict pc32, const char* restrict s, size_t n, mbstate_t* restrict ps)
{
    static unsigned internal_state;
    if (!ps)
        ps = (void*) &internal_state;
    if (!s)
        return mbrtoc32 (0, "", 1, ps);
    wchar_t wc;
    size_t ret = mbrtowc (&wc, s, n, ps);
    if (ret <= 4 && pc32)
        *pc32 = wc;
    return ret;
}

size_t mbrtoc16 (char16_t* restrict pc16, const char* restrict s, size_t n, mbstate_t* restrict ps)
{
    static unsigned internal_state;
    if (!ps)
        ps = (void*) &internal_state;
    unsigned* pending = (unsigned*) ps;

    if (!s)
        return mbrtoc16 (0, "", 1, ps);

    /* mbrtowc states for partial UTF-8 characters have the high bit set;
     * we use nonzero states without high bit for pending surrogates. */
    if ((int) *pending > 0)
    {
        if (pc16)
            *pc16 = *pending;
        *pending = 0;
        return -3;
    }

    wchar_t wc;
    size_t ret = mbrtowc (&wc, s, n, ps);
    if (ret <= 4)
    {
        if (wc >= 0x10000)
        {
            *pending = (wc & 0x3ff) + 0xdc00;
            wc = 0xd7c0 + (wc >> 10);
        }
        if (pc16)
            *pc16 = wc;
    }
    return ret;
}