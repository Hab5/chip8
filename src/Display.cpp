#include "Display.hpp"


Display::Display(Chip8& c8)
: c8(c8), assembly(Disassembler::Disassemble(c8.filename)) {

    std::setlocale(LC_ALL, "en_US.UTF-8"); // Proper unicode
    initscr();                             // Init ncurses

    ////////// INPUT SETTINGS ////////

    curs_set(0);           // Invisible cursor
    noecho();              // Disable input echoing
    cbreak();              // Disable line buffering
    nodelay(stdscr, true); // Non Blocking getch()
    keypad(stdscr, true);  // Function keys

    ///// WINDOWS INITIALIZATION /////

    left  = newwin(SCREEN_HEIGHT/2+2, 23, 0, 0);                 // Variables
    main  = newwin(SCREEN_HEIGHT/2+2, SCREEN_WIDTH+2, 0, 23);    // Emulator
    right = newwin(SCREEN_HEIGHT/2+2, 23, 0, SCREEN_WIDTH+2+23); // Assembly

    wattrset(left, A_BOLD);
    wattrset(main, A_BOLD);
    wattrset(right, A_BOLD);

    //////// COLOR DEFINITION ///////

    use_default_colors(); start_color();

    init_pair(1, 232, 232); // NO PIXELS
    init_pair(2, 232, 114); // 0 TOP, 1 BOTTOM
    init_pair(3, 114, 232); // 1 TOP, 0 BOTTOM
    init_pair(4, 114, 114); // 1 TOP, 1 BOTTOM

    init_pair(5, 114, -1);  // NAME
    init_pair(6, 204, -1);  // VALUE
    init_pair(7, 194, -1);  // REGISTER ASM
    init_pair(8, 223, -1);  // HEX ASM
    init_pair(9, 230, -1);  // OTHER ASM

    // assembly = Disassembler::Disassemble(c8.filename);
}

Display::~Display() { endwin(); }

void Display::Refresh() const {
    UserInput();
    LeftPannel();
    MainPannel();
    RightPannel();
    refresh();
}


void Display::UserInput() const {
    int input = getch();

    if (input == ERR) return;

    if      (input == 27)  c8.quit   = true;                                            // Del
    else if (input == 32 ) c8.paused ^= 1;                                              // Space
    else if (input == 9  ) c8.step   = true;                                            // Tab
    else if (input == 10 ) c8.Reset();                                                  // Enter
    else if (input == 45 ) c8.cycle_speed -= c8.cycle_speed > 20.0f   ? 20.0f : 0.0f;   // Minus
    else if (input == 43 ) c8.cycle_speed += c8.cycle_speed < 9000.0f ? 20.0f : 0.0f;   // Plus
    else                   c8.hexpad.SetKey(input);                                     // Hexpad
}

// Emulator
void Display::MainPannel() const {
    if (c8.ST > 0) wattron(main, COLOR_PAIR(6)); // Color box for sound timer
    box(main,  0, 0);
    mvwprintw(main, 0, 2, "[%s]", c8.filename.c_str());
    mvwprintw(main, 17, 2,
        "[esc->quit]─[enter->reset]─[spc->pause]─[tab->step]─[-%.0fHz+]", c8.cycle_speed);

    // halfblock char + bg color for correct aspect ratio
    for (int row = 0; row < SCREEN_HEIGHT; row+=2) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            auto pixel_top = c8.GetPixel(col, row  );
            auto pixel_bot = c8.GetPixel(col, row+1);
            auto color = (pixel_bot << 1 | pixel_top) + 1;
            wattron(main, COLOR_PAIR(color));
            mvwaddwstr(main, row/2+1, col+1, L"▄");
            wattroff(main, COLOR_PAIR(color));
        }
    }
    wrefresh(main);
}

// Variables
void Display::LeftPannel() const {
    auto field = [this](int y, int x1, const char* name, int x2, int val, int hex_f=0x02) {
        wattron(left, COLOR_PAIR(5));
        mvwprintw(left, y, x1, name);
        wattron(left, COLOR_PAIR(6));
        mvwprintw(left, y, x2, (hex_f == 2) ? "0x%02x" : "0x%04x", val);
        wattroff(left, COLOR_PAIR(6));
    };

    auto sep = [this](int y, int x, const char* wchar) {
        mvwprintw(left, y, x, wchar);
    };

    auto stack_val = [this](int depth) {
        return ((int)c8.stack.size() >= depth) ? c8.stack.back()-depth : 0xdead;
    };

    auto keypad_row_keys =
        [this](int n, const char* k1, const char* k2, const char* k3, const char* k4) {
            int i = 0;
            for (auto k: {k1, k2, k3, k4}) {
                wattron(left, COLOR_PAIR(c8.hexpad.GetKeyFromMap((char)k[0]) ? 5 : 6));
                mvwprintw(left, 13+n, 13+i, k); i+=2;
            } wattroff(left, COLOR_PAIR(6));
        };

    werase(left);
    box(left,  0, 0);

    mvwprintw(left, 0, 3, "┐REG┌──┬");                 mvwprintw(left, 0, 12, "┐VARIOUS┌");
    field(1 , 2, "v0", 5, c8.V[0]);  sep(1 , 10, "│"); field(1 , 12, "OP", 15, c8.OP, 0x04);
    field(2 , 2, "v1", 5, c8.V[1]);  sep(2 , 10, "│"); field(2 , 12, "PC", 15, c8.PC-2, 0x04);
    field(3 , 2, "v2", 5, c8.V[2]);  sep(3 , 10, "│"); field(3 , 12, "I",  15, c8.I, 0x04);
    field(4 , 2, "v3", 5, c8.V[3]);  sep(4 , 10, "│"); field(4 , 12, "DT", 15, c8.DT, 0x04);
    field(5 , 2, "v4", 5, c8.V[4]);  sep(5 , 10, "│"); field(5 , 12, "ST", 15, c8.ST, 0x04);
    field(6 , 2, "v5", 5, c8.V[5]);  sep(6 , 10, "│"); mvwprintw(left, 6 , 11, "──┐STACK┌──");
    field(7 , 2, "v6", 5, c8.V[6]);  sep(7 , 10, "│"); field(7 , 12, "s1", 15, stack_val(1), 0x04);
    field(8 , 2, "v7", 5, c8.V[7]);  sep(8 , 10, "│"); field(8 , 12, "s2", 15, stack_val(2), 0x04);
    field(9 , 2, "v8", 5, c8.V[8]);  sep(9 , 10, "│"); field(9 , 12, "s3", 15, stack_val(3), 0x04);
    field(10, 2, "v9", 5, c8.V[9]);  sep(10, 10, "│"); field(10, 12, "s4", 15, stack_val(4), 0x04);
    field(11, 2, "va", 5, c8.V[10]); sep(11, 10, "│"); field(11, 12, "s5", 15, stack_val(5), 0x04);
    field(12, 2, "vb", 5, c8.V[11]); sep(12, 10, "│"); mvwprintw(left, 12, 11, "───┐PAD┌───");
    field(13, 2, "vc", 5, c8.V[12]); sep(13, 10, "│"); keypad_row_keys(0, "1 ", "2 ", "3 ", "4");
    field(14, 2, "vd", 5, c8.V[13]); sep(14, 10, "│"); keypad_row_keys(1, "q ", "w ", "e ", "r");
    field(15, 2, "ve", 5, c8.V[14]); sep(15, 10, "│"); keypad_row_keys(2, "a ", "s ", "d ", "f");
    field(16, 2, "vf", 5, c8.V[15]); sep(16, 10, "│"); keypad_row_keys(3, "z ", "x ", "c ", "v");
    mvwprintw(left, 6 ,  10, "├"); mvwprintw(left, 6 , 22, "┤");
    mvwprintw(left, 12,  10, "├"); mvwprintw(left, 12, 22, "┤");
    mvwprintw(left, 17, 10, "┴");

    wrefresh(left);
}

// Assembly
void Display::RightPannel() const {

    auto idx = std::abs(c8.PC-2 - ENTRY_POINT) / 2;
    static int old_idx;

    auto format_assembly = [this, idx](auto offset) {
        auto line = assembly[idx+offset];

        // Address
        wattron(right, COLOR_PAIR(5));
        mvwprintw(right, offset+1, 2, "%04x", (c8.PC-2)+offset*2);

        // Mmemonics
        wattron(right, COLOR_PAIR(6));
        mvwprintw(right, offset+1, 7,  "%s", std::string(line.begin(), line.begin()+4).c_str());

        // Split Arguments
        auto begin = 0;
        auto args = std::string(line.begin()+5, line.end());
        auto tokens = std::vector<std::string>(); tokens.reserve(3);
        for (auto end=0; (size_t)(end = args.find(',', end)) != std::string::npos; ++end) {
            tokens.push_back(args.substr(begin, end-begin));
            begin = end+1;
        } tokens.push_back(args.substr(begin));

        // Print Arguments
        auto args_offset = 0; auto count = 0;
        for (auto token: tokens) {
            auto color = 0;

            if      (token[0] == 'v') color = 7;
            else if (token[0] == '$') color = 8;
            else                      color = 9;

            wattron(right, COLOR_PAIR(color));
            mvwprintw(right, offset+1, 13+args_offset, "%s%c", token.c_str(),
                      (count++ < (int)tokens.size()-1 ? ',' : 0));
            args_offset += token.length()+1;
            wattroff(right, COLOR_PAIR(color));
        }
    };

    // Only refresh if current instruction isn't on screen already
    if (idx < old_idx || (idx - old_idx) > 15 || idx == 0) {
        werase(right);
        for (auto i = 0; i < 16; i++)
            if (idx+i < (int)assembly.size())
                format_assembly(i);
        old_idx = idx;
    }

    box(right, 0, 0);
    mvwprintw(right, 0 , 1, "┐ASSEMBLY┌");

    // Highlight current instruction
    mvwchgat(right, idx-old_idx+1, 7, 4, A_STANDOUT | A_BOLD | A_DIM, 6, nullptr);
    wrefresh(right);
    mvwchgat(right, idx-old_idx+1, 7, 4, A_BOLD, 6, nullptr);
}
