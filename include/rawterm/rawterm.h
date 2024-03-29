//////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2023 Ttibsi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////
// Code source: https://github.com/Ttibsi/rawterm/blob/main/rawterm.h
// Version: v3.0.0
//////////////////////////////////////////////////////////////////////////////

#ifndef RAWTERM_H
#define RAWTERM_H

#include <cctype>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#if __linux__
    #include <sys/ioctl.h>
    #include <termios.h>
#elif _WIN32
    #define WINDOWS_LEAN_AND_MEAN
    #include <windows.h>
#endif


// enable/disable raw mode
// https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

namespace rawterm {

    namespace detail {
        #if __linux__
            inline termios orig;
        #endif
        inline bool sigtstp_called = false;
        inline bool sigcont_called = false;
    } // namespace detail

    enum struct Mod {
        Alt_L,
        Arrow,
        Backspace,
        Control,
        Delete,
        Enter,
        Escape,
        Function,
        None,
        Shift,
        Space,
        Tab, 
        Unknown
    };

    struct Key {
        char code;
        std::deque<rawterm::Mod> mod;
        std::string raw;

        rawterm::Mod getMod();
        bool isCharInput();
    };

    //  Sequential calls to this function returns the modifiers pressed
    inline rawterm::Mod Key::getMod() {
        if (mod.empty()) {
            return rawterm::Mod::None;
        } else {
            rawterm::Mod val = mod[0];
            mod.pop_front();
            return val;
        }
    }

    // Check that the key pressed is a printable character
    inline bool Key::isCharInput() {
        return std::isprint(static_cast<unsigned char>(code)) &&
               code != ' ' && (mod.empty() || mod[0] == Mod::Shift);
    }


    // 0 == vertical == ^v, 1 == horizontal == <>
    struct Pos {
        std::size_t vertical;
        std::size_t horizontal;

	friend constexpr bool operator==(const Pos& self, const Pos& other) {
		return self.vertical == other.vertical && self.horizontal == other.horizontal;
	}
    };

    struct Color {
        std::uint8_t red;
        std::uint8_t green;
        std::uint8_t blue;
    };

    // Color presets
    inline const Color black{ 0, 0, 0 };
    inline const Color gray{ 127, 127, 127 };
    inline const Color white{ 255, 255, 255 };
    inline const Color red{ 255, 0, 0 };
    inline const Color orange{ 255, 127, 0 };
    inline const Color yellow{ 255, 255, 0 };
    inline const Color lime{ 127, 255, 0 };
    inline const Color green{ 0, 255, 0 };
    inline const Color mint{ 0, 255, 127 };
    inline const Color cyan{ 0, 255, 255 };
    inline const Color azure{ 0, 127, 255 };
    inline const Color blue{ 0, 0, 255 };
    inline const Color violet{ 127, 0, 255 };
    inline const Color purple{ 255, 0, 255 };
    inline const Color magenta{ 255, 0, 127 };

    // Enter/leave alternate screen
    // https://stackoverflow.com/a/12920036
    // Will need to find another solution for windows (#ifdef WIN32)

    void clear_screen();
    inline void move_cursor(rawterm::Pos pos);

    // Return to terminal "cooked" mode - this function doesn't need to be
    // called due to the `atexit` call
    inline void disable_raw_mode() {
        #if __linux__
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawterm::detail::orig) == -1) {
            move_cursor({ 1, 1 });
            clear_screen();
            move_cursor({ 1, 1 });
            std::perror("tcsetattr");
        }
        #elif _WIN32
	    SetConsoleOutputCP(437);
        #endif
    }

    // Switch terminal to raw mode, enabling character-level reading of input
    // without waiting for a newline character
    inline unsigned int enable_raw_mode() {
        #if __linux__
            if (tcgetattr(STDIN_FILENO, &rawterm::detail::orig) == -1) {
                std::perror("tcgetattr");
            }
            std::atexit(rawterm::disable_raw_mode);

            termios raw = rawterm::detail::orig;
            raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
            raw.c_oflag &= ~(OPOST);
            // raw.c_lflag |= ~(CS8); // Disable to allow term_size reading
            raw.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG );

            if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
                std::perror("tcsetattr");
            }
            return 0;
        #elif _WIN32
	    //https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-sgr-terminal-sequences
            DWORD in_flags = 0;
            if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &in_flags)) {
                return GetLastError();
            }
            
	    in_flags &= ~(ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT );
        in_flags |= ENABLE_VIRTUAL_TERMINAL_INPUT;

	    SetConsoleMode( GetStdHandle(STD_INPUT_HANDLE), in_flags);
	    SetConsoleOutputCP(65001); // enable utf-8
            std::atexit(rawterm::disable_raw_mode);
            return 0;
        #endif
    }

    // Switch to an alternative terminal screen -- should be supported on all
    // terminal emulators
    inline void enter_alt_screen() { std::cout << "\x1B[?1049h" << std::flush; }

    // Exit alternate screen mode
    inline void exit_alt_screen() { std::cout << "\x1B[?1049l" << std::flush; }

    static const std::unordered_set<char> asciiLetters{
        '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39',
        '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48', '\x49',
        '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F', '\x50', '\x51', '\x52',
        '\x53', '\x54', '\x55', '\x56', '\x57', '\x58', '\x59', '\x5A', '\x61',
        '\x62', '\x63', '\x64', '\x65', '\x66', '\x67', '\x68', '\x69', '\x6A',
        '\x6B', '\x6C', '\x6D', '\x6E', '\x6F', '\x70', '\x71', '\x72', '\x73',
        '\x74', '\x75', '\x76', '\x77', '\x78', '\x79', '\x7A'
    };

    // Enables the use of ctrl+c and ctrl+z
    inline bool is_signals_enabled = false;
    inline void enable_signals() {
        #if __linux__
            is_signals_enabled = true; 
        #endif
    }

    // wrapper functions for custom signal handlers
    inline void sigtstp_handler(std::function<void(void)> func) {
        if (detail::sigtstp_called) {
            func();
            detail::sigtstp_called = false;
        }
    }

    inline void sigcont_handler(std::function<void(void)> func) {
        if (detail::sigcont_called) {
            func();
            detail::sigcont_called = false;
        }
    }

    // Read user input and return a Key object ready to read the value
    inline rawterm::Key process_keypress() {

        #if __linux__
            // Backgrounding
            std::signal(SIGTSTP, [](int) {
                detail::sigtstp_called = true;
                rawterm::exit_alt_screen();
                std::raise(SIGSTOP);
            });

            // Foregrounding
            std::signal(SIGCONT, [](int) {
                detail::sigcont_called = true;
                rawterm::enter_alt_screen();
                rawterm::enable_raw_mode();
            });
        #endif

        std::string characters = std::string(32, '\0');
        if (read(STDIN_FILENO, characters.data(), 32) < 0) {
            std::perror(
                "ERROR: something went wrong during reading user input: ");
            return { ' ', { rawterm::Mod::Unknown }, "" };
        }

        std::stringstream ss;
        ss << std::hex;
        for (char c : characters.substr(0, characters.find('\0'))) {
            ss << "\\x" << static_cast<int>(static_cast<unsigned char>(c));
        }
        const std::string raw = ss.str();

        // TODO: alt-gr, multiple modifier keys?
        // NOTE: https://www.rapidtables.com/code/text/ascii-table.html
        switch (characters[0]) {
        case '\x01':
            return { 'a', { rawterm::Mod::Control }, raw };
        case '\x02':
            return { 'b', { rawterm::Mod::Control }, raw };
        case '\x03':
            if (is_signals_enabled) { raise(SIGINT); }
            return { 'c', { rawterm::Mod::Control }, raw };
        case '\x04':
            return { 'd', { rawterm::Mod::Control }, raw };
        case '\x05':
            return { 'e', { rawterm::Mod::Control }, raw };
        case '\x06':
            return { 'f', { rawterm::Mod::Control }, raw };
        case '\x07':
            return { 'g', { rawterm::Mod::Control }, raw };
        case '\x08':
            return { 'h', { rawterm::Mod::Control }, raw };
        case '\x09':
            return { 'i', { rawterm::Mod::Tab }, raw };
        case '\x0A':
            return { 'j', { rawterm::Mod::Control }, raw };
        case '\x0B':
            return { 'k', { rawterm::Mod::Control }, raw };
        case '\x0C':
            return { 'l', { rawterm::Mod::Control }, raw };
        case '\x0D':
            // Enter is ^m
            return { 'm', { rawterm::Mod::Enter }, raw };
        case '\x0E':
            return { 'n', { rawterm::Mod::Control }, raw };
        case '\x0F':
            return { 'o', { rawterm::Mod::Control }, raw };
        case '\x10':
            return { 'p', { rawterm::Mod::Control }, raw };
        case '\x11':
            return { 'q', { rawterm::Mod::Control }, raw };
        case '\x12':
            return { 'r', { rawterm::Mod::Control }, raw };
        case '\x13':
            return { 's', { rawterm::Mod::Control }, raw };
        case '\x14':
            return { 't', { rawterm::Mod::Control }, raw };
        case '\x15':
            return { 'u', { rawterm::Mod::Control }, raw };
        case '\x16':
            return { 'v', { rawterm::Mod::Control }, raw };
        case '\x17':
            return { 'w', { rawterm::Mod::Control }, raw };
        case '\x18':
            return { 'x', { rawterm::Mod::Control }, raw };
        case '\x19':
            return { 'y', { rawterm::Mod::Control }, raw };
        case '\x1A':
            #if __linux__
                if (is_signals_enabled) { raise(SIGTSTP); }
            #endif
            return { 'z', { rawterm::Mod::Control }, raw };
        case '\x1B':
            // ESCAPE

            // characters[0] is escape char
            // if characters[1] is equal to a letter (upper or lowercase), it's
            // left alt+letter if characters[1] is [, it's an arrow key (abcd
            // after for udrl) if characters[2] is \x31 (1), it can be f5+ f1
            // \x1B\x4F\x50 f2 \x1B\x4F\x51 f3 \x1B\x4F\x52 f4 \x1B\x4F\x53 f5
            // \x1B\x5B\x31\x35\x7E f6 \x1B\x5B\x31\x37\x7E f7
            // \x1B\x5B\x31\x38\x7E f8 \x1B\x5B\x31\x39\x7E f9
            // \x1B\x5B\x32\x30\x7E f10 \x1B\x5B\x32\x31\x7E f11
            // \x1B\x5B\x32\x33\x7E f12 \x1B\x5B\x32\x34\x7E delete:
            // \x1B\x5B\x33\x7E

            if (raw.size() == 4) {
                return { ' ', { rawterm::Mod::Escape }, raw }; // esc
            }

            if (raw.size() == 8 && asciiLetters.contains(characters[1])) {
                Key k = { characters[1], { rawterm::Mod::Alt_L }, raw };

                if (characters[1] >= 'A' && characters[1] <= 'Z') {
                    k.mod.push_back(rawterm::Mod::Shift);
                }

                return k;
            }
            if (characters[1] == '\x5B') {
                // ARROWS
                switch (characters[2]) {
                case '\x41':
                    return { 'A', { rawterm::Mod::Arrow }, raw }; // up
                case '\x42':
                    return { 'B', { rawterm::Mod::Arrow }, raw }; // down
                case '\x43':
                    return { 'C', { rawterm::Mod::Arrow }, raw }; // right
                case '\x44':
                    return { 'D', { rawterm::Mod::Arrow }, raw }; // left
                case '\x33':
                    return { ' ', { rawterm::Mod::Delete }, raw }; // delete

                // FUNCTIONS pt 2
                case '\x31':
                    switch (characters[3]) {
                    case '\x35':
                        return { '5', { rawterm::Mod::Function }, raw }; // f5
                    case '\x37':
                        return { '6', { rawterm::Mod::Function }, raw }; // f6
                    case '\x38':
                        return { '7', { rawterm::Mod::Function }, raw }; // f7
                    case '\x39':
                        return { '8', { rawterm::Mod::Function }, raw }; // f8
                    }
                    break;

                case '\x32':
                    switch (characters[3]) {
                    case '\x30':
                        return { '9', { rawterm::Mod::Function }, raw }; // f9
                    case '\x31':
                        return { '0', { rawterm::Mod::Function }, raw }; // f10
                    case '\x33':
                        return { '1', { rawterm::Mod::Function }, raw }; // f11
                    case '\x34':
                        return { '2', { rawterm::Mod::Function }, raw }; // f12
                    }
                    break;

                }
            } else if (raw.size() == 12 && characters[1] == '\x4F') {
                // FUNCTIONS pt 1
                switch (characters[2]) {
                case '\x50':
                    return { '1', { rawterm::Mod::Function }, raw }; // f1
                case '\x51':
                    return { '2', { rawterm::Mod::Function }, raw }; // f2
                case '\x52':
                    return { '3', { rawterm::Mod::Function }, raw }; // f3
                case '\x53':
                    return { '4', { rawterm::Mod::Function }, raw }; // f4
                }
            }
            break;

        case '\x1C':
            return { ' ', {}, raw };
        case '\x1D':
            return { ' ', {}, raw };
        case '\x1E':
            return { ' ', {}, raw };
        case '\x1F':
            return { ' ', {}, raw };
        case '\x20':
            return { ' ', { rawterm::Mod::Space }, raw };
        case '\x21':
            return { '!', {}, raw };
        case '\x22':
            return { '"', {}, raw };
        case '\x23':
            return { '#', {}, raw };
        case '\x24':
            return { '$', {}, raw };
        case '\x25':
            return { '%', {}, raw };
        case '\x26':
            return { '&', {}, raw };
        case '\x27':
            return { '\'', {}, raw };
        case '\x28':
            return { '(', {}, raw };
        case '\x29':
            return { ')', {}, raw };
        case '\x2A':
            return { '*', {}, raw };
        case '\x2B':
            return { '+', {}, raw };
        case '\x2C':
            return { ',', {}, raw };
        case '\x2D':
            return { '-', {}, raw };
        case '\x2E':
            return { '.', {}, raw };
        case '\x2F':
            return { '/', {}, raw };
        case '\x30':
            // NUMBERS
            return { '0', {}, raw };
        case '\x31':
            return { '1', {}, raw };
        case '\x32':
            return { '2', {}, raw };
        case '\x33':
            return { '3', {}, raw };
        case '\x34':
            return { '4', {}, raw };
        case '\x35':
            return { '5', {}, raw };
        case '\x36':
            return { '6', {}, raw };
        case '\x37':
            return { '7', {}, raw };
        case '\x38':
            return { '8', {}, raw };
        case '\x39':
            return { '9', {}, raw };
        case '\x3A':
            return { ':', {}, raw };
        case '\x3B':
            return { ';', {}, raw };
        case '\x3C':
            return { '<', {}, raw };
        case '\x3D':
            return { '=', {}, raw };
        case '\x3E':
            return { '>', {}, raw };
        case '\x3F':
            return { '?', {}, raw };
        case '\x40':
            return { '@', {}, raw };
        case '\x41':
            // UPPERCASE LETTERS
            return { 'A', { rawterm::Mod::Shift }, raw };
        case '\x42':
            return { 'B', { rawterm::Mod::Shift }, raw };
        case '\x43':
            return { 'C', { rawterm::Mod::Shift }, raw };
        case '\x44':
            return { 'D', { rawterm::Mod::Shift }, raw };
        case '\x45':
            return { 'E', { rawterm::Mod::Shift }, raw };
        case '\x46':
            return { 'F', { rawterm::Mod::Shift }, raw };
        case '\x47':
            return { 'G', { rawterm::Mod::Shift }, raw };
        case '\x48':
            return { 'H', { rawterm::Mod::Shift }, raw };
        case '\x49':
            return { 'I', { rawterm::Mod::Shift }, raw };
        case '\x4A':
            return { 'J', { rawterm::Mod::Shift }, raw };
        case '\x4B':
            return { 'K', { rawterm::Mod::Shift }, raw };
        case '\x4C':
            return { 'L', { rawterm::Mod::Shift }, raw };
        case '\x4D':
            return { 'M', { rawterm::Mod::Shift }, raw };
        case '\x4E':
            return { 'N', { rawterm::Mod::Shift }, raw };
        case '\x4F':
            return { 'O', { rawterm::Mod::Shift }, raw };
        case '\x50':
            return { 'P', { rawterm::Mod::Shift }, raw };
        case '\x51':
            return { 'Q', { rawterm::Mod::Shift }, raw };
        case '\x52':
            return { 'R', { rawterm::Mod::Shift }, raw };
        case '\x53':
            return { 'S', { rawterm::Mod::Shift }, raw };
        case '\x54':
            return { 'T', { rawterm::Mod::Shift }, raw };
        case '\x55':
            return { 'U', { rawterm::Mod::Shift }, raw };
        case '\x56':
            return { 'V', { rawterm::Mod::Shift }, raw };
        case '\x57':
            return { 'W', { rawterm::Mod::Shift }, raw };
        case '\x58':
            return { 'X', { rawterm::Mod::Shift }, raw };
        case '\x59':
            return { 'Y', { rawterm::Mod::Shift }, raw };
        case '\x5A':
            return { 'Z', { rawterm::Mod::Shift }, raw };
        case '\x5B':
            return { '[', {}, raw };
        case '\x5C':
            return { '\\', {}, raw };
        case '\x5D':
            return { ']', {}, raw };
        case '\x5E':
            return { '^', {}, raw };
        case '\x5F':
            return { '_', {}, raw };
        case '\x60':
            return { '`', {}, raw };
        case '\x61':
            // LOWERCASE LETTERS
            return { 'a', {}, raw };
        case '\x62':
            return { 'b', {}, raw };
        case '\x63':
            return { 'c', {}, raw };
        case '\x64':
            return { 'd', {}, raw };
        case '\x65':
            return { 'e', {}, raw };
        case '\x66':
            return { 'f', {}, raw };
        case '\x67':
            return { 'g', {}, raw };
        case '\x68':
            return { 'h', {}, raw };
        case '\x69':
            return { 'i', {}, raw };
        case '\x6A':
            return { 'j', {}, raw };
        case '\x6B':
            return { 'k', {}, raw };
        case '\x6C':
            return { 'l', {}, raw };
        case '\x6D':
            return { 'm', {}, raw };
        case '\x6E':
            return { 'n', {}, raw };
        case '\x6F':
            return { 'o', {}, raw };
        case '\x70':
            return { 'p', {}, raw };
        case '\x71':
            return { 'q', {}, raw };
        case '\x72':
            return { 'r', {}, raw };
        case '\x73':
            return { 's', {}, raw };
        case '\x74':
            return { 't', {}, raw };
        case '\x75':
            return { 'u', {}, raw };
        case '\x76':
            return { 'v', {}, raw };
        case '\x77':
            return { 'w', {}, raw };
        case '\x78':
            return { 'x', {}, raw };
        case '\x79':
            return { 'y', {}, raw };
        case '\x7A':
            return { 'z', {}, raw };
        case '\x7B':
            return { '{', {}, raw };
        case '\x7C':
            return { '|', {}, raw };
        case '\x7D':
            return { '}', {}, raw };
        case '\x7E':
            return { '~', {}, raw };
        case '\x7F':
            // BACKSPACE
            return { ' ', { rawterm::Mod::Backspace }, raw };
        }
        return { ' ', { rawterm::Mod::Unknown }, raw };
    }

    // Read the current size of the terminal window and return as a Pos object
    inline rawterm::Pos get_term_size() {
        #if __linux__
            struct winsize w;
            ioctl(0, TIOCGWINSZ, &w);
            return Pos{ w.ws_row, w.ws_col };

        #elif _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            unsigned int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            unsigned int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

            return Pos{rows, columns};
        #endif
    }

    // Move the terminal cursor to the given position, starting from 0,0
    // Note that terminals sometimes handle 0,0 and 1,1 as the same position
    inline void move_cursor(rawterm::Pos pos) {
        std::cout << "\x1B[" << std::to_string(pos.vertical) << ';'
                  << std::to_string(pos.horizontal) << 'H' << std::flush;
    }

    // Move cursor up x lines (default 1)
    inline void move_cursor_up(int in = 1) { std::cout << "\x1b[" + std::to_string(in) + "A" << std::flush; }

    // Move cursor down x lines (default 1)
    inline void move_cursor_down(int in = 1) { std::cout << "\x1b[" + std::to_string(in) + "B" << std::flush; }

    // Move cursor right x cols (default 1)
    inline void move_cursor_right(int in = 1) { std::cout << "\x1b[" + std::to_string(in) + "C" << std::flush; }

    // Move cursor left x cols (default 1)
    inline void move_cursor_left(int in = 1) { std::cout << "\x1b[" + std::to_string(in) + "D" << std::flush; }

    // Move the terminal cursor relatively to its current position
    inline void offset_cursor(std::pair<int, int> offset) {
        if (offset.first < 0) {
            std::cout << "\x1B[" << -offset.first << 'D';
        } else if (offset.first > 0) {
            std::cout << "\x1B[" << offset.first << 'C';
        }

        if (offset.second < 0) {
            std::cout << "\x1B[" << -offset.second << 'A';
        } else if (offset.second > 0) {
            std::cout << "\x1B[" << offset.second << 'B';
        }
    }

    inline void save_cursor_position() { std::cout << "\x1B[s" << std::flush; }

    inline void load_cursor_position() { std::cout << "\x1B[u" << std::flush; }

    // https://stackoverflow.com/a/48449104
    inline void cursor_block_blink() {
        std::cout << "\1\x1B[1 q\2" << std::flush;
    }

    inline void cursor_block() { std::cout << "\1\x1B[2 q\2" << std::flush; }

    inline void cursor_underscore_blink() {
        std::cout << "\1\x1B[3 q\2" << std::flush;
    }

    inline void cursor_underscore() {
        std::cout << "\1\x1B[4 q\2" << std::flush;
    }

    inline void cursor_pipe_blink() {
        std::cout << "\1\x1B[5 q\2" << std::flush;
    }

    inline void cursor_pipe() { std::cout << "\1\x1B[6 q\6" << std::flush; }

    inline void cursor_hide() { std::cout << "\x1B[?25l" << std::flush; }

    inline void cursor_show() { std::cout << "\x1B[?25h" << std::flush; }

    // Format text output in bold
    inline std::string bold(const std::string &s) {
        return "\x1B[1m" + s + "\x1B[22m";
    }

    inline std::wstring bold(const std::wstring &s) {
        return L"\x1B[1m" + s + L"\x1B[22m";
    }

    // Format text output in italics
    inline std::string italic(const std::string &s) {
        return "\x1B[3m" + s + "\x1B[23m";
    }

    inline std::wstring italic(const std::wstring &s) {
        return L"\x1B[3m" + s + L"\x1B[23m";
    }

    // Format text output underlined
    inline std::string underline(const std::string &s) {
        return "\x1B[4m" + s + "\x1B[24m";
    }

    inline std::wstring underline(const std::wstring &s) {
        return L"\x1B[4m" + s + L"\x1B[24m";
    }

    // Format text output to blink
    inline std::string blink(const std::string &s) {
        return "\x1B[5m" + s + "\x1B[25m";
    }

    inline std::wstring blink(const std::wstring &s) {
        return L"\x1B[5m" + s + L"\x1B[25m";
    }

    // Format text output with reversed colours
    inline std::string inverse(const std::string &s) {
        return "\x1B[7m" + s + "\x1B[27m";
    }

    inline std::wstring inverse(const std::wstring &s) {
        return L"\x1B[7m" + s + L"\x1B[27m";
    }

    // Format text output to be invisible
    inline std::string hidden(const std::string &s) {
        return "\x1B[8m" + s + "\x1B[28m";
    }

    inline std::wstring hidden(const std::wstring &s) {
        return L"\x1B[8m" + s + L"\x1B[28m";
    }

    // Format text output with a strikethrough
    inline std::string strikethrough(const std::string &s) {
        return "\x1B[9m" + s + "\x1B[29m";
    }

    inline std::wstring strikethrough(const std::wstring &s) {
        return L"\x1B[9m" + s + L"\x1B[29m";
    }

    // Set the foreground colour based on a given Color object
    inline std::string fg(const std::string &s, const Color color) {
        return "\x1B[38;2;" + std::to_string(color.red) + ';' +
               std::to_string(color.green) + ';' + std::to_string(color.blue) +
               'm' + s + "\x1B[39m";
    }

    inline std::wstring fg(const std::wstring &s, const Color color) {
        return L"\x1B[38;2;" + std::to_wstring(color.red) + L';' +
               std::to_wstring(color.green) + L';' + std::to_wstring(color.blue) +
               L'm' + s + L"\x1B[39m";
    }


    // Set the background colour based on a given Color object
    inline std::string bg(const std::string &s, const Color color) {
        return "\x1B[48;2;" + std::to_string(color.red) + ';' +
               std::to_string(color.green) + ';' + std::to_string(color.blue) +
               'm' + s + "\x1B[49m";
    }

    inline std::wstring bg(const std::wstring &s, const Color color) {
        return L"\x1B[48;2;" + std::to_wstring(color.red) + L';' +
               std::to_wstring(color.green) + L';' + std::to_wstring(color.blue) +
               L'm' + s + L"\x1B[49m";
    }

    // clear screen entirely
    inline void clear_screen() { std::cout << "\x1B[2J\x1B[H"; }

    // clear screen from beginning until position
    inline void clear_screen_until(const Pos pos) {
        std::cout << "\x1B[s\x1B[" << std::to_string(pos.vertical) << ';'
                  << std::to_string(pos.horizontal) << "H\x1B[1J\x1B[u";
    }

    // clear screen from position until end
    inline void clear_screen_from(const Pos pos) {
        std::cout << "\x1B[s\x1B[" << std::to_string(pos.vertical) << ';'
                  << std::to_string(pos.horizontal) << "H\x1B[0J\x1B[u";
    }

    // clear cursor's line entirely
    inline void clear_line() { std::cout << "\x1B[2K\r"; }

    // clear position's line entirely
    inline void clear_line(const Pos pos) {
        std::cout << "\x1B[s\x1B[" << std::to_string(pos.vertical)
                  << "H\x1B[2K\x1B[u";
    }

    // clear cursor's line from beginning until cursor's column
    inline void clear_line_until() { std::cout << "\x1B[1K"; }

    // clear position's line from beginning until position's column
    inline void clear_line_until(const Pos pos) {
        std::cout << "\x1B[s\x1B[" << std::to_string(pos.vertical) << ';'
                  << std::to_string(pos.horizontal) << "H\x1B[1K\x1B[u";
    }

    // clear cursor's line from cursor's column until end
    inline void clear_line_from() { std::cout << "\x1B[0K"; }

    // clear position's line from position's column until end
    inline void clear_line_from(const Pos pos) {
        std::cout << "\x1B[s\x1B[" << std::to_string(pos.vertical) << ';'
                  << std::to_string(pos.horizontal) << "H\x1B[0K\x1B[u";
    }


    // Convert a Mod enum to a string
    inline std::string to_string(const Mod m) {
        switch (m) {
        case Mod::Alt_L:
            return "Alt_L";
        case Mod::Arrow:
            return "Arrow";
        case Mod::Backspace:
            return "Backspace";
        case Mod::Control:
            return "Control";
        case Mod::Delete:
            return "Delete";
        case Mod::Enter:
            return "Enter";
        case Mod::Escape:
            return "Escape";
        case Mod::Function:
            return "Function";
        case Mod::None:
            return "None";
        case Mod::Shift:
            return "Shift";
        case Mod::Space:
            return "Space";
        case Mod::Tab:
            return "Tab";
        case Mod::Unknown:
            return "Unknown";
        }
        
        return "";
    }

    inline std::wstring to_wstring(const Mod m) {
        switch (m) {
        case Mod::Alt_L:
            return L"Alt_L";
        case Mod::Arrow:
            return L"Arrow";
        case Mod::Backspace:
            return L"Backspace";
        case Mod::Control:
            return L"Control";
        case Mod::Delete:
            return L"Delete";
        case Mod::Enter:
            return L"Enter";
        case Mod::Escape:
            return L"Escape";
        case Mod::Function:
            return L"Function";
        case Mod::None:
            return L"None";
        case Mod::Shift:
            return L"Shift";
        case Mod::Space:
            return L"Space";
        case Mod::Tab:
            return L"Tab";
        case Mod::Unknown:
            return L"Unknown";
        }
    }

} // namespace rawterm

#endif // RAWTERM_H
