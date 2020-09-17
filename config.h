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
   // Axis       Mod Switches Func              Arg
    { GPA_LJ_X,  0,  0,       mouse_movement,   { .us=X            } },
    { GPA_LJ_Y,  0,  0,       mouse_movement,   { .us=Y            } },
    { GPA_LB,    0,  0,       mouse_multiplier, { 0                } },
    { GPA_RB,    0,  1,       send_key,         { .us=KEY_LEFTCTRL } },
    { GPA_RJ_X,  0,  0,       mouse_scroll,     { .us=X            } },
    { GPA_RJ_Y,  0,  0,       mouse_scroll,     { .us=Y            } },
};