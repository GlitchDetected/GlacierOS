#include <kernel.h>
#include <stdbool.h>
#include <string.h>
#include <graphics.h>
#include <x86.h>

struct graphics_info graphics;

void draw_rect(uint32_t* framebuffer_pointer,
	const uint32_t pixels_per_scaline,
	const uint16_t _x,
	const uint16_t _y,
	const uint16_t width,
	const uint16_t height,
	const uint32_t color)
{
	/** Pointer to the current pixel in the buffer. */
	uint32_t* at;

	uint16_t row = 0;
	uint16_t col = 0;
	for(row = 0; row < height; row++) {
		for(col = 0; col < width; col++) {
			at = framebuffer_pointer + _x + col;
			at += ((_y + row) * pixels_per_scaline);

			*at = color;
		}
	}
}

void draw_pixel(uint32_t* framebuffer_pointer,
	const uint32_t pixels_per_scaline,
	const uint16_t _x,
	const uint16_t _y,
	const uint32_t color)
{
	/** Pointer to the current pixel in the buffer. */
	uint32_t* at = framebuffer_pointer + _x + (_y * pixels_per_scaline);
	*at = color;
}

uint32_t convert_rgb_to_32bit_colour(const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	return (r << 16) | (g << 8) | b;
}

/**
 * @brief Initializes graphics mode using the boot info provided by the bootloader.
 */
void init_graphics(boot_params_t* boot_info) {
    if (!boot_info->graphic_out_protocol.FrameBufferBase) {
        HALT_AND_CATCH_FIRE("Framebuffer pointer invalid! Kernel cannot continue.\n");
    }

    graphics.framebuffer = (uint32_t*) boot_info->graphic_out_protocol.FrameBufferBase;
    graphics.width       = boot_info->graphic_out_protocol.Info->HorizontalResolution;
    graphics.height      = boot_info->graphic_out_protocol.Info->VerticalResolution;
    graphics.pitch       = boot_info->graphic_out_protocol.Info->PixelsPerScanLine;
    graphics.bpp         = 32; // UEFI GOP defaults to 32-bit

    // Clear screen
    for (uint32_t y = 0; y < graphics.height; y++) {
        for (uint32_t x = 0; x < graphics.width; x++) {
            graphics.framebuffer[y * graphics.pitch + x] = 0x000000; // black
        }
    }
}