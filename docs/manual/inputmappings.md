# Input Mappings

Below is a list of all valid key and button mappings in Cacao Engine:
```{code-block} cpp
//======================= KEYBOARD MAPPINGS =======================
//Any printable characters use their ASCII codes
//The other ones are just made up and their explanations will be noted
//Any keys with "KP" are keys featured on the keypad/numpad
//The "super" key is a Linux term; this is the Windows key or the Command key on macOS
#define CACAO_KEY_ENTER 10
#define CACAO_KEY_ESCAPE 27
#define CACAO_KEY_BACKSPACE 8
#define CACAO_KEY_TAB 9
#define CACAO_KEY_SPACE 32
#define CACAO_KEY_APOSTROPHE 39
#define CACAO_KEY_COMMA 44
#define CACAO_KEY_MINUS 45
#define CACAO_KEY_EQUALS 61
#define CACAO_KEY_PERIOD 46
#define CACAO_KEY_SLASH 47
#define CACAO_KEY_0 48
#define CACAO_KEY_1 49
#define CACAO_KEY_2 50
#define CACAO_KEY_3 51
#define CACAO_KEY_4 52
#define CACAO_KEY_5 53
#define CACAO_KEY_6 54
#define CACAO_KEY_7 55
#define CACAO_KEY_8 56
#define CACAO_KEY_9 57
#define CACAO_KEY_SEMICOLON 59
#define CACAO_KEY_LEFT_BRACKET 91
#define CACAO_KEY_RIGHT_BRACKET 93
#define CACAO_KEY_BACKSLASH 92
#define CACAO_KEY_GRAVE_ACCENT 96
#define CACAO_KEY_A 97
#define CACAO_KEY_B 98
#define CACAO_KEY_C 99
#define CACAO_KEY_D 100
#define CACAO_KEY_E 101
#define CACAO_KEY_F 102
#define CACAO_KEY_G 103
#define CACAO_KEY_H 104
#define CACAO_KEY_I 105
#define CACAO_KEY_J 106
#define CACAO_KEY_K 107
#define CACAO_KEY_L 108
#define CACAO_KEY_M 109
#define CACAO_KEY_N 110
#define CACAO_KEY_O 111
#define CACAO_KEY_P 112
#define CACAO_KEY_Q 113
#define CACAO_KEY_R 114
#define CACAO_KEY_S 115
#define CACAO_KEY_T 116
#define CACAO_KEY_U 117
#define CACAO_KEY_V 118
#define CACAO_KEY_W 119
#define CACAO_KEY_X 120
#define CACAO_KEY_Y 121
#define CACAO_KEY_Z 122
#define CACAO_KEY_CAPS_LOCK 227   //Extended ASCII pi symbol, chosen at random
//The F_ keys follow the pattern of a, e, i, o, u, each with an acute and then a grave accent.
#define CACAO_KEY_F1 160
#define CACAO_KEY_F2 133
#define CACAO_KEY_F3 130
#define CACAO_KEY_F4 138
#define CACAO_KEY_F5 161
#define CACAO_KEY_F6 141
#define CACAO_KEY_F7 162
#define CACAO_KEY_F8 149
#define CACAO_KEY_F9 163
#define CACAO_KEY_F10 151
#define CACAO_KEY_F11 174
#define CACAO_KEY_F12 175
#define CACAO_KEY_PRINT_SCREEN 17 //ASCII device control 1 (since this is a kind of device function)
#define CACAO_KEY_SCROLL_LOCK 18  //ASCII device control 2 (since this is a kind of device function)
#define CACAO_KEY_PAUSE 19		  //ASCII device control 3 (since this is a kind of device function)
#define CACAO_KEY_INSERT 13		  //ASCII carraige return, chosen because in insert mode, you overwrite things like if you kept typing after a carraige return
#define CACAO_KEY_DELETE 127	  //ASCII delete
#define CACAO_KEY_HOME 60		  //ASCII <, chosen because home moves cursor to the left
#define CACAO_KEY_PAGE_UP 2		  //ASCII start of text, since text starts at the top of a page
#define CACAO_KEY_END 62		  //ASCII >, chosen because end moves cursor to the right
#define CACAO_KEY_PAGE_DOWN 3	  //ASCII end of text, since text starts at the bottom of a page
#define CACAO_KEY_RIGHT 175		  //Extended ASCII right French quotation marks (since they point right)
#define CACAO_KEY_LEFT 174		  //Extended ASCII left French quotation marks (since they point left)
#define CACAO_KEY_DOWN 24		  //ASCII cancel, chosen at random
#define CACAO_KEY_UP 94			  //ASCII caret (since it points up)
#define CACAO_KEY_NUM_LOCK 20	  //ASCII device control 4 (since this is a kind of device function)
#define CACAO_KEY_KP_DIVIDE 246	  //Extended ASCII divide symbol
#define CACAO_KEY_KP_MULTIPLY 42  //ASCII asterisk, since that's the multiplication operator
#define CACAO_KEY_KP_MINUS 95	  //ASCII underscore, since it's on the minus key but minus is already taken
#define CACAO_KEY_KP_PLUS 43	  //ASCII plus
#define CACAO_KEY_KP_ENTER 12	  //ASCII new page, chosen because it's another form of moving down
//The keypad numbers are uppercase letters since those aren't already taken, and we just use the ones in that range with the same ending digit
#define CACAO_KEY_KP_1 71
#define CACAO_KEY_KP_2 72
#define CACAO_KEY_KP_3 73
#define CACAO_KEY_KP_4 74
#define CACAO_KEY_KP_5 75
#define CACAO_KEY_KP_6 76
#define CACAO_KEY_KP_7 77
#define CACAO_KEY_KP_8 78
#define CACAO_KEY_KP_9 79
#define CACAO_KEY_KP_0 70
#define CACAO_KEY_KP_PERIOD 248   //Extended ASCII degree symbol, chosen at random
//The modifier keys are just other uppercase letters that aren't taken, no reason
#define CACAO_KEY_LEFT_CONTROL 80
#define CACAO_KEY_LEFT_SHIFT 81
#define CACAO_KEY_LEFT_ALT 82
#define CACAO_KEY_LEFT_SUPER 83
#define CACAO_KEY_RIGHT_CONTROL 84
#define CACAO_KEY_RIGHT_SHIFT 85
#define CACAO_KEY_RIGHT_ALT 86
#define CACAO_KEY_RIGHT_SUPER 87

//======================= MOUSE BUTTON MAPPINGS =======================
#define CACAO_BUTTON_LEFT 0
#define CACAO_BUTTON_MIDDLE 2
#define CACAO_BUTTON_RIGHT 1
```