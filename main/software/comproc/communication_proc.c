//ID: 33931097 and 35492694
#include <stdio.h>
#include <stdlib.h>
#include "altera_avalon_spi.h"
#include "system.h"
#include "io.h"

#define BW_RATE         0x2C
#define POWER_CONTROL   0x2D
#define DATA_FORMAT     0x31
#define INT_ENABLE      0x2E
#define INT_MAP         0x2F
#define INT_SOURCE      0x30

#define THRESH_ACT      0x24
#define THRESH_INACT    0x25
#define TIME_INACT      0x26
#define ACT_INACT_CTL   0x27
#define THRESH_FF       0x28
#define TIME_FF         0x29
#define TAP_AXES        0x2A
#define TAP_THRES       0x1D
#define LATENT          0x22
#define DUR             0x21
#define WINDOW          0x23

#define X_LB            0x32
#define Y_LB            0x34
#define Z_LB            0x36
#define DEVID           0x00

#define CONFIG_LENGTH   (16 * 2)

#define CAM_SPI_SLAVE   0
#define ACCEL_SPI_SLAVE 1

#define PIO_EDGE_CAPTURE_OFFSET 3

#define READ_XYZ        (0xC0 | X_LB)
#define DOUBLE_TAP_BIT  0x20

#define CAM_W       160
#define CAM_H       120
#define CAM_SIZE    ((CAM_W * CAM_H) / 2)
#define IMG_SIZE    (CAM_W * CAM_H)

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
#define CONTROL_BASE      0x04500000

#define ORIGINAL0_BASE    (FRAME_BASE + 0x00000)
#define ORIGINAL1_BASE    (FRAME_BASE + 0x08000)


#define SHARED_MODE_READ()        IORD_32DIRECT(CONTROL_BASE, 0x00)
#define SHARED_MODE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x00, (v))

#define SHARED_SINGLE_READ()      IORD_32DIRECT(CONTROL_BASE, 0x04)
#define SHARED_SINGLE_WRITE(v)    IOWR_32DIRECT(CONTROL_BASE, 0x04, (v))

#define SHARED_Q0_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x08, (v))
#define SHARED_Q1_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x0C, (v))
#define SHARED_Q2_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x10, (v))
#define SHARED_Q3_WRITE(v)        IOWR_32DIRECT(CONTROL_BASE, 0x14, (v))

#define SHARED_Q0_READ() 		  IORD_32DIRECT(CONTROL_BASE, 0x08)
#define SHARED_Q1_READ()		  IORD_32DIRECT(CONTROL_BASE, 0x0C)
#define SHARED_Q2_READ()		  IORD_32DIRECT(CONTROL_BASE, 0x10)
#define SHARED_Q3_READ()		  IORD_32DIRECT(CONTROL_BASE, 0x14)

#define BUF0_STATE_READ()         IORD_32DIRECT(CONTROL_BASE, 0x18)
#define BUF0_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x18, (v))

#define BUF1_STATE_READ()         IORD_32DIRECT(CONTROL_BASE, 0x1C)
#define BUF1_STATE_WRITE(v)       IOWR_32DIRECT(CONTROL_BASE, 0x1C, (v))

#define BLUR0_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x20, (v))
#define BLUR1_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x24, (v))

#define EDGE0_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x28, (v))
#define EDGE1_STATE_WRITE(v)      IOWR_32DIRECT(CONTROL_BASE, 0x2C, (v))

#define DISPLAY_EMPTY 0
#define DISPLAY_READY 1
#define DISPLAY_BUSY  2

#define DISPLAY_STATE_WRITE(v) IOWR_32DIRECT(CONTROL_BASE, 0x30, (v))

static int single_image_choice = IMAGE_ORIGINAL;
static int next_write_buffer = 0;
static int tilt_cooldown = 5000;

alt_u8 gyro_config[CONFIG_LENGTH] = {
    DATA_FORMAT,   0x0B,
    THRESH_ACT,    0x04,
    THRESH_INACT,  0x02,
    TIME_INACT,    0x02,
    ACT_INACT_CTL, 0xFF,
    THRESH_FF,     0x09,
    TIME_FF,       0x46,
    TAP_THRES,     0x20,
    TAP_AXES,      0x07,
    LATENT,        0x20,
    DUR,           0x30,
    WINDOW,        0x50,
    BW_RATE,       0x0A,
    INT_ENABLE,    0x20,
    INT_MAP,       0x20,
    POWER_CONTROL, 0x08
};



static void unpack_frame(alt_u8 *packed_frame, alt_u8 *unpacked_frame)
{
    int i;
    int out_index = 0;

    for (i = 0; i < CAM_SIZE; i++)
    {
        alt_u8 packed = packed_frame[i];

        unpacked_frame[out_index++] = (packed >> 4) & 0x0F;
        unpacked_frame[out_index++] = packed & 0x0F;
    }
}

static void accel_write_reg(alt_u8 reg, alt_u8 value)
{
    alt_u8 tx[2];
    alt_u8 dummy;

    tx[0] = reg;
    tx[1] = value;

    alt_avalon_spi_command(SPI_BASE, ACCEL_SPI_SLAVE, 2, tx, 0, &dummy, 0);
}

static alt_u8 accel_read_reg(alt_u8 reg)
{
    alt_u8 tx = (0x80 | reg);
    alt_u8 rx[1] = {0};

    alt_avalon_spi_command(SPI_BASE, ACCEL_SPI_SLAVE, 1, &tx, 1, rx, 0);

    return rx[0];
}

static void accel_read_xyz(alt_16 *x, alt_16 *y, alt_16 *z)
{
    alt_u8 cmd = READ_XYZ;
    alt_u8 buf[6] = {0};

    alt_avalon_spi_command(SPI_BASE, ACCEL_SPI_SLAVE, 1, &cmd, 6, buf, 0);

    *x = (alt_16)((buf[1] << 8) | buf[0]);
    *y = (alt_16)((buf[3] << 8) | buf[2]);
    *z = (alt_16)((buf[5] << 8) | buf[4]);
}

static void accel_init(void)
{
    int i;

    IOWR(ACCELEROMETER_BASE, PIO_EDGE_CAPTURE_OFFSET, 0);

    for (i = 0; i < CONFIG_LENGTH; i += 2)
    {
        accel_write_reg(gyro_config[i], gyro_config[i + 1]);
    }

    printf("ADXL345 DEVID = 0x%02X\n", accel_read_reg(DEVID));
}

static void rotate_quad_choices_clockwise(void)
{
    int q0 = SHARED_Q0_READ();
    int q1 = SHARED_Q1_READ();
    int q2 = SHARED_Q2_READ();
    int q3 = SHARED_Q3_READ();

    SHARED_Q0_WRITE(q2);
    SHARED_Q1_WRITE(q0);
    SHARED_Q2_WRITE(q3);
    SHARED_Q3_WRITE(q1);
}

static void rotate_quad_choices_anticlockwise(void)
{
    int q0 = SHARED_Q0_READ();
    int q1 = SHARED_Q1_READ();
    int q2 = SHARED_Q2_READ();
    int q3 = SHARED_Q3_READ();

    SHARED_Q0_WRITE(q1);
    SHARED_Q1_WRITE(q3);
    SHARED_Q2_WRITE(q0);
    SHARED_Q3_WRITE(q2);
}

static void handle_accelerometer(void)
{
    alt_u8 int_src;
    alt_16 xData = 0;
    alt_16 yData = 0;
    alt_16 zData = 0;

    int_src = accel_read_reg(INT_SOURCE);
    IOWR(ACCELEROMETER_BASE, PIO_EDGE_CAPTURE_OFFSET, 0x1);

    if (int_src & DOUBLE_TAP_BIT)
    {
        if (SHARED_MODE_READ() == MODE_SINGLE)
        {
            single_image_choice++;

            if (single_image_choice > IMAGE_EDGE)
            {
                single_image_choice = IMAGE_ORIGINAL;
            }

            SHARED_SINGLE_WRITE(single_image_choice);

            printf("Double Tap: single image choice = %d\n", single_image_choice);
        }
        else
        {
            SHARED_MODE_WRITE(MODE_SINGLE);
            printf("Double Tap: switch to single mode\n");
        }
    }

    accel_read_xyz(&xData, &yData, &zData);

    if (SHARED_MODE_READ() == MODE_QUAD)
    {
        if (tilt_cooldown > 0)
        {
            tilt_cooldown--;
        }

        if (tilt_cooldown == 0 && xData > 200)
        {
            rotate_quad_choices_clockwise();
            tilt_cooldown = 20;
            printf("Tilt right: rotate quad choices clockwise, x=%d\n", xData);
        }
        else if (tilt_cooldown == 0 && xData < -200)
        {
            rotate_quad_choices_anticlockwise();
            tilt_cooldown = 20;
            printf("Tilt left: rotate quad choices anticlockwise, x=%d\n", xData);
        }
    }
}
static int get_empty_buffer(void)
{
    if (next_write_buffer == 0)
    {

        if (BUF0_STATE_READ() == BUFFER_EMPTY)
        {
            next_write_buffer = 1;
            return 0;
        }

        if (BUF1_STATE_READ() == BUFFER_EMPTY)
        {
            next_write_buffer = 0;
            return 1;
        }
    }
    else
    {
        if (BUF1_STATE_READ() == BUFFER_EMPTY)
        {
            next_write_buffer = 0;
            return 1;
        }

        if (BUF0_STATE_READ() == BUFFER_EMPTY)
        {
            next_write_buffer = 1;
            return 0;
        }
    }

    return -1;
}

static void handle_camera(alt_u8 *packed_frame)
{
    alt_u8 tx = 0x16;
    alt_u8 *target_frame;
    int buffer_index;
    static int stuck_counter = 0;

    buffer_index = get_empty_buffer();

    if (buffer_index < 0)
    {
        stuck_counter++;

        if (stuck_counter > 1000000)
        {
            printf("WARNING: buffers stuck, forcing reset\n");

            BUF0_STATE_WRITE(BUFFER_EMPTY);
            BUF1_STATE_WRITE(BUFFER_EMPTY);

            EDGE0_STATE_WRITE(BUFFER_EMPTY);
            EDGE1_STATE_WRITE(BUFFER_EMPTY);

            stuck_counter = 0;
        }

        return;
    }

    stuck_counter = 0;
    if (!IORD(CAMERA_READY_BASE, 0))
    {
        return;
    }

    buffer_index = get_empty_buffer();

    if (buffer_index < 0)
    {
        return;
    }

    if (buffer_index == 0)
    {
        BUF0_STATE_WRITE(BUFFER_BUSY);
        printf("buf0 busy\n");
        BLUR0_STATE_WRITE(BUFFER_EMPTY);
        EDGE0_STATE_WRITE(BUFFER_EMPTY);
        target_frame = (alt_u8 *) ORIGINAL0_BASE;
    }
    else
    {
        BUF1_STATE_WRITE(BUFFER_BUSY);
        printf("buf1 busy\n");
        BLUR1_STATE_WRITE(BUFFER_EMPTY);
        EDGE1_STATE_WRITE(BUFFER_EMPTY);
        target_frame = (alt_u8 *) ORIGINAL1_BASE;
    }

    alt_avalon_spi_command(
        SPI_BASE,
        CAM_SPI_SLAVE,
        1, &tx,
        CAM_SIZE, packed_frame,
        0
    );

    unpack_frame(packed_frame, target_frame);

    if (buffer_index == 0)
    {
        BUF0_STATE_WRITE(BUFFER_READY);
        printf("buf0 ready\n");
    }
    else
    {
        BUF1_STATE_WRITE(BUFFER_READY);
        printf("buf1 ready\n");
    }
}

int main(void)
{
    alt_u8 *packed_frame;

    packed_frame = (alt_u8 *) malloc(CAM_SIZE);

    if (packed_frame == NULL)
    {
        printf("ERROR: packed_frame malloc failed\n");

        while (1)
        {
        }
    }

    printf("communication_proc running\n");

    SHARED_MODE_WRITE(MODE_SINGLE);
    SHARED_SINGLE_WRITE(IMAGE_ORIGINAL);

    SHARED_Q0_WRITE(IMAGE_ORIGINAL);
    SHARED_Q1_WRITE(IMAGE_FLIPPED);
    SHARED_Q2_WRITE(IMAGE_BLURRED);
    SHARED_Q3_WRITE(IMAGE_EDGE);

    BUF0_STATE_WRITE(BUFFER_EMPTY);
    BUF1_STATE_WRITE(BUFFER_EMPTY);

    BLUR0_STATE_WRITE(BUFFER_EMPTY);
    BLUR1_STATE_WRITE(BUFFER_EMPTY);

    EDGE0_STATE_WRITE(BUFFER_EMPTY);
    EDGE1_STATE_WRITE(BUFFER_EMPTY);

    DISPLAY_STATE_WRITE(DISPLAY_EMPTY);


    accel_init();

    while (1)
    {
        handle_accelerometer();
        handle_camera(packed_frame);
    }

    return 0;
}
