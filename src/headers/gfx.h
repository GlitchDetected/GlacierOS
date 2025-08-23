#ifndef __GFX_H
#define __GFX_H

#include <stdint.h>
#include <graphics.h>

void gfx_blit(struct graphics_info* video, int x, int y, int width, int height, uint32_t* src);

#endif
