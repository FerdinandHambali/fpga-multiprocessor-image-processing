//ID: 33931097, 32920474 and 33531153
#include <stdio.h>
#include "system.h"
#include "io.h"
#include "alt_types.h"

#define CAM_W 160
#define CAM_H 120
#define VGA_W 320

#define IMAGE_ORIGINAL 0
#define IMAGE_FLIPPED  1
#define IMAGE_BLURRED  2
#define IMAGE_EDGE     3

#define MODE_SINGLE 0
#define MODE_QUAD   1

#define BUFFER_EMPTY 0
#define BUFFER_READY 1
#define BUFFER_BUSY  2

#define DISPLAY_EMPTY 0
#define DISPLAY_READY 1
#define DISPLAY_BUSY  2

#define BENCHMARK_FRAMES 10

#define FRAME_BASE        0x04400000

#define ORIGINAL0_BASE    (FRAME_BASE + 0x00000)
#define ORIGINAL1_BASE    (FRAME_BASE + 0x08000)

#define BLURRED0_BASE     (FRAME_BASE + 0x10000)
#define BLURRED1_BASE     (FRAME_BASE + 0x18000)

#define EDGE0_BASE        (FRAME_BASE + 0x20000)
#define EDGE1_BASE        (FRAME_BASE + 0x28000)

#define FLIPPED0_BASE     (FRAME_BASE + 0x30000)
#define FLIPPED1_BASE     (FRAME_BASE + 0x38000)

#define CONTROL_BASE      0x04500000

#define BUF0_STATE_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x18, (v))
#define BUF1_STATE_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x1C, (v))

#define EDGE0_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x28, (v))
#define EDGE1_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x2C, (v))

#define DISPLAY_STATE_READ()       IORD_32DIRECT(CONTROL_BASE, 0x30)
#define DISPLAY_STATE_WRITE(v)     IOWR_32DIRECT(CONTROL_BASE, 0x30, (v))

#define DISPLAY_MODE_READ()        IORD_32DIRECT(CONTROL_BASE, 0x34)
#define DISPLAY_SINGLE_READ()      IORD_32DIRECT(CONTROL_BASE, 0x38)

#define DISPLAY_Q0_READ()          IORD_32DIRECT(CONTROL_BASE, 0x3C)
#define DISPLAY_Q1_READ()          IORD_32DIRECT(CONTROL_BASE, 0x40)
#define DISPLAY_Q2_READ()          IORD_32DIRECT(CONTROL_BASE, 0x44)
#define DISPLAY_Q3_READ()          IORD_32DIRECT(CONTROL_BASE, 0x48)

#define DISPLAY_ORIGINAL_ADDR_READ()   IORD_32DIRECT(CONTROL_BASE, 0x4C)
#define DISPLAY_BUFFER_INDEX_READ()    IORD_32DIRECT(CONTROL_BASE, 0x50)


#define FPS_AVG_FRAMES 16

static alt_u32 last_frame_done_us = 0;
static int throughput_ready = 0;

static alt_u32 benchmark_total_interval_us = 0;
static int benchmark_count = 0;
static int benchmark_active = 0;
static int benchmark_request = 1;

static int last_mode_printed = -1;
static int last_single_printed = -1;
static alt_u32 last_quad_printed = 0xFFFFFFFF;


static alt_u32 fps_interval_sum_us = 0;
static int fps_interval_count = 0;

static const char *image_name(int image)
{
    if (image == IMAGE_FLIPPED)
    {
        return "flipped";
    }
    else if (image == IMAGE_BLURRED)
    {
        return "blurred";
    }
    else if (image == IMAGE_EDGE)
    {
        return "edge";
    }
    else
    {
        return "original";
    }
}

static alt_u8 seven_seg_digit(int digit)
{
    switch (digit)
    {
        case 0: return 0xC0;
        case 1: return 0xF9;
        case 2: return 0xA4;
        case 3: return 0xB0;
        case 4: return 0x99;
        case 5: return 0x92;
        case 6: return 0x82;
        case 7: return 0xF8;
        case 8: return 0x80;
        case 9: return 0x90;
        default: return 0xFF;
    }
}

static void display_fps_x100(alt_u32 fps_x100)
{
    alt_u8 hex0;
    alt_u8 hex1;
    alt_u8 hex2;
    alt_u8 hex3;
    alt_u32 hex0_word;
    alt_u32 hex5_word;

    if (fps_x100 > 9999)
    {
        fps_x100 = 9999;
    }

    hex0 = seven_seg_digit(fps_x100 % 10);
    hex1 = seven_seg_digit((fps_x100 / 10) % 10);
    hex2 = seven_seg_digit((fps_x100 / 100) % 10);
    hex3 = seven_seg_digit((fps_x100 / 1000) % 10);

    hex2 = hex2 & 0x7F;

    hex0_word = ((alt_u32)hex2 << 16) |
                ((alt_u32)hex1 << 8) |
                (alt_u32)hex0;

    hex5_word = ((alt_u32)0xFF << 16) |
                ((alt_u32)0xFF << 8) |
                (alt_u32)hex3;

    IOWR(HEX0_BASE, 0, hex0_word);
    IOWR(HEX5_BASE, 0, hex5_word);
}

static inline void write_pixel_hw(int addr, alt_u8 pixel)
{
    IOWR(PIXEL_SEND_BASE, 0, addr);
    IOWR(PIXEL_OUTPUT_BASE, 0, pixel & 0x0F);
    IOWR(PIXEL_WE_BASE, 0, 1);
    IOWR(PIXEL_WE_BASE, 0, 0);
}

static inline void write_2x2_block(int x, int y, alt_u8 pixel)
{
    int row0;
    int row1;

    row0 = y * VGA_W;
    row1 = (y + 1) * VGA_W;

    write_pixel_hw(row0 + x,     pixel);
    write_pixel_hw(row0 + x + 1, pixel);
    write_pixel_hw(row1 + x,     pixel);
    write_pixel_hw(row1 + x + 1, pixel);
}

static void display_quadrant_unpacked(alt_u8 *image, int x_offset, int y_offset)
{
    int x;
    int y;
    int addr;
    alt_u8 pixel;

    for (y = 0; y < CAM_H; y++)
    {
        for (x = 0; x < CAM_W; x++)
        {
            pixel = image[y * CAM_W + x];
            addr = (y_offset + y) * VGA_W + (x_offset + x);
            write_pixel_hw(addr, pixel);
        }
    }
}

static void display_single_unpacked(alt_u8 *image)
{
    int x;
    int y;
    int dx;
    int dy;
    alt_u8 pixel;

    for (y = 0; y < CAM_H; y++)
    {
        dy = y * 2;

        for (x = 0; x < CAM_W; x++)
        {
            pixel = image[y * CAM_W + x];
            dx = x * 2;

            write_2x2_block(dx, dy, pixel);
        }
    }
}

static alt_u8 *select_image_buffer(
    int image_choice,
    alt_u8 *original,
    alt_u8 *flipped,
    alt_u8 *blurred,
    alt_u8 *edge
)
{
    if (image_choice == IMAGE_FLIPPED)
    {
        return flipped;
    }
    else if (image_choice == IMAGE_BLURRED)
    {
        return blurred;
    }
    else if (image_choice == IMAGE_EDGE)
    {
        return edge;
    }
    else
    {
        return original;
    }
}

static void get_image_bank(
    int buffer_index,
    alt_u8 **flipped,
    alt_u8 **blurred,
    alt_u8 **edge
)
{
    if (buffer_index == 0)
    {
        *flipped = (alt_u8 *) FLIPPED0_BASE;
        *blurred = (alt_u8 *) BLURRED0_BASE;
        *edge = (alt_u8 *) EDGE0_BASE;
    }
    else
    {
        *flipped = (alt_u8 *) FLIPPED1_BASE;
        *blurred = (alt_u8 *) BLURRED1_BASE;
        *edge = (alt_u8 *) EDGE1_BASE;
    }
}

static void display_quad_processed(
    alt_u8 *original,
    alt_u8 *flipped,
    alt_u8 *blurred,
    alt_u8 *edge
)
{
    alt_u8 *img0;
    alt_u8 *img1;
    alt_u8 *img2;
    alt_u8 *img3;

    img0 = select_image_buffer(DISPLAY_Q0_READ(), original, flipped, blurred, edge);
    img1 = select_image_buffer(DISPLAY_Q1_READ(), original, flipped, blurred, edge);
    img2 = select_image_buffer(DISPLAY_Q2_READ(), original, flipped, blurred, edge);
    img3 = select_image_buffer(DISPLAY_Q3_READ(), original, flipped, blurred, edge);

    display_quadrant_unpacked(img0, 0,   0);
    display_quadrant_unpacked(img1, 160, 0);
    display_quadrant_unpacked(img2, 0,   120);
    display_quadrant_unpacked(img3, 160, 120);
}

static void display_single_processed(
    alt_u8 *original,
    alt_u8 *flipped,
    alt_u8 *blurred,
    alt_u8 *edge
)
{
    alt_u8 *selected;

    selected = select_image_buffer(
        DISPLAY_SINGLE_READ(),
        original,
        flipped,
        blurred,
        edge
    );

    display_single_unpacked(selected);
}

static void release_buffer(int buffer_index)
{
    if (buffer_index == 0)
    {
        BUF0_STATE_WRITE(BUFFER_EMPTY);
        EDGE0_STATE_WRITE(BUFFER_EMPTY);
    }
    else
    {
        BUF1_STATE_WRITE(BUFFER_EMPTY);
        EDGE1_STATE_WRITE(BUFFER_EMPTY);
    }
}

static int update_throughput_benchmark(
    int mode,
    int single_choice,
    alt_u32 quad_value,
    alt_u32 interval_us
)
{
    if (benchmark_request)
    {
        benchmark_total_interval_us = 0;
        benchmark_count = 0;
        benchmark_active = 1;
        benchmark_request = 0;
    }

    if (!benchmark_active)
    {
        return 0;
    }

    benchmark_total_interval_us += interval_us;
    benchmark_count++;

    if (benchmark_count >= BENCHMARK_FRAMES)
    {
        alt_u32 average_interval_us;
        alt_u32 average_fps_x100;

        average_interval_us = benchmark_total_interval_us / BENCHMARK_FRAMES;

        if (average_interval_us != 0)
        {
            average_fps_x100 =
                (100000000UL + (average_interval_us / 2)) / average_interval_us;
        }
        else
        {
            average_fps_x100 = 0;
        }

        if (mode == MODE_SINGLE)
        {
            printf("BENCHMARK RESULT: SINGLE %s | Time = %lu us | FPS = %lu.%02lu\n",
                   image_name(single_choice),
                   average_interval_us,
                   average_fps_x100 / 100,
                   average_fps_x100 % 100);
        }
        else
        {
            printf("BENCHMARK RESULT: QUAD Q0=%d Q1=%d Q2=%d Q3=%d | Time = %lu us | FPS = %lu.%02lu\n",
                   (int)((quad_value >> 0) & 0x3),
                   (int)((quad_value >> 2) & 0x3),
                   (int)((quad_value >> 4) & 0x3),
                   (int)((quad_value >> 6) & 0x3),
                   average_interval_us,
                   average_fps_x100 / 100,
                   average_fps_x100 % 100);
        }

        benchmark_total_interval_us = 0;
        benchmark_count = 0;
        benchmark_active = 0;

        return 1;
    }

    return 0;
}

int main(void)
{
    int buffer_index;
    int mode;
    int single_choice;
    alt_u32 quad_value;

    alt_u8 *original;
    alt_u8 *flipped;
    alt_u8 *blurred;
    alt_u8 *edge;

    alt_u32 end_us;
    alt_u32 interval_us;
    alt_u32 fps_x100;

    printf("data_transfer_proc running\n");

    while (1)
    {
        if (DISPLAY_STATE_READ() != DISPLAY_READY)
        {
            continue;
        }

        DISPLAY_STATE_WRITE(DISPLAY_BUSY);

        buffer_index = (int)DISPLAY_BUFFER_INDEX_READ();
        original = (alt_u8 *)DISPLAY_ORIGINAL_ADDR_READ();

        get_image_bank(buffer_index, &flipped, &blurred, &edge);

        mode = (int)DISPLAY_MODE_READ();
        single_choice = (int)DISPLAY_SINGLE_READ();

        quad_value =
            ((alt_u32)DISPLAY_Q0_READ() << 0) |
            ((alt_u32)DISPLAY_Q1_READ() << 2) |
            ((alt_u32)DISPLAY_Q2_READ() << 4) |
            ((alt_u32)DISPLAY_Q3_READ() << 6);

        if (mode == MODE_QUAD)
        {
            display_quad_processed(
                original,
                flipped,
                blurred,
                edge
            );
        }
        else
        {
            display_single_processed(
                original,
                flipped,
                blurred,
                edge
            );
        }

        end_us = IORD(COUNTER_BASE, 0);

        if (throughput_ready)
        {
        	interval_us = end_us - last_frame_done_us;

        	/*
        	 * Do not display FPS from only one frame.
        	 * Edge mode can naturally alternate between waiting and not waiting,
        	 * so we average several frame intervals to make the HEX display stable.
        	 */
        	if (interval_us != 0)
        	{
        	    fps_interval_sum_us += interval_us;
        	    fps_interval_count++;

        	    if (fps_interval_count >= FPS_AVG_FRAMES)
        	    {
        	        alt_u32 average_interval_us;

        	        average_interval_us = fps_interval_sum_us / fps_interval_count;

        	        if (average_interval_us != 0)
        	        {
        	            fps_x100 =
        	                (100000000UL + (average_interval_us / 2)) / average_interval_us;
        	        }
        	        else
        	        {
        	            fps_x100 = 0;
        	        }

        	        display_fps_x100(fps_x100);

        	        fps_interval_sum_us = 0;
        	        fps_interval_count = 0;
        	    }
        	}

            if (mode == MODE_SINGLE)
            {
                if (last_mode_printed != MODE_SINGLE ||
                    last_single_printed != single_choice)
                {
                    benchmark_request = 1;
                    last_mode_printed = MODE_SINGLE;
                    last_single_printed = single_choice;
                }
            }
            else
            {
                if (last_mode_printed != MODE_QUAD ||
                    last_quad_printed != quad_value)
                {
                    benchmark_request = 1;
                    last_mode_printed = MODE_QUAD;
                    last_quad_printed = quad_value;
                }
            }

            if (update_throughput_benchmark(
                    mode,
                    single_choice,
                    quad_value,
                    interval_us
                ))
            {
                throughput_ready = 0;
            }
        }

        last_frame_done_us = end_us;
        throughput_ready = 1;

        release_buffer(buffer_index);

        DISPLAY_STATE_WRITE(DISPLAY_EMPTY);
    }

    return 0;
}
