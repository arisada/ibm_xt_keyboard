/* Convert IBM XT keyboard to USB
 * References:
 * https://github.com/NicoHood/HID/blob/master/src/KeyboardLayouts/ImprovedKeylayouts.h#L67
 * https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf 
 * https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
 */

#define LAYOUT_US_ENGLISH
#define HID_CUSTOM_LAYOUT
#include <HID-Project.h>
#include <HID-Settings.h>

#define CLK 2
#define DATA 3
#define RST 4

//#define HAVE_SERIAL

int clk_state;
int state;
int scancode;
KeyboardKeycode codes[] = {
  /* 0 */
  KEY_ERROR_UNDEFINED /* error */, KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6,
  KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB,
  /* 0x10 */
  KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, 
  KEY_O, KEY_P, KEY_LEFT_BRACE, KEY_RIGHT_BRACE, KEY_ENTER, KEY_LEFT_CTRL, KEY_A, KEY_S,
  /* 0x20 */
  KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE,
  KEY_BACKSLASH, KEY_LEFT_SHIFT, KEY_NON_US /*FIXME*/, KEY_Z, KEY_X, KEY_C, KEY_V,
  /* 0x30 */
  KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RIGHT_SHIFT, KEYPAD_MULTIPLY,
  KEY_LEFT_ALT, KEY_SPACE, /* CAPS lock but let's make it ALTGR instead */ KEY_RIGHT_ALT, KEY_F1,
  KEY_F2, KEY_F3, KEY_F4, KEY_F5,
  /* 0x40 */
  KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUM_LOCK, KEY_SCROLL_LOCK, KEYPAD_7,
  KEYPAD_8, KEYPAD_9, KEYPAD_SUBTRACT, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_ADD, KEYPAD_1,
  /* 0x50 */
  KEYPAD_2, KEYPAD_3, KEYPAD_0, KEYPAD_DOT
};
#define NCODES (sizeof(codes)/sizeof(codes[0]))

void setup() {
#ifdef HAVE_SERIAL
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
#endif
  pinMode(CLK, INPUT);
  pinMode(DATA, INPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  clk_state = digitalRead(CLK);
  state = 0;
#ifdef HAVE_SERIAL
  Serial.println("Hello! \n");
#else
  Keyboard.begin();
#endif
  digitalWrite(RST, HIGH);
}

void reset(){
  digitalWrite(RST, LOW);
  delay(50);
  digitalWrite(RST, HIGH);
}

void down(){
}

void do_scan(){
  char buffer[64];
  int header = scancode & 1;
  int code = (scancode >> 1) & 0x7f;
  int key_release = (scancode >> 8) & 1;
#ifdef HAVE_SERIAL
  sprintf(buffer, " hdr: %.1x keycode: %.2x release: %.1x", header, code, key_release);
  Serial.println(buffer);
#else
  if (code < NCODES){
    KeyboardKeycode k = codes[code];
    if (key_release){
      Keyboard.release(k);
    } else {
      Keyboard.press(k);
    }
  }
#endif
}

void up() {
  int data;
  data = digitalRead(DATA);
#ifdef HAVE_SERIAL
  if(data == HIGH){
    Serial.print('1');
  } else {
    Serial.print('0');
  }
#endif
  if (state == 0 && data != HIGH){
    /* Attempt to resync */
    reset();
#ifdef HAVE_SERIAL
    Serial.println("Lost sync");
#endif
    return;
  }
  scancode |= (data==HIGH ? 1:0) << state;
  if (state == 9){
    do_scan();
    state = 0;
    scancode = 0;
  } else {
    state++;
  }
}

void loop() {
  int state = digitalRead(CLK);
  if (state != clk_state){
    clk_state = state;
    if (state == HIGH){
      up();
    } else {
      down();
    }
  }
}
