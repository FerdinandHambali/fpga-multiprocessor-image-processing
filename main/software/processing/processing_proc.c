//ID: 33931097, 32920474 and 33531153
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "io.h"

#define CAM_W 160
#define CAM_H 120

#define IMAGE_ORIGINAL 0
#define IMAGE_FLIPPED  1
#define IMAGE_BLURRED  2
#define IMAGE_EDGE     3

#define MODE_SINGLE 0
#define MODE_QUAD   1

#define BUFFER_EMPTY 0
#define BUFFER_READY 1
#define BUFFER_BUSY  2

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

#define SHARED_MODE_READ()        IORD_32DIRECT(CONTROL_BASE, 0x00)
#define SHARED_MODE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x00, (v))

#define SHARED_SINGLE_READ()      IORD_32DIRECT(CONTROL_BASE, 0x04)

#define SHARED_Q0_READ()          IORD_32DIRECT(CONTROL_BASE, 0x08)
#define SHARED_Q0_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x08, (v))

#define SHARED_Q1_READ()          IORD_32DIRECT(CONTROL_BASE, 0x0C)
#define SHARED_Q1_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x0C, (v))

#define SHARED_Q2_READ()          IORD_32DIRECT(CONTROL_BASE, 0x10)
#define SHARED_Q2_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x10, (v))

#define SHARED_Q3_READ()          IORD_32DIRECT(CONTROL_BASE, 0x14)
#define SHARED_Q3_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x14, (v))

#define BUF0_STATE_READ()         IORD_32DIRECT(CONTROL_BASE, 0x18)
#define BUF0_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x18, (v))

#define BUF1_STATE_READ()         IORD_32DIRECT(CONTROL_BASE, 0x1C)
#define BUF1_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x1C, (v))

#define EDGE0_STATE_READ()        IORD_32DIRECT(CONTROL_BASE, 0x28)
#define EDGE0_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x28, (v))

#define EDGE1_STATE_READ()        IORD_32DIRECT(CONTROL_BASE, 0x2C)
#define EDGE1_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x2C, (v))

#define DISPLAY_EMPTY 0
#define DISPLAY_READY 1
#define DISPLAY_BUSY  2

#define DISPLAY_STATE_READ()          IORD_32DIRECT(CONTROL_BASE, 0x30)
#define DISPLAY_STATE_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x30, (v))

#define DISPLAY_MODE_WRITE(v)         IOWR_32DIRECT(CONTROL_BASE, 0x34, (v))
#define DISPLAY_SINGLE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x38, (v))

#define DISPLAY_Q0_WRITE(v)           IOWR_32DIRECT(CONTROL_BASE, 0x3C, (v))
#define DISPLAY_Q1_WRITE(v)           IOWR_32DIRECT(CONTROL_BASE, 0x40, (v))
#define DISPLAY_Q2_WRITE(v)           IOWR_32DIRECT(CONTROL_BASE, 0x44, (v))
#define DISPLAY_Q3_WRITE(v)           IOWR_32DIRECT(CONTROL_BASE, 0x48, (v))

#define DISPLAY_ORIGINAL_ADDR_WRITE(v) IOWR_32DIRECT(CONTROL_BASE, 0x4C, (v))
#define DISPLAY_BUFFER_INDEX_WRITE(v)  IOWR_32DIRECT(CONTROL_BASE, 0x50, (v))


static int last_key1 = 1;
static alt_u32 last_switch_value = 0xFFFFFFFF;

static int next_read_buffer = 0;




static void flip_image(alt_u8 *inputImage, alt_u8 *outputImage, int width, int height)
{
    int x;
    int y;
    int src_x;
    int src_y;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            src_x = width - 1 - x;
            src_y = height - 1 - y;

            outputImage[y * width + x] = inputImage[src_y * width + src_x] & 0x0F;
        }
    }
}

static void blur_image(alt_u8 *inputImage, alt_u8 *outputImage, int width, int height)
{
    int x;
    int y;
    int i;
    int sum;

    for (i = 0; i < width * height; i++)
    {
        outputImage[i] = 0;
    }

    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            sum =
                (inputImage[(y - 1) * width + (x - 1)] & 0x0F) +
                (inputImage[(y - 1) * width + x] & 0x0F) +
                (inputImage[(y - 1) * width + (x + 1)] & 0x0F) +

                (inputImage[y * width + (x - 1)] & 0x0F) +
                (inputImage[y * width + x] & 0x0F) +
                (inputImage[y * width + (x + 1)] & 0x0F) +

                (inputImage[(y + 1) * width + (x - 1)] & 0x0F) +
                (inputImage[(y + 1) * width + x] & 0x0F) +
                (inputImage[(y + 1) * width + (x + 1)] & 0x0F);

            outputImage[y * width + x] = (alt_u8)(sum / 9);
        }
    }
}

static int quad_needs_image(int image)
{
    if (SHARED_Q0_READ() == image)
    {
        return 1;
    }

    if (SHARED_Q1_READ() == image)
    {
        return 1;
    }

    if (SHARED_Q2_READ() == image)
    {
        return 1;
    }

    if (SHARED_Q3_READ() == image)
    {
        return 1;
    }

    return 0;
}

static int edge_is_needed(int mode)
{
    if (mode == MODE_SINGLE)
    {
        return SHARED_SINGLE_READ() == IMAGE_EDGE;
    }

    return quad_needs_image(IMAGE_EDGE);
}

static int selected_images_are_ready(int buffer_index, int mode)
{
    if (!edge_is_needed(mode))
    {
        return 1;
    }

    if (buffer_index == 0)
    {
        return EDGE0_STATE_READ() == BUFFER_READY;
    }

    return EDGE1_STATE_READ() == BUFFER_READY;
}

static void process_for_single_mode(
    alt_u8 *original,
    alt_u8 *flipped,
    alt_u8 *blurred
)
{
    int choice = SHARED_SINGLE_READ();

    if (choice == IMAGE_FLIPPED)
    {
        flip_image(original, flipped, CAM_W, CAM_H);
    }
    else if (choice == IMAGE_BLURRED)
    {
        blur_image(original, blurred, CAM_W, CAM_H);
    }
}

static void process_for_quad_mode(
    alt_u8 *original,
    alt_u8 *flipped,
    alt_u8 *blurred
)
{
    if (quad_needs_image(IMAGE_FLIPPED))
    {
        flip_image(original, flipped, CAM_W, CAM_H);
    }

    if (quad_needs_image(IMAGE_BLURRED))
    {
        blur_image(original, blurred, CAM_W, CAM_H);
    }
}

static void update_quad_choices_from_switches(void)
{
    alt_u32 sw = IORD(SWITCH_BASE, 0);

    SHARED_Q0_WRITE((sw >> 0) & 0x3);
    SHARED_Q1_WRITE((sw >> 2) & 0x3);
    SHARED_Q2_WRITE((sw >> 4) & 0x3);
    SHARED_Q3_WRITE((sw >> 6) & 0x3);
}

static void update_display_mode_from_key(void)
{
    int keys = IORD(KEY_BASE, 0);
    int key1 = (keys >> 1) & 0x1;

    if (last_key1 == 1 && key1 == 0)
    {
        if (SHARED_MODE_READ() == MODE_SINGLE)
        {
            SHARED_MODE_WRITE(MODE_QUAD);
            printf("Display mode: QUAD\n");
        }
        else
        {
            SHARED_MODE_WRITE(MODE_SINGLE);
            printf("Display mode: SINGLE\n");
        }

    }

    last_key1 = key1;
}

static void update_quad_choices_if_switch_changed(void)
{
    alt_u32 sw = IORD(SWITCH_BASE, 0) & 0xFF;

    if (sw != last_switch_value)
    {
        update_quad_choices_from_switches();
        last_switch_value = sw;

        printf("Updated quad choices from switches: %d %d %d %d\n",
               SHARED_Q0_READ(),
               SHARED_Q1_READ(),
               SHARED_Q2_READ(),
               SHARED_Q3_READ());

    }
}

static int get_ready_buffer(void)
{
    if (next_read_buffer == 0)
    {
        if (BUF0_STATE_READ() == BUFFER_READY)
        {
            next_read_buffer = 1;
            return 0;
        }

        if (BUF1_STATE_READ() == BUFFER_READY)
        {
            next_read_buffer = 0;
            return 1;
        }
    }
    else
    {
        if (BUF1_STATE_READ() == BUFFER_READY)
        {
            next_read_buffer = 0;
            return 1;
        }

        if (BUF0_STATE_READ() == BUFFER_READY)
        {
            next_read_buffer = 1;
            return 0;
        }
    }

    return -1;
}

static void send_display_request(
    int buffer_index,
    alt_u8 *original_frame,
    int mode,
    int single_choice,
    alt_u32 quad_value
)
{
    while (DISPLAY_STATE_READ() != DISPLAY_EMPTY)
    {
    }

    DISPLAY_MODE_WRITE(mode);
    DISPLAY_SINGLE_WRITE(single_choice);

    DISPLAY_Q0_WRITE((quad_value >> 0) & 0x3);
    DISPLAY_Q1_WRITE((quad_value >> 2) & 0x3);
    DISPLAY_Q2_WRITE((quad_value >> 4) & 0x3);
    DISPLAY_Q3_WRITE((quad_value >> 6) & 0x3);

    DISPLAY_ORIGINAL_ADDR_WRITE((alt_u32)original_frame);
    DISPLAY_BUFFER_INDEX_WRITE((alt_u32)buffer_index);

    DISPLAY_STATE_WRITE(DISPLAY_READY);
}

int main(void)
{
    alt_u8 *original_frame;
    alt_u8 *flipped_frame;
    alt_u8 *blurred_frame;

    int buffer_index;
    int mode;
    int single_choice;
    alt_u32 quad_value;


    printf("processing_proc running\n");

    while (1)
    {
        update_display_mode_from_key();

        if (SHARED_MODE_READ() == MODE_QUAD)
        {
            update_quad_choices_if_switch_changed();
        }

        buffer_index = get_ready_buffer();

        if (buffer_index < 0)
        {
            continue;
        }

        mode = SHARED_MODE_READ();

        if (!selected_images_are_ready(buffer_index, mode))
        {
            next_read_buffer = buffer_index;
            continue;
        }

        if (buffer_index == 0)
        {
            original_frame = (alt_u8 *) ORIGINAL0_BASE;
            flipped_frame = (alt_u8 *) FLIPPED0_BASE;
            blurred_frame = (alt_u8 *) BLURRED0_BASE;
        }
        else
        {
            original_frame = (alt_u8 *) ORIGINAL1_BASE;
            flipped_frame = (alt_u8 *) FLIPPED1_BASE;
            blurred_frame = (alt_u8 *) BLURRED1_BASE;
        }

        single_choice = SHARED_SINGLE_READ();

        quad_value =
            ((alt_u32)SHARED_Q0_READ() << 0) |
            ((alt_u32)SHARED_Q1_READ() << 2) |
            ((alt_u32)SHARED_Q2_READ() << 4) |
            ((alt_u32)SHARED_Q3_READ() << 6);

        if (mode == MODE_QUAD)
        {
            process_for_quad_mode(
                original_frame,
                flipped_frame,
                blurred_frame
            );

        }
        else
        {
            process_for_single_mode(
                original_frame,
                flipped_frame,
                blurred_frame
            );

        }

        send_display_request(
            buffer_index,
            original_frame,
            mode,
            single_choice,
            quad_value
        );
    }

    return 0;
}
