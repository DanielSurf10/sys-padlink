#ifndef DRAWING_H
# define DRAWING_H

//----------------------------------------------------------------------------//
//                      defines                      //
//----------------------------------------------------------------------------//

# ifndef REG_LCDBACKLIGHTMAIN
#  define REG_LCDBACKLIGHTMAIN (u32)(0x1ED02240 - 0x1EB00000)
# endif

# ifndef REG_LCDBACKLIGHTSUB
#  define REG_LCDBACKLIGHTSUB (u32)(0x1ED02A40 - 0x1EB00000)
# endif

# define drawPixelRGB(x, y, r, g, b) drawPixelRGBFramebuffer(0, x, y, r, g, b)
# define drawBox(x, y, width, height, r, g, b) drawBoxFramebuffer(0, x, y, width, height, r, g, b)
# define drawString(sx, sy, text, ...) drawStringFramebuffer(0, sx, sy, text, ##__VA_ARGS__)

//----------------------------------------------------------------------------//
//                     functions                     //
//----------------------------------------------------------------------------//

void clearScreen(void);
void drawPixelRGBFramebuffer(u8 *fb, int x, int y, u8 r, u8 g, u8 b);
void drawBoxFramebuffer(u8 *fb, int x, int y, int width, int height, u8 r, u8 g, u8 b);
void drawStringFramebuffer(u8 *fb, int sx, int sy, char *text, ...);

void disableBacklight();
void enableBacklight();

#endif
