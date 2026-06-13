/* ISC License
 * Copyright (c) 2018 Andrew Reece
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef blit32_H
#define blit32_H

#include <stdint.h>
#include <stddef.h>

#define blit32_WIDTH  5
#define blit32_HEIGHT 6
#define blit32_ADVANCE (blit32_WIDTH + 1)
#define blit32_DESCENDER 2
#define blit32_BASELINE_OFFSET (blit32_DESCENDER + 1)
#define blit32_ROW_ADVANCE (blit32_HEIGHT + blit32_BASELINE_OFFSET)
#define blit32_EXTRA_BITS(x) (((x) >> 30) & 3)

typedef uint32_t blit32_glyph;

#ifndef blit_H
#ifndef blit_pixel
#define blit_pixel uint32_t
#endif/*blit_pixel*/
#ifndef blit_NO_INLINE
#define blit_inline inline
#else /*blit_NO_INLINE*/
#define blit_inline
#endif/*blit_NO_INLINE*/
#define blit_NUM_GLYPHS 95
#define blit_IndexFromASCII(ascii) ((uint32_t)(ascii) - ' ')
#define blit_ASCIIFromIndex(index) ((char)((index) + ' '))

typedef struct blit_props
{
    blit_pixel *Buffer;
    blit_pixel  Value;
    int32_t     Scale;
    int32_t     BufWidth;
    int32_t     BufHeight;
    enum {blit_Clip, blit_Wrap} Wrap;
} blit_props;
#define blit_H
#endif/*blit_H*/

#ifndef blit32_ARRAY_ONLY
#define blit32_Glyphs Blit32.Glyphs
typedef struct blit32_font
{
    const blit32_glyph Glyphs[blit_NUM_GLYPHS];
    const uint32_t Width;
    const uint32_t Height;
    const uint32_t Descender;
    const uint32_t Advance;
    const uint32_t RowAdvance;
            blit_props Props;
} blit32_font;

#define blit32_SCALE(Font, scale) Font->Props.Scale = scale;       \
    *(uint32_t *)&Font->Width      = blit32_WIDTH       * scale; \
    *(uint32_t *)&Font->Height     = blit32_HEIGHT      * scale; \
    *(uint32_t *)&Font->Descender  = blit32_DESCENDER   * scale; \
    *(uint32_t *)&Font->Advance    = blit32_ADVANCE     * scale; \
    *(uint32_t *)&Font->RowAdvance = blit32_ROW_ADVANCE * scale
#ifndef blit32_MACRO_INLINE
blit_inline void blit32_Scale(blit32_font *Font, int32_t Scale) { blit32_SCALE(Font, Scale); }
#else /*blit32_MACRO_INLINE*/
#define blit32_Scale(Font, scale) \
do { int32_t blit32_TempScale = (scale); blit32_SCALE((Font), blit32_TempScale); } while (0)
#endif/*blit32_MACRO_INLINE*/

blit32_font Blit32 = {
#else/*blit32_ARRAY_ONLY*/
blit32_glyph blit32_Glyphs[blit_NUM_GLYPHS] =
#endif/*blit32_ARRAY_ONLY*/
    {
        /* all chars up to 32 are non-printable */
        0x00000000,0x08021084,0x0000294a,0x15f52bea,0x08fa38be,0x33a22e60,0x2e94d8a6,0x00001084,
        0x10421088,0x04421082,0x00a23880,0x00471000,0x04420000,0x00070000,0x0c600000,0x02222200,
        0x1d3ad72e,0x3e4214c4,0x3e22222e,0x1d18320f,0x210fc888,0x1d183c3f,0x1d17844c,0x0222221f,
        0x1d18ba2e,0x210f463e,0x0c6018c0,0x04401000,0x10411100,0x00e03800,0x04441040,0x0802322e,
        0x3c1ef62e,0x231fc544,0x1f18be2f,0x3c10862e,0x1f18c62f,0x3e10bc3f,0x0210bc3f,0x1d1c843e,
        0x2318fe31,0x3e42109f,0x0c94211f,0x23149d31,0x3e108421,0x231ad6bb,0x239cd671,0x1d18c62e,
        0x0217c62f,0x30eac62e,0x2297c62f,0x1d141a2e,0x0842109f,0x1d18c631,0x08454631,0x375ad631,
        0x22a21151,0x08421151,0x3e22221f,0x1842108c,0x20820820,0x0c421086,0x00004544,0xbe000000,
        0x00000082,0x1c97b000,0x0e949c21,0x1c10b800,0x1c94b908,0x3c1fc5c0,0x42211c4c,0x4e87252e,
        0x12949c21,0x0c210040,0x8c421004,0x12519521,0x0c210842,0x235aac00,0x12949c00,0x0c949800,
        0x4213a526,0x7087252e,0x02149800,0x0e837000,0x0c213c42,0x0e94a400,0x0464a400,0x155ac400,
        0x36426c00,0x4e872529,0x1e223c00,0x1843188c,0x08421084,0x0c463086,0x0006d800,
#ifndef blit32_ARRAY_ONLY
    },
    blit32_WIDTH,
    blit32_HEIGHT,
    blit32_DESCENDER,
    blit32_ADVANCE,
    blit32_ROW_ADVANCE,
    /* Default props: no buffer, zero value, scale 1, zero dimensions, clipping wrap */
    { NULL, 0, 1, 0, 0, blit_Clip },
#endif/*blit32_ARRAY_ONLY*/
};

int32_t blit32_TextNExplicit(blit_pixel *Buffer, blit_pixel Value, int32_t Scale, int32_t BufWidth, int32_t BufHeight, int32_t Wrap, int32_t StartX, int32_t StartY, int32_t StrLen, char *String)
{
    int32_t IsNegative = BufWidth < 0;
    int32_t DrawDir = IsNegative ? -1 : 1;
    int32_t AbsBufWidth = DrawDir * BufWidth;
    uint32_t LinesPrinted = 1;
    size_t i;
    int32_t x, y;
    
    StrLen = StrLen >= 0 ? StrLen : 0x7FFFFFFF;
    for(i = 0, x = StartX, y = StartY; String[i] && i < (size_t)StrLen; ++i)
    {
        char c = String[i];
        int32_t EndY = y + (blit32_HEIGHT + blit32_DESCENDER) * Scale * DrawDir;
        int32_t BufUnderflow  = EndY < 0 || y < 0;
        int32_t BufOverflow   = BufHeight && (EndY >= BufHeight || y >= BufHeight);
        int32_t BufYMinExceed = BufUnderflow || (IsNegative && BufOverflow);
        int32_t BufYMaxExceed = BufOverflow  || (IsNegative && BufUnderflow);
        int32_t BufXMinExceed = x < 0;

        int32_t CharWidth = (c == '\t') ? 4 * Scale * blit32_ADVANCE : Scale * blit32_WIDTH;
        int32_t BufXMaxExceed = (x + CharWidth) > AbsBufWidth;

        if(BufYMaxExceed) { break; }
        
        if (c == '\n' || c == '\r') {
            BufXMaxExceed = 0;
        }

        if((! BufYMinExceed && (! BufXMaxExceed || Wrap)) || c == '\n' || c == '\r')
        {
            if(BufXMaxExceed && c != '\n' && c != '\r' && c != '\t' && c != ' ') { 
                c = '\n'; 
                --i; 
            }
            else if(BufXMinExceed && c != '\n' && c != '\r') { 
                c = ' '; 
            }

            switch(c)
            {
                default:
                {
                    uint32_t glY, pxY, glX, pxX;
                    blit32_glyph Glyph = blit32_Glyphs[blit_IndexFromASCII(c)];
                    
                    int32_t DesiredOffset = (blit32_EXTRA_BITS(Glyph) > 0) ? (int32_t)blit32_DESCENDER : 0;
                    uint32_t OffsetY = (uint32_t)(y + (DesiredOffset * Scale * DrawDir));
                    
                    blit_pixel *Pixel, *Row = Buffer + OffsetY * (uint32_t)AbsBufWidth + x;
                    
                    for (glY = 0; glY < blit32_HEIGHT; ++glY)
                    {
                        for (pxY = (uint32_t)Scale; pxY--; Row += BufWidth)
                        {
                            Pixel = Row;
                            for (glX = 0; glX < blit32_WIDTH; ++glX)
                            {
                                uint32_t Shift = glY * blit32_WIDTH + glX;
                                if ((Glyph >> Shift) & 1)
                                {
                                    for (pxX = (uint32_t)Scale; pxX--; *Pixel++ = Value);
                                }
                                else
                                {
                                    Pixel += Scale;
                                }
                            }
                        }
                    }
                    x += Scale * blit32_ADVANCE;
                    break;
                }
                case  ' ':
                {
                    uint32_t sY, pY, pX;
                    // Uso la stessa identica formula del blocco default per l'inidirizzamento lineare della riga
                    blit_pixel *Pixel, *Row = Buffer + (uint32_t)y * (uint32_t)AbsBufWidth + x;
                    for(sY = 0; sY < (uint32_t)(blit32_HEIGHT + blit32_DESCENDER); ++sY)
                        for(pY = (uint32_t)Scale; pY--; Row += BufWidth)
                            for(Pixel = Row, pX = (uint32_t)(Scale * blit32_ADVANCE); pX--; *Pixel++ = 0);

                    x += Scale * blit32_ADVANCE;
                    break;
                }
                case '\b': x -=           Scale * blit32_ADVANCE; break;
                case '\t':
                {
                    uint32_t sY, pY, pX;
                    // Forza il calcolo della riga partendo dall'indirizzo base puro + x corrente
                    blit_pixel *Pixel, *Row = Buffer + (uint32_t)y * (uint32_t)AbsBufWidth + x;
                    for(sY = 0; sY < (uint32_t)(blit32_HEIGHT + blit32_DESCENDER); ++sY)
                        for(pY = (uint32_t)Scale; pY--; Row += BufWidth)
                            for(Pixel = Row, pX = (uint32_t)(4 * Scale * blit32_ADVANCE); pX--; *Pixel++ = 0);

                    x += 4 * Scale * blit32_ADVANCE;
                    break;
                }
                case '\n': y += DrawDir * Scale * blit32_ROW_ADVANCE; ++LinesPrinted; __attribute__((fallthrough));
                case '\r': x  = StartX;                        break;
            }
        }
    }
    return (int32_t)LinesPrinted;
}

#ifndef blit32_NO_HELPERS
#ifndef blit32_MACRO_INLINE

blit_inline int32_t blit32_TextExplicit(blit_pixel *Buffer, blit_pixel Value, int32_t Scale, int32_t BufWidth, int32_t BufHeight, int32_t Wrap, int32_t StartX, int32_t StartY, char *String)
{ return blit32_TextNExplicit(Buffer, Value, Scale, BufWidth, BufHeight, Wrap, StartX, StartY, -1, String); }
blit_inline int32_t blit32_TextNProps(blit_props Props, int32_t StartX, int32_t StartY, int32_t StrLen, char *String)
{ return blit32_TextNExplicit(Props.Buffer, Props.Value, Props.Scale, Props.BufWidth, Props.BufHeight, Props.Wrap, StartX, StartY, StrLen, String); }
blit_inline int32_t blit32_TextProps(blit_props Props, int32_t StartX, int32_t StartY, char *String)
{ return blit32_TextNExplicit(Props.Buffer, Props.Value, Props.Scale, Props.BufWidth, Props.BufHeight, Props.Wrap, StartX, StartY, -1, String); }
blit_inline int32_t blit32_TextN(int32_t StartX, int32_t StartY, int32_t StrLen, char *String)
{ return blit32_TextNExplicit(Blit32.Props.Buffer, Blit32.Props.Value, Blit32.Props.Scale, Blit32.Props.BufWidth, Blit32.Props.BufHeight, Blit32.Props.Wrap, StartX, StartY, StrLen, String); }
blit_inline int32_t blit32_Text(int32_t StartX, int32_t StartY, char *String)
{ return blit32_TextNExplicit(Blit32.Props.Buffer, Blit32.Props.Value, Blit32.Props.Scale, Blit32.Props.BufWidth, Blit32.Props.BufHeight, Blit32.Props.Wrap, StartX, StartY, -1, String); }

#else/*blit32_NO_INLINE*/

#define blit32_TextExplicit(Buffer, Value, Scale, BufWidth, BufHeight, Wrap, StartX, StartY, String) \
    blit32_TextNExplicit(Buffer, Value, Scale, BufWidth, BufHeight, Wrap, StartX, StartY, -1, String)
#define blit32_TextNProps(Props, StartX, StartY, StrLen, String) \
    blit32_TextNExplicit((Props).Buffer, (Props).Value, (Props).Scale, (Props).BufWidth, (Props).BufHeight, (Props).Wrap, StartX, StartY, StrLen, String)
#define blit32_TextProps(Props, StartX, StartY, String) \
    blit32_TextNExplicit((Props).Buffer, (Props).Value, (Props).Scale, (Props).BufWidth, (Props).BufHeight, (Props).Wrap, StartX, StartY, -1, String)
#define blit32_TextN(StartX, StartY, StrLen, String) \
    blit32_TextNExplicit(Blit32.Props.Buffer, Blit32.Props.Value, Blit32.Props.Scale, Blit32.Props.BufWidth, Blit32.Props.BufHeight, Blit32.Props.Wrap, StartX, StartY, StrLen, String)
#define blit32_Text(StartX, StartY, String) \
    blit32_TextNExplicit(Blit32.Props.Buffer, Blit32.Props.Value, Blit32.Props.Scale, Blit32.Props.BufWidth, Blit32.Props.BufHeight, Blit32.Props.Wrap, StartX, StartY, -1, String)

#endif/*blit32_NO_INLINE*/
#endif/*blit32_NO_HELPERS*/

#endif/*blit32_H*/