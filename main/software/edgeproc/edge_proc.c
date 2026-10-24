//ID: 33931097, 33531153, 32920474, 35492694
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "io.h"

#ifndef ALT_CI_MULTCALCULATOR_0
#warning "ALT_CI_MULTCALCULATOR_0 is NOT defined in system.h"
#else
#warning "ALT_CI_MULTCALCULATOR_0 IS defined"
#endif

#define CAM_W 160
#define CAM_H 120
#define IMG_SIZE (CAM_W * CAM_H)

#define BUFFER_EMPTY 0
#define BUFFER_READY 1
#define BUFFER_BUSY  2

#define FRAME_BASE        0x04400000
#define CONTROL_BASE      0x04500000

#define ORIGINAL0_BASE    (FRAME_BASE + 0x00000)
#define ORIGINAL1_BASE    (FRAME_BASE + 0x08000)

#define EDGE0_BASE        (FRAME_BASE + 0x20000)
#define EDGE1_BASE        (FRAME_BASE + 0x28000)



#define ORIG0_STATE_OFFSET  0x18
#define ORIG1_STATE_OFFSET  0x1C

#define EDGE0_STATE_OFFSET  0x28
#define EDGE1_STATE_OFFSET  0x2C

#define CTRL_READ(offset)      IORD_32DIRECT(CONTROL_BASE, offset)
#define CTRL_WRITE(offset, v)  IOWR_32DIRECT(CONTROL_BASE, offset, v)

#define ORIG0_STATE_READ()     CTRL_READ(ORIG0_STATE_OFFSET)
#define ORIG1_STATE_READ()     CTRL_READ(ORIG1_STATE_OFFSET)

#define EDGE0_STATE_READ()     CTRL_READ(EDGE0_STATE_OFFSET)
#define EDGE0_STATE_WRITE(v)   CTRL_WRITE(EDGE0_STATE_OFFSET, v)

#define EDGE1_STATE_READ()     CTRL_READ(EDGE1_STATE_OFFSET)
#define EDGE1_STATE_WRITE(v)   CTRL_WRITE(EDGE1_STATE_OFFSET, v)

#define IMAGE_EDGE 3

#define MODE_SINGLE 0
#define MODE_QUAD   1

#define SHARED_MODE_READ()      IORD_32DIRECT(CONTROL_BASE, 0x00)
#define SHARED_SINGLE_READ()    IORD_32DIRECT(CONTROL_BASE, 0x04)

#define SHARED_Q0_READ()        IORD_32DIRECT(CONTROL_BASE, 0x08)
#define SHARED_Q1_READ()        IORD_32DIRECT(CONTROL_BASE, 0x0C)
#define SHARED_Q2_READ()        IORD_32DIRECT(CONTROL_BASE, 0x10)
#define SHARED_Q3_READ()        IORD_32DIRECT(CONTROL_BASE, 0x14)



static inline int sobel_ci(int packed_pixels, int threshold)
{
    return ALT_CI_MULTCALCULATOR_0(packed_pixels, threshold);
}

static void edge_detect_image(alt_u8 *inputImage, alt_u8 *outputImage, int width, int height)
{
    int x;
    int y;
    int i;
    int threshold = 15;
    int packed_pixels;
    int result;

    for (i = 0; i < width * height; i++)
    {
        outputImage[i] = 0;
    }

    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            packed_pixels =
                ((inputImage[(y - 1) * width + (x - 1)] & 0x0F) << 0)  |
                ((inputImage[(y - 1) * width + (x    )] & 0x0F) << 4)  |
                ((inputImage[(y - 1) * width + (x + 1)] & 0x0F) << 8)  |
                ((inputImage[(y    ) * width + (x - 1)] & 0x0F) << 12) |
                ((inputImage[(y    ) * width + (x + 1)] & 0x0F) << 16) |
                ((inputImage[(y + 1) * width + (x - 1)] & 0x0F) << 20) |
                ((inputImage[(y + 1) * width + (x    )] & 0x0F) << 24) |
                ((inputImage[(y + 1) * width + (x + 1)] & 0x0F) << 28);

            result = sobel_ci(packed_pixels, threshold);

            outputImage[y * width + x] = (alt_u8)(result & 0x0F);
        }
    }
}

static int edge_is_needed(void)
{
    if (SHARED_MODE_READ() == MODE_SINGLE)
    {
        return SHARED_SINGLE_READ() == IMAGE_EDGE;
    }

    if (SHARED_Q0_READ() == IMAGE_EDGE)
    {
        return 1;
    }

    if (SHARED_Q1_READ() == IMAGE_EDGE)
    {
        return 1;
    }

    if (SHARED_Q2_READ() == IMAGE_EDGE)
    {
        return 1;
    }

    if (SHARED_Q3_READ() == IMAGE_EDGE)
    {
        return 1;
    }

    return 0;
}

static void handle_edge_buffer_0(void)
{
    alt_u8 *original;
    alt_u8 *edge;

    if (ORIG0_STATE_READ() != BUFFER_READY)
    {
        return;
    }

    // If edge not needed, mark edge buffer as READY immediately
    if (!edge_is_needed())
    {
        if (EDGE0_STATE_READ() == BUFFER_EMPTY)
        {
            EDGE0_STATE_WRITE(BUFFER_READY);
        }
        return;
    }

    if (EDGE0_STATE_READ() != BUFFER_EMPTY)
    {
        return;
    }

    EDGE0_STATE_WRITE(BUFFER_BUSY);

    original = (alt_u8 *) ORIGINAL0_BASE;
    edge = (alt_u8 *) EDGE0_BASE;

    edge_detect_image(original, edge, CAM_W, CAM_H);

    EDGE0_STATE_WRITE(BUFFER_READY);
}

static void handle_edge_buffer_1(void)
{
    alt_u8 *original;
    alt_u8 *edge;

    if (ORIG1_STATE_READ() != BUFFER_READY)
    {
        return;
    }

    // If edge not needed, mark edge buffer as READY immediately
    if (!edge_is_needed())
    {
        if (EDGE1_STATE_READ() == BUFFER_EMPTY)
        {
            EDGE1_STATE_WRITE(BUFFER_READY);
        }
        return;
    }

    if (EDGE1_STATE_READ() != BUFFER_EMPTY)
    {
        return;
    }

    EDGE1_STATE_WRITE(BUFFER_BUSY);

    original = (alt_u8 *) ORIGINAL1_BASE;
    edge = (alt_u8 *) EDGE1_BASE;

    edge_detect_image(original, edge, CAM_W, CAM_H);

    EDGE1_STATE_WRITE(BUFFER_READY);
}



int main(void)
{
    printf("edge_proc running\n");

    while (1)
    {
        handle_edge_buffer_0();
        handle_edge_buffer_1();
    }

    return 0;
}
