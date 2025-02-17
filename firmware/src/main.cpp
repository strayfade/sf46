//#define KEYBOARD_LEFTSIDE

#define KEYBOARD_DEBOUNCE_MS 50

#define KEY_MOD_1 0xFD
#define KEY_MOD_2 0xFE

#include <Arduino.h>
#include <vector>
#include "../include/BleKeyboard.h"

BleKeyboard bleKeyboard("sf46", "Noah Taylor");

class key {
public:
  uint8_t code;
  bool isPressed = false;
  long lastStateChange = 0;
  key() {
    code = 0;
  }
  key(int keyCode) {
    code = keyCode;
  }
};

#define keymap_t std::vector<std::vector<key>>
struct keyboard_t {
  std::vector<char> rows = { 5, 6, 7, 4 };
  std::vector<char> cols = { 3, 2, 1, 0, 21, 20 };
  std::vector<keymap_t> layers;
  int modifier = 0;
} keyboard;

void onKeyStateChange(key* ref) {
  Serial.print(keyboard.modifier);
  Serial.print(" : ");
  Serial.print(ref->lastStateChange);
  Serial.print(" : ");
  Serial.print(ref->isPressed ? "Pressed" : "Release");
  Serial.print(" : ");
  Serial.println(ref->code);

  uint8_t keyCode = ref->code;
  if (keyCode == 0) return;

  switch (keyCode) {
    case KEY_MOD_1:
      keyboard.modifier = ref->isPressed ? 1 : 0;
      return;
    case KEY_MOD_2:
      keyboard.modifier = ref->isPressed ? 2 : 0;
      return;
  }
  
  if (ref->isPressed) {
    bleKeyboard.press(keyCode);
  }
  else {
    bleKeyboard.release(keyCode);
  }
}

#define k(v) key(v)
#ifdef KEYBOARD_LEFTSIDE
keymap_t layer0 = {
  {
    k(KEY_TAB),
    k(KEY_CAPS_LOCK),
    k(KEY_LEFT_SHIFT)
  },
  {
    k('q'),
    k('a'),
    k('z'),
    k(KEY_MOD_1)
  },
  {
    k('w'),
    k('s'),
    k('x'),
    k(KEY_LEFT_ALT)
  },
  {
    k('e'),
    k('d'),
    k('c'),
    k(KEY_LEFT_GUI)
  },
  {
    k('r'),
    k('f'),
    k('v'),
    k(KEY_LEFT_CTRL)
  },
  {
    k('t'),
    k('g'),
    k('b'),
    k(' ')
  }
};
keymap_t layer1 = {
  {
    k(KEY_ESC),
    k(),
    k()
  },
  {
    k('1'),
    k(),
    k(),
    k(KEY_MOD_1)
  },
  {
    k('2'),
    k(),
    k(),
    k()
  },
  {
    k('3'),
    k(),
    k(),
    k()
  },
  {
    k('4'),
    k(),
    k(),
    k()
  },
  {
    k('5'),
    k(),
    k(),
    k()
  }
};
keymap_t layer2 = {
  {
    k(),
    k(),
    k()
  },
  {
    k(),
    k(),
    k(),
    k(KEY_MOD_1)
  },
  {
    k(),
    k(),
    k(),
    k()
  },
  {
    k(),
    k(),
    k(),
    k()
  },
  {
    k(),
    k(),
    k(),
    k()
  },
  {
    k(),
    k(),
    k(),
    k()
  }
};
#else
keymap_t layer0 = {
  {
    k(KEY_BACKSPACE),
    k(KEY_RETURN),
    k(KEY_RIGHT_SHIFT)
  },
  {
    k('p'),
    k(';'),
    k('/'),
    k(KEY_MOD_1)
  },
  {
    k('o'),
    k('l'),
    k('.'),
    k(KEY_RIGHT_ALT)
  },
  {
    k('i'),
    k('k'),
    k(','),
    k(KEY_RIGHT_GUI)
  },
  {
    k('u'),
    k('j'),
    k('m'),
    k(KEY_MOD_2)
  },
  {
    k('y'),
    k('h'),
    k('n'),
    k(' ')
  }
};
keymap_t layer1 = {
  {
    k(KEY_DELETE),
    k(),
    k()
  },
  {
    k('0'),
    k(),
    k(),
    k(KEY_MOD_1)
  },
  {
    k('9'),
    k(),
    k(),
    k()
  },
  {
    k('8'),
    k(),
    k(),
    k()
  },
  {
    k('7'),
    k(),
    k(),
    k(KEY_MOD_2)
  },
  {
    k('6'),
    k(),
    k(),
    k()
  }
};
keymap_t layer2 = {
  {
    k(),
    k(),
    k()
  },
  {
    k('='),
    k('\''),
    k(),
    k(KEY_MOD_1)
  },
  {
    k('-'),
    k(KEY_RIGHT_ARROW),
    k(),
    k()
  },
  {
    k(KEY_UP_ARROW),
    k(KEY_DOWN_ARROW),
    k(),
    k()
  },
  {
    k(),
    k(KEY_LEFT_ARROW),
    k(),
    k(KEY_MOD_2)
  },
  {
    k(),
    k(),
    k(),
    k()
  }
};
#endif

void setup() {
  Serial.begin(9600);

  bleKeyboard.begin();

  for (int x = 0; x < keyboard.cols.size(); x++) {
    pinMode(keyboard.cols[x], OUTPUT);
    digitalWrite(keyboard.cols[x], LOW);
  }
  for (int x = 0; x < keyboard.rows.size(); x++) {
    pinMode(keyboard.rows[x], INPUT_PULLDOWN);
  }

  // Set layers
  keyboard.layers = { layer0, layer1, layer2 };
}

void loop() {

  // Wait for keyboard to connect to host
  if (!bleKeyboard.isConnected()) {
    delay(1000);
    return;
  }
  
  // Read matrix of keys
  for (int x = 0; x < keyboard.cols.size(); x++) {

    digitalWrite(keyboard.cols[x], HIGH);

    for (int y = 0; y < keyboard.rows.size(); y++) {

      if (keyboard.modifier < keyboard.layers.size()) {
        if (x < keyboard.layers[keyboard.modifier].size()) {
          if (y < keyboard.layers[keyboard.modifier][x].size()) {

            // Get current key to check
            key* currentKey = &keyboard.layers[keyboard.modifier][x][y];
            bool keyIsPressed = digitalRead(keyboard.rows[y]);

            // Check key status and do debounce check
            unsigned long currentTime = millis();
            if (keyIsPressed != currentKey->isPressed && (currentTime - currentKey->lastStateChange) > KEYBOARD_DEBOUNCE_MS) {

              // Perform actions based on keypress
              currentKey->isPressed = keyIsPressed;
              currentKey->lastStateChange = currentTime;
              onKeyStateChange(currentKey);
            }
          }
        }
      }
    }
    digitalWrite(keyboard.cols[x], LOW);
  }
}