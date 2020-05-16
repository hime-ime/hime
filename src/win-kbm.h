/**
 @file win-kbm.h
 @brief Virtual keyboard definition

 Some rare users maybe need to translate those key defines.
 So we kept those N_("stuff").

 Note that our po/Makefile do not search .h files so those
 strings will not present (by default) in .pot nor .po files.

*/

#define COLN 19

static KEY keys[][COLN] = {
    {{XK_Escape, N_ ("Esc")}, {XK_F1, N_ ("F1")}, {XK_F2, N_ ("F2")}, {XK_F3, N_ ("F3")}, {XK_F4, N_ ("F4")}, {XK_F5, N_ ("F5")}, {XK_F6, N_ ("F6")}, {XK_F7, N_ ("F7")}, {XK_F8, N_ ("F8")}, {XK_F9, N_ ("F9")}, {XK_F10, N_ ("F10")}, {XK_F11, N_ ("F11")}, {XK_F12, N_ ("F12")}, {XK_Print, N_ ("Pr"), 0, 8}, {XK_Scroll_Lock, N_ ("Slk"), 0, 8}, {XK_Pause, N_ ("Pau"), 0, 8}},

    {{'`', N_ (" ` "), '~'}, {'1', N_ (" 1 "), '!'}, {'2', N_ (" 2 "), '@'}, {'3', N_ (" 3 "), '#'}, {'4', N_ (" 4 "), '$'}, {'5', N_ (" 5 "), '%'}, {'6', N_ (" 6 "), '^'}, {'7', N_ (" 7 "), '&'}, {'8', N_ (" 8 "), '*'}, {'9', N_ (" 9 "), '('}, {'0', N_ (" 0 "), ')'}, {'-', N_ (" - "), '_'}, {'=', N_ (" = "), '+'}, {XK_BackSpace, N_ ("←"), 0, K_FILL}, {XK_Insert, N_ ("Ins"), 0, 8}, {XK_Home, N_ ("Ho"), 0, 8}, {XK_Prior, N_ ("P↑"), 0, 8}},

    {{XK_Tab, N_ ("Tab")}, {'q', N_ (" q ")}, {'w', N_ (" w ")}, {'e', N_ (" e ")}, {'r', N_ (" r ")}, {'t', N_ (" t ")}, {'y', N_ (" y ")}, {'u', N_ (" u ")}, {'i', N_ (" i ")}, {'o', N_ (" o ")}, {'p', N_ (" p ")}, {'[', N_ (" [ "), '{'}, {']', N_ (" ] "), '}'}, {'\\', N_ (" \\ "), '|', K_FILL}, {XK_Delete, N_ ("Del"), 0, 8}, {XK_End, N_ ("En"), 0, 8}, {XK_Next, N_ ("P↓"), 0, 8}},

    {{XK_Caps_Lock, N_ ("Caps"), 0, K_CAPSLOCK}, {'a', N_ (" a ")}, {'s', N_ (" s ")}, {'d', N_ (" d ")}, {'f', N_ (" f ")}, {'g', N_ (" g ")}, {'h', N_ (" h ")}, {'j', N_ (" j ")}, {'k', N_ (" k ")}, {'l', N_ (" l ")}, {';', N_ (" ; "), ':'}, {'\'', N_ (" ' "), '"'}, {XK_Return, N_ (" Enter "), 0, 1}, {XK_Num_Lock, N_ ("Num"), 0, 8}, {XK_KP_Add, N_ (" + "), 0, 8}},

    {{XK_Shift_L, N_ ("  Shift  "), 0, K_HOLD}, {'z', N_ (" z ")}, {'x', N_ (" x ")}, {'c', N_ (" c ")}, {'v', N_ (" v ")}, {'b', N_ (" b ")}, {'n', N_ (" n ")}, {'m', N_ (" m ")}, {',', N_ (" , "), '<'}, {'.', N_ (" . "), '>'}, {'/', N_ (" / "), '?'}, {XK_Shift_R, N_ (" Shift"), 0, K_HOLD | K_FILL}, {XK_KP_Multiply, N_ (" * "), 0, 8}, {XK_Up, N_ ("↑"), 0, 8}},
    {{XK_Control_L, N_ ("Ctrl"), 0, K_HOLD}, {XK_Super_L, N_ ("◆")}, {XK_Alt_L, N_ ("Alt"), 0, K_HOLD}, {' ', N_ ("Space"), 0, 1}, {XK_Alt_R, N_ ("Alt"), 0, K_HOLD}, {XK_Super_R, N_ ("◆")}, {XK_Menu, N_ ("■")}, {XK_Control_R, N_ ("Ctrl"), 0, K_HOLD}, {XK_Left, N_ ("←"), 0, 8}, {XK_Down, N_ ("↓"), 0, 8}, {XK_Right, N_ ("→"), 0, 8}}};
