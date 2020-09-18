static const int    loop_delay = 1e6/60; // Nanoseconds between sending mouse events (1e6/<cycles per second>)
static const int    deadzone   = 8000;   // Below what value (0-65k) stick/bumper should not trigger events
static const double mouse_slow = 7.0;    // Lowest mouse speed multiplier
static const double mouse_fast = 20.0;   // Greatest mouse speed multiplier

static ButtonRule btnRules[] = {
   // Btn       Mask    Func                      Arg
    { GPB_LD,   MOD_RT, send_key,                 { .us=KEY_G         } },
    { GPB_RD,   MOD_RT, send_key,                 { .us=KEY_H         } },
    { GPB_UD,   MOD_RT, send_key,                 { .us=KEY_J         } },
    { GPB_DD,   MOD_RT, send_key,                 { .us=KEY_K         } },
    { GPB_A,    MOD_RT, morse_input,              { .us=MORSE_SHORT   } },
    { GPB_B,    MOD_RT, morse_input,              { .us=MORSE_LONG    } },
    { GPB_X,    MOD_RT, morse_reset_or_backspace, { 0                 } },
    { GPB_Y,    MOD_RT, morse_write_or_space,     { 0                 } },
    // { GPB_RT,   0,      morse_reset,              { 0                 } },
    { GPB_A,    0,      send_key,                 { .us=BTN_LEFT      } },
    { GPB_B,    0,      send_key,                 { .us=BTN_RIGHT     } },
    { GPB_X,    0,      send_key,                 { .us=KEY_BACKSPACE } },
    { GPB_Y,    0,      send_key,                 { .us=KEY_SPACE     } },
    { GPB_STRT, 0,      send_key,                 { .us=KEY_ENTER     } },
    { GPB_LJ,   0,      send_key,                 { .us=KEY_ESC       } },
    { GPB_BCK,  0,      send_key,                 { .us=KEY_LEFTMETA  } },
    { GPB_RJ,   0,      send_key,                 { .us=BTN_MIDDLE    } },
    { GPB_LD,   0,      send_key,                 { .us=KEY_LEFT      } },
    { GPB_RD,   0,      send_key,                 { .us=KEY_RIGHT     } },
    { GPB_UD,   0,      send_key,                 { .us=KEY_UP        } },
    { GPB_DD,   0,      send_key,                 { .us=KEY_DOWN      } },
};

static AxisRule axisRules[] = {
   // Axis      Mask Switches Func              Arg
    { GPA_LJ_X, 0,   0,       mouse_movement,   { .us=X            } },
    { GPA_LJ_Y, 0,   0,       mouse_movement,   { .us=Y            } },
    { GPA_LB,   0,   0,       mouse_multiplier, { 0                } },
    { GPA_RB,   0,   1,       send_key,         { .us=KEY_LEFTCTRL } },
    { GPA_RJ_X, 0,   0,       mouse_scroll,     { .us=X            } },
    { GPA_RJ_Y, 0,   0,       mouse_scroll,     { .us=Y            } },
};

// Binary tree of all morse combinations
// 0-bit is short and 1-bit is long:
// Eg: .-. -> 010 (2), length:3 -> 3rd row, index 2 -> R
// Eg: .--- -> 0111 (7), length:4 -> 4th row, index 7 -> J
// Put 0 for undefined combinations, each row must be twice the length of previous
static const unsigned short morse_tree[] = {
    KEY_E, KEY_T,
    KEY_I, KEY_A, KEY_N, KEY_M,
    KEY_S, KEY_U, KEY_R, KEY_W, KEY_D, KEY_K, KEY_G, KEY_O,
    KEY_H, KEY_V, KEY_F, 0, KEY_L, 0, KEY_P, KEY_J, KEY_B, KEY_X, KEY_C, KEY_Y, KEY_Z, KEY_Q, 0, 0,
    KEY_5, KEY_4, 0, KEY_3, 0, 0, 0, KEY_2, 0, 0, KEY_KPPLUS, 0, 0, 0, 0, KEY_1, KEY_6, KEY_EQUAL, KEY_SLASH, 0, 0, 0, 0, 0, KEY_7, 0, 0, 0, KEY_8, 0, KEY_9, KEY_0, 0 };