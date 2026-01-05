#include "game.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

namespace {
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 860;
constexpr int kBoardOffsetX = 60;
constexpr int kBoardOffsetY = 120;
constexpr int kCellSize = 40;
constexpr int kBoardPixelSize = kCellSize * Game::kBoardSize;

struct Color {
    unsigned long pixel;
};

Color MakeColor(Display *display, int r, int g, int b) {
    XColor color;
    color.red = static_cast<unsigned short>(r * 257);
    color.green = static_cast<unsigned short>(g * 257);
    color.blue = static_cast<unsigned short>(b * 257);
    color.flags = DoRed | DoGreen | DoBlue;
    Colormap colormap = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, colormap, &color);
    return {color.pixel};
}

bool IsPointInBoard(int x, int y) {
    return x >= kBoardOffsetX && x <= kBoardOffsetX + kBoardPixelSize &&
           y >= kBoardOffsetY && y <= kBoardOffsetY + kBoardPixelSize;
}

std::pair<int, int> ScreenToCell(int x, int y) {
    int cell_x = (x - kBoardOffsetX) / kCellSize;
    int cell_y = (y - kBoardOffsetY) / kCellSize;
    return {cell_x, cell_y};
}
}

int main() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        return 1;
    }

    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0,
                                        kWindowWidth, kWindowHeight, 1,
                                        BlackPixel(display, screen), WhitePixel(display, screen));

    Atom delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &delete_message, 1);

    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, nullptr);

    Color board_bg = MakeColor(display, 245, 235, 215);
    Color board_line = MakeColor(display, 90, 60, 30);
    Color stone_black = MakeColor(display, 40, 40, 40);
    Color stone_white = MakeColor(display, 230, 230, 230);
    Color highlight = MakeColor(display, 200, 40, 40);
    Color text_color = MakeColor(display, 20, 80, 40);

    Game game;
    bool game_over = false;
    int winner = 0;

    bool running = true;
    while (running) {
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);
            if (event.type == ClientMessage) {
                if (static_cast<Atom>(event.xclient.data.l[0]) == delete_message) {
                    running = false;
                }
            } else if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_r || key == XK_R) {
                    game.reset();
                    game_over = false;
                    winner = 0;
                }
            } else if (event.type == ButtonPress) {
                if (event.xbutton.button == Button1) {
                    int mouse_x = event.xbutton.x;
                    int mouse_y = event.xbutton.y;
                    if (!game_over && game.currentPlayer() == 1 && IsPointInBoard(mouse_x, mouse_y)) {
                        auto [x, y] = ScreenToCell(mouse_x, mouse_y);
                        if (game.placeStone(x, y, 1)) {
                            if (game.checkWin(x, y, 1)) {
                                game_over = true;
                                winner = 1;
                            } else if (game.isBoardFull()) {
                                game_over = true;
                                winner = 0;
                            } else {
                                game.setCurrentPlayer(2);
                            }
                        }
                    }
                }
            }
        }

        if (!game_over && game.currentPlayer() == 2) {
            auto [x, y] = game.computeAIMove();
            if (game.placeStone(x, y, 2)) {
                if (game.checkWin(x, y, 2)) {
                    game_over = true;
                    winner = 2;
                } else if (game.isBoardFull()) {
                    game_over = true;
                    winner = 0;
                } else {
                    game.setCurrentPlayer(1);
                }
            }
        }

        XSetForeground(display, gc, board_bg.pixel);
        XFillRectangle(display, window, gc, 0, 0, kWindowWidth, kWindowHeight);

        XSetForeground(display, gc, board_line.pixel);
        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX;
            int start_y = kBoardOffsetY + i * kCellSize;
            int end_x = kBoardOffsetX + kBoardPixelSize;
            XDrawLine(display, window, gc, start_x, start_y, end_x, start_y);
        }
        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX + i * kCellSize;
            int start_y = kBoardOffsetY;
            int end_y = kBoardOffsetY + kBoardPixelSize;
            XDrawLine(display, window, gc, start_x, start_y, start_x, end_y);
        }

        for (int y = 0; y < Game::kBoardSize; ++y) {
            for (int x = 0; x < Game::kBoardSize; ++x) {
                int value = game.at(x, y);
                if (value == 0) {
                    continue;
                }
                int center_x = kBoardOffsetX + x * kCellSize + kCellSize / 2;
                int center_y = kBoardOffsetY + y * kCellSize + kCellSize / 2;
                XSetForeground(display, gc, value == 1 ? stone_black.pixel : stone_white.pixel);
                XFillArc(display, window, gc, center_x - 16, center_y - 16, 32, 32, 0, 360 * 64);
                XSetForeground(display, gc, board_line.pixel);
                XDrawArc(display, window, gc, center_x - 16, center_y - 16, 32, 32, 0, 360 * 64);
            }
        }

        if (auto last = game.lastMove()) {
            int x = last->first;
            int y = last->second;
            int rect_x = kBoardOffsetX + x * kCellSize + 2;
            int rect_y = kBoardOffsetY + y * kCellSize + 2;
            XSetForeground(display, gc, highlight.pixel);
            XDrawRectangle(display, window, gc, rect_x, rect_y, kCellSize - 4, kCellSize - 4);
        }

        std::string status = "Turn: ";
        if (game_over) {
            if (winner == 1) {
                status = "Human wins!";
            } else if (winner == 2) {
                status = "AI wins!";
            } else {
                status = "Draw";
            }
        } else {
            status += game.currentPlayer() == 1 ? "Human" : "AI";
        }
        XSetForeground(display, gc, text_color.pixel);
        XDrawString(display, window, gc, 60, 60, status.c_str(), static_cast<int>(status.size()));
        XDrawString(display, window, gc, 60, 90, "Click to place stones. Press R to restart.", 44);

        XFlush(display);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
