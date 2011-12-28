#define COLN 19
static KEY keys[][COLN]={
{{XK_Escape,_L("Esc")},{XK_F1,_L("F1")},{XK_F2,_L("F2")},{XK_F3,_L("F3")},{XK_F4,_L("F4")},{XK_F5,_L("F5")},{XK_F6,_L("F6")},{XK_F7,_L("F7")},{XK_F8,_L("F8")},{XK_F9,_L("F9")},{XK_F10,_L("F10")},{XK_F11,_L("F11")},{XK_F12,_L("F12")},{XK_Print,_L("Pr"),0,8},{XK_Scroll_Lock,_L("Slk"),0,8},{XK_Pause,_L("Pau"),0,8}},

{{'`',_L(" ` "),'~'},{'1',_L(" 1 "), '!'},{'2',_L(" 2 "),'@'},{'3',_L(" 3 "),'#'},{'4',_L(" 4 "),'$'},{'5',_L(" 5 "),'%'},{'6',_L(" 6 "),'^'},{'7',_L(" 7 "),'&'},{'8',_L(" 8 "),'*'},{'9',_L(" 9 "),'('},{'0',_L(" 0 "),')'},{'-',_L(" - "),'_'},{'=',_L(" = "),'+'},
{XK_BackSpace,_L("←"),0, K_FILL},
{XK_Insert,_L("Ins"),0,8},{XK_Home,_L("Ho"),0,8}, {XK_Prior,_L("P↑"),0,8}},

{{XK_Tab, _L("Tab")}, {'q',_L(" q ")},{'w',_L(" w ")},{'e',_L(" e ")},{'r',_L(" r ")},{'t',_L(" t ")}, {'y',_L(" y ")},{'u',_L(" u ")},{'i',_L(" i ")},{'o',_L(" o ")}, {'p',_L(" p ")},{'[',_L(" [ "),'{'},{']',_L(" ] "),'}'},{'\\',_L(" \\ "),'|',K_FILL},{XK_Delete,_L("Del"),0,8},{XK_End,_L("En"),0,8},
{XK_Next,_L("P↓"),0,8}},

{{XK_Caps_Lock, _L("Caps"), 0, K_CAPSLOCK},{'a',_L(" a ")},{'s',_L(" s ")},{'d',_L(" d ")},{'f', _L(" f ")},{'g',_L(" g ")},{'h',_L(" h ")},{'j',_L(" j ")},{'k',_L(" k ")},{'l',_L(" l ")},{';',_L(" ; "),':'},{'\'',_L(" ' "),'"'},{XK_Return,_L(" Enter "),0,1},{XK_Num_Lock,_L("Num"),0,8},{XK_KP_Add,_L(" + "),0,8}},

{{XK_Shift_L,_L("  Shift  "),0,K_HOLD},{'z',_L(" z ")},{'x',_L(" x ")},{'c',_L(" c ")},{'v',_L(" v ")},{'b',_L(" b ")},{'n',_L(" n ")},{'m',_L(" m ")},{',',_L(" , "),'<'},{'.',_L(" . "),'>'},{'/',_L(" / "),'?'},{XK_Shift_R,_L(" Shift"),0,K_HOLD|K_FILL},{XK_KP_Multiply,_L(" * "),0,8},
{XK_Up,_L("↑"),0,8}},
{{XK_Control_L,_L("Ctrl"),0,K_HOLD},{XK_Super_L,_L("◆")},{XK_Alt_L,_L("Alt"),0,K_HOLD},{' ',_L("Space"),0,1}, {XK_Alt_R,_L("Alt"),0,K_HOLD},{XK_Super_R,_L("◆")},{XK_Menu,_L("■")}, {XK_Control_R,_L("Ctrl"),0,K_HOLD},
{XK_Left, _L("←"),0,8},{XK_Down,_L("↓"),0,8},{XK_Right, _L("→"),0,8}}
};
