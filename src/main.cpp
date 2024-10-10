// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>

#include <gccore.h>
#include <network.h>
#include <wiiuse/wpad.h>

#include <gxflux/gfx.h>
#include <gxflux/gfx_con.h>

#include "socket.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


using namespace std::literals;

using std::runtime_error;
using std::array;


struct gfx_raii {

    gfx_raii()
        noexcept
    {
        gfx_video_init(nullptr);
        gfx_init();
    }

    ~gfx_raii()
        noexcept
    {
        gfx_deinit();
        gfx_video_deinit();
    }

};


struct gfx_con_raii {

    gfx_con_raii()
    {
        gfx_screen_coords_t coords {
            16.0f,
            32.0f,
            gfx_video_get_width() - 16.0f,
            gfx_video_get_height() - 32.0f
        };
        if (!gfx_con_init(&coords))
            throw runtime_error{"gfx_con_init() failed."};
    }

    ~gfx_con_raii()
        noexcept
    {
        gfx_con_deinit();
    }

};


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


std::string
to_string(in_addr addr)
{
    return inet_ntoa(addr);
}


void
main_loop()
{
    bool replace_eol = true;
    bool running = true;

    std::printf("Initializing network...\n");
    redraw_console();
    net::init_guard net_guard;

    net::socket server{AF_INET, SOCK_STREAM};
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(4405);
    server.bind(addr);
    server.listen(1);

    in_addr host_addr{ net_gethostip() };
    std::printf("Listening to TCP connections on %s:%u\n",
                to_string(host_addr).c_str(),
                unsigned(ntohs(addr.sin_port)));
    redraw_console();

    net::socket client;

    std::printf("Press 1/X to toggle replacing EOL ('\\r' -> '\\n')\n");
    std::printf("Press RESET/HOME/START to exit.\n");

    if (!usb_isgeckoalive(0))
        std::printf("WARNING: no gecko detected on slot A\n");

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


        if (server.is_readable()) {
            try {
                sockaddr_in addr;
                client = server.accept(addr);

                std::printf("Client connected from %s\n",
                            to_string(addr.sin_addr).c_str());
                redraw_console();
            }
            catch (std::exception& e) {
                std::printf("Failed: %s\n", e.what());
                redraw_console();
            }
        }


        if (client && client.is_readable()) {
            try {
                static array<char, 4096> client_buf;
                const int r = client.recv(client_buf.data(), client_buf.size() - 1);
                if (r > 0) {
                    client_buf[r] = '\0';

                    if (replace_eol) {
                        for (int idx = 0; idx < r; ++idx)
                            if (client_buf[idx] == '\n')
                                client_buf[idx] = '\r';
                    }

                    int s = usb_sendbuffer_safe(0, client_buf.data(), r);
                    if (s < r) {
                        std::printf("ERROR: could not send all data to gecko: %d < %d\n",
                                    s, r);
                        redraw_console();
                    }
                } else if (r == 0) {
                    client.close();
                    std::printf("\nClosed client connection.\n");
                    redraw_console();
                }
            }
            catch (std::exception& e) {
                std::printf("Client handling error: %s\n", e.what());
                redraw_console();
            }
        }


        static array<char, 4096> usb_buf;

        int r = usb_recvbuffer(0, usb_buf.data(), usb_buf.size() - 1);
        if (r > 0) {
            usb_buf[r] = '\0';

            for (int i = 0; i < r; ++i) {
                char& c = usb_buf[i];

                // Stop on first 0xff, usually means unpowered/disconnected Gecko.
                if (c == 0xff) {
                    c ='\0';
                    r = i;
                    continue;
                }

                // Replace EOL if requested.
                if (replace_eol && c == '\r')
                    c = '\n';
            }

            // r might have changed if we encountered 0xff
            if (r) {
                std::fputs(usb_buf.data(), stdout);
                if (client)
                    client.send_all(usb_buf.data(), r);
            }
        }

        redraw_console();
    }
}


int main()
{
    VIDEO_Init();
    WPAD_Init();
    PAD_Init();
    gfx_raii gfx_guard;
    gfx_con_raii gfx_con_guard;

    try {
        std::printf("%s\n", PACKAGE_STRING);

        main_loop();

    }
    catch (std::exception& e) {
        std::printf("\nERROR: %s\n", e.what());
        for (int i = 0; i < 600; ++i)
            redraw_console();
        return -1;
    }
}
