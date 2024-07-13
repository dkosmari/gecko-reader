// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include <gccore.h>
#include <network.h>
#include <wiiuse/wpad.h>

#include <gxflux/gfx.h>
#include <gxflux/gfx_con.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


void
redraw_console()
{
    std::fflush(stdout);

    VIDEO_WaitVSync();

    if (gfx_frame_start()) {
        gfx_con_draw();
        gfx_frame_end();
    }
    VIDEO_Flush();
}


static int log_fd = -1;
static struct sockaddr_in bcast_addr;

bool
log_init()
{
    std::printf("Initializing network... ");
    redraw_console();

    int net_status = net_init();
    if (net_status) {
        std::printf("failed: (%d) %s\n", net_status, std::strerror(-net_status));
        redraw_console();
        return false;
    }
    std::printf("OK.\n");
    redraw_console();

    log_fd = net_socket(AF_INET, SOCK_DGRAM, 0);
    if (log_fd < 0)
        return false;

#if 0
    // Note: it doesn't appear the Wii understands the SO_BROADCAST option at all.
    std::printf("Setting UDP log fd up for broadcast... ");
    redraw_console();

    unsigned enable = 1;
    int opt_status = net_setsockopt(log_fd,
                                    SOL_SOCKET,
                                    SO_BROADCAST,
                                    &enable,
                                    sizeof enable);
    if (opt_status < 0)
        std::printf("failed: (%d) %s\n", opt_status, std::strerror(-opt_status));
    else
        std::printf("OK.\n");
    redraw_console();
#endif

    std::memset(&bcast_addr, 0, sizeof bcast_addr);
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(4405);
    bcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // Note: the only way to get the Wii to send to broadcast address is with
    // net_connect().
    std::printf("Broadcasting to UDP port %d... ", ntohs(bcast_addr.sin_port));
    redraw_console();

    int con_status = net_connect(log_fd,
                                 reinterpret_cast<struct sockaddr*>(&bcast_addr),
                                 sizeof bcast_addr);
    if (con_status < 0)
        std::printf("net_connect() failed: (%d) %s\n",
                    con_status, std::strerror(-con_status));
    else
        std::printf("OK\n");
    redraw_console();

    return true;
}


void
log_send(const char* buf, int size)
{
    if (log_fd == -1 || size <= 0)
        return;

    net_send(log_fd, buf, size, 0);
}


int main()
{
    VIDEO_Init();
    WPAD_Init();
    PAD_Init();

    gfx_video_init(nullptr);
    gfx_init();
    gfx_screen_coords_t coords {
        16.0f,
        16.0f,
        gfx_video_get_width() - 16.0f,
        gfx_video_get_height() - 16.0f
    };
    gfx_con_init(&coords);

    std::printf("%s\n", PACKAGE_STRING);

    if (!log_init())
        goto error;

    std::printf("Press 1/X to toggle replacing EOL ('\\r' -> '\\n')\n");
    std::printf("Press RESET/HOME/START to exit.\n");

    if (!usb_isgeckoalive(0))
        std::printf("WARNING: no gecko detected on slot A\n");

    redraw_console();

    {
        bool replace_eol = true;
        bool running = true;

        auto toggle_replace_eol = [&replace_eol]
        {
            replace_eol = !replace_eol;
            std::printf(":: replace_eol is %s\n", replace_eol ? "true" : "false");
        };

        while (running) {

            if (SYS_ResetButtonDown()) {
                std::printf("RESET pressed, exiting...\n");
                running = false;
            }

            PAD_ScanPads();
            u32 gpressed = PAD_ButtonsDown(PAD_CHAN0);
            if (gpressed) {
                if (gpressed & PAD_BUTTON_START) {
                    running = false;
                    std::printf("GC START pressed, exiting...\n");
                }
                if (gpressed & PAD_BUTTON_X)
                    toggle_replace_eol();
            }

            WPAD_ScanPads();
            u32 wpressed = WPAD_ButtonsDown(WPAD_CHAN_0);
            if (wpressed) {
                if (wpressed & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) {
                    running = false;
                    std::printf("Wiimote HOME pressed, exiting...\n");
                }
                if (wpressed & (WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_X))
                    toggle_replace_eol();
            }

            // Note: keep buf below the MTU, so we don't have problems with the UDP packet
            static char buf[1200 + 1];
            int r = usb_recvbuffer(0, buf, (sizeof buf) - 1);
            if (r > 0) {
                bool all_ff = true;

                buf[r] = '\0';

                for (int i = 0; i < r; ++i) {
                    if (buf[i] != 0xff)
                        all_ff = false;
                    if (replace_eol && buf[i] == '\r')
                        buf[i] = '\n';
                }

                if (!all_ff) {
                    std::fputs(buf, stdout);
                    log_send(buf, r);
                }
            }

            redraw_console();
        }
    }

    if (log_fd != -1)
        net_close(log_fd);

    net_deinit();


    gfx_con_deinit();
    gfx_deinit();
    gfx_video_deinit();
    return 0;


 error:

    for (int i = 0; i < 300; ++i)
        redraw_console();

    net_deinit();
    gfx_con_deinit();
    gfx_deinit();
    gfx_video_deinit();
    return -1;
}
