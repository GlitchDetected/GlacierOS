#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <stdint.h>
#include <bootinfo.h>
#include <kernel.h>

struct graphics_info {
    uint32_t* framebuffer;
    uint32_t  pitch; // pixels per scanline
    uint32_t  width;
    uint32_t  height;
    uint32_t  bpp;
};

extern struct graphics_info graphics;

/**
 * @brief Draws a rectangle onto the framebuffer.
 * Draws a rectangle onto the video frame buffer.
 * @param[in] framebuffer_pointer A pointer to the video framebuffer.
 * @param[in] pixels_per_scaline The number of pixels per scanline. Also known as 'pitch'.
 * In some more exotic video modes this may be different to the visible screen width.
 * @param[in] _x The x coordinate to draw the rect to.
 * @param[in] _y The y coordinate to draw the rect to.
 * @param[in] width The width of the rectangle to draw.
 * @param[in] height The height of the rectangle to draw.
 * @param[in] color The color to draw.
 */
void draw_rect(uint32_t* framebuffer_pointer,
	const uint32_t pixels_per_scaline,
	const uint16_t _x,
	const uint16_t _y,
	const uint16_t width,
	const uint16_t height,
	const uint32_t color);

/**
 * @brief Paints a pixel of a certain colour onto the framebuffer.
 * @param[in] framebuffer_pointer A pointer to the video framebuffer.
 * @param[in] pixels_per_scaline The number of pixels per scanline. Also known as 'pitch'.
 * In some more exotic video modes this may be different to the visible screen width.
 * @param[in] _x The x coordinate to pixel.
 * @param[in] _y The y coordinate to pixel.
 * @param[in] color The color to draw.
 */
void draw_pixel(uint32_t* framebuffer_pointer,
	const uint32_t pixels_per_scaline,
	const uint16_t _x,
	const uint16_t _y,
	const uint32_t color);

/**
  * @brief Converts an RGB colour to a 32-bit colour suitable for using with
  * a framebuffer using the UEFI Graphics Output Protocol.
  * @param[in] r The red component.
  * @param[in] g The green component.
  * @param[in] b The blue component.
  * @return The colour encoded as a 32-bit integer.
  */
uint32_t convert_rgb_to_32bit_colour(const uint8_t r,
	const uint8_t g,
	const uint8_t b);

/**
 * Initialise framebuffer info from the bootloader Boot_Info.
 */
void init_graphics(boot_params_t *params);

#endif
