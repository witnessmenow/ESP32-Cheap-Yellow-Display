/*
Midbar
Distributed under the MIT License
© Copyright Maxim Bortnikov 2024
For more information please visit
https://sourceforge.net/projects/midbar-firebase-edition/
https://github.com/Northstrix/Midbar-Firebase-Edition
Required libraries:
https://github.com/Northstrix/AES_in_CBC_mode_for_microcontrollers
https://github.com/ulwanski/sha512
https://github.com/Bodmer/TFT_eSPI
https://github.com/intrbiz/arduino-crypto
https://github.com/techpaul/PS2KeyAdvanced
https://github.com/techpaul/PS2KeyMap
https://github.com/mobizt/Firebase-ESP32
*/
// !!! Before uploading this sketch -
// Switch the partition scheme to the
// "Huge APP (3MB No OTA/1MB SPIFFS)" !!!
#include <FirebaseESP32.h>
#include "aes.h"
#include "Crypto.h"
#include "sha512.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include "midbaricon.h"
#include <PS2KeyAdvanced.h>
#include <PS2KeyMap.h>
#include <Wire.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>

#define WIFI_SSID "accessPpointName"
#define WIFI_PASSWORD "accessPointPassword"
#define API_KEY "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define DATABASE_URL "https://database-name-default-rtdb.firebaseio.com/"

#define MAX_NUM_OF_RECS 999
#define DELAY_FOR_SLOTS 24
const unsigned int iterations = 20451;

#define DATAPIN 16
#define IRQPIN 5
#define TYPE_DELAY 72

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite mvng_bc = TFT_eSprite(&tft);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
PS2KeyAdvanced keyboard;
PS2KeyMap keymap;
RNG random_number;

int m;
int clb_m;
String dec_st;
String dec_tag;
int decract;
byte array_for_CBC_mode[16];
String input_from_the_ps2_keyboard;
int chosen_lock_screen;
int read_keyboard_delay = 60;
int curr_key;
int curr_pos;
int prsd_key;
uint16_t code;
int k;
bool finish_input;
bool act;
bool decrypt_tag;
bool rec_d;
bool decrypt_hash;
byte data_from_keyboard;
byte sdown = 90;
uint16_t colors[5] = { // Purple, Yellow, Green, Shade of Blue N1, Shade of Blue N2
  0xb81c, 0xfde0, 0x87a0, 0x041c, 0x051b
};
const uint16_t current_inact_clr = colors[1];
const uint16_t five_six_five_red_color = 0xf940;
String succs_ver_inscr = "Integrity Verified Successfully!";
String faild_ver_inscr = "Integrity Verification Failed!";
bool display_moving_background = true;
bool display_pattern_while_adding_and_printing_record = true;
String fuid = ""; 
bool isAuthenticated = false;
uint8_t back_aes_key[32]; 
uint32_t aes_mode[3] = {128, 192, 256};
uint8_t aes_key[32];
byte hmackey[32];

int generate_random_number() {
  return random_number.get();
}

void type_on_virtual_keyboard(String data_to_type){
  int lng = data_to_type.length();
  for (int i = 0; i < lng; i++){
    Wire.beginTransmission(4);
    Wire.write(byte(data_to_type.charAt(i)));
    Wire.endTransmission();
    delay(TYPE_DELAY);
  }
}

int derive_key_with_pbkdf2(const char *password, size_t password_len, const unsigned char *salt, size_t salt_len, unsigned int iterations, size_t key_len, unsigned char *output_key) {
    mbedtls_md_context_t ctx; // Declare mbedtls_md_context_t structure
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256; // Specify hash function type
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(md_type);

    if (md_info == NULL) {
        return MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE;
    }

    // Initialize MD context
    mbedtls_md_init(&ctx);

    // Setup MD context
    int ret = mbedtls_md_setup(&ctx, md_info, 1); // 1 for HMAC

    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return ret;
    }

    // Perform PBKDF2 key derivation
    ret = mbedtls_pkcs5_pbkdf2_hmac(&ctx, (const unsigned char *)password, password_len, salt, salt_len, iterations, key_len, output_key);

    // Free MD context
    mbedtls_md_free(&ctx);

    return ret;
}

// AES in CBC Mode(Below)

void back_aes_k() {
  for (int i = 0; i < 32; i++) {
    back_aes_key[i] = aes_key[i];
  }
}

void rest_aes_k() {
  for (int i = 0; i < 32; i++) {
    aes_key[i] = back_aes_key[i];
  }
}

void incr_aes_key() {
  if (aes_key[15] == 255) {
    aes_key[15] = 0;
    if (aes_key[14] == 255) {
      aes_key[14] = 0;
      if (aes_key[13] == 255) {
        aes_key[13] = 0;
        if (aes_key[12] == 255) {
          aes_key[12] = 0;
          if (aes_key[11] == 255) {
            aes_key[11] = 0;
            if (aes_key[10] == 255) {
              aes_key[10] = 0;
              if (aes_key[9] == 255) {
                aes_key[9] = 0;
                if (aes_key[8] == 255) {
                  aes_key[8] = 0;
                  if (aes_key[7] == 255) {
                    aes_key[7] = 0;
                    if (aes_key[6] == 255) {
                      aes_key[6] = 0;
                      if (aes_key[5] == 255) {
                        aes_key[5] = 0;
                        if (aes_key[4] == 255) {
                          aes_key[4] = 0;
                          if (aes_key[3] == 255) {
                            aes_key[3] = 0;
                            if (aes_key[2] == 255) {
                              aes_key[2] = 0;
                              if (aes_key[1] == 255) {
                                aes_key[1] = 0;
                                if (aes_key[0] == 255) {
                                  aes_key[0] = 0;
                                } else {
                                  aes_key[0]++;
                                }
                              } else {
                                aes_key[1]++;
                              }
                            } else {
                              aes_key[2]++;
                            }
                          } else {
                            aes_key[3]++;
                          }
                        } else {
                          aes_key[4]++;
                        }
                      } else {
                        aes_key[5]++;
                      }
                    } else {
                      aes_key[6]++;
                    }
                  } else {
                    aes_key[7]++;
                  }
                } else {
                  aes_key[8]++;
                }
              } else {
                aes_key[9]++;
              }
            } else {
              aes_key[10]++;
            }
          } else {
            aes_key[11]++;
          }
        } else {
          aes_key[12]++;
        }
      } else {
        aes_key[13]++;
      }
    } else {
      aes_key[14]++;
    }
  } else {
    aes_key[15]++;
  }
}

int getNum(char ch) {
  int num = 0;
  if (ch >= '0' && ch <= '9') {
    num = ch - 0x30;
  } else {
    switch (ch) {
    case 'A':
    case 'a':
      num = 10;
      break;
    case 'B':
    case 'b':
      num = 11;
      break;
    case 'C':
    case 'c':
      num = 12;
      break;
    case 'D':
    case 'd':
      num = 13;
      break;
    case 'E':
    case 'e':
      num = 14;
      break;
    case 'F':
    case 'f':
      num = 15;
      break;
    default:
      num = 0;
    }
  }
  return num;
}

char getChar(int num) {
  char ch;
  if (num >= 0 && num <= 9) {
    ch = char(num + 48);
  } else {
    switch (num) {
    case 10:
      ch = 'a';
      break;
    case 11:
      ch = 'b';
      break;
    case 12:
      ch = 'c';
      break;
    case 13:
      ch = 'd';
      break;
    case 14:
      ch = 'e';
      break;
    case 15:
      ch = 'f';
      break;
    }
  }
  return ch;
}

void back_key() {
  back_aes_k();
}

void rest_key() {
  rest_aes_k();
}

void clear_variables() {
  input_from_the_ps2_keyboard = "";
  dec_st = "";
  dec_tag = "";
  decract = 0;
  return;
}

void split_by_sixteen_for_encryption(char plntxt[], int k, int str_len) {
  int res[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };

  for (int i = 0; i < 16; i++) {
    if (i + k > str_len - 1)
      break;
    res[i] = plntxt[i + k];
  }

  for (int i = 0; i < 16; i++) {
    res[i] ^= array_for_CBC_mode[i];
  }
  
  encrypt_with_aes(res);
}

void encrypt_iv_for_aes() {
  int iv[16]; // Initialization vector
  for (int i = 0; i < 16; i++){
    iv[i] = random(256);
  }
  for (int i = 0; i < 16; i++){
    array_for_CBC_mode[i] = iv[i];
  }
  encrypt_with_aes(iv);
}

void encrypt_with_aes(int to_be_encr[]) {
  uint8_t text[16];
  for(int i = 0; i < 16; i++){
    text[i] = to_be_encr[i];
  }
  uint8_t cipher_text[16];
  int i = 0;
  aes_context ctx;
  set_aes_key(&ctx, aes_key, aes_mode[m]);
  aes_encrypt_block(&ctx, cipher_text, text);
    incr_aes_key();
    /*
    for (int i = 0; i < 16; i++) {
      if (cipher_text[i] < 16)
        Serial.print("0");
      Serial.print(cipher_text[i], HEX);
    }
    */
    for (int i = 0; i < 16; i++) {
     if (decract > 0){
        if (i < 16){
          array_for_CBC_mode[i] = int(cipher_text[i]);
        }  
     }
     if (cipher_text[i] < 16)
        dec_st += "0";
      dec_st += String(cipher_text[i], HEX);
    }
    decract++;
}

void split_for_decr(char ct[], int ct_len, int p, int decract1) {
  int br = false;
  int res[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };
  byte prev_res[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };
  for (int i = 0; i < 32; i += 2) {
    if (i + p > ct_len - 1) {
      br = true;
      break;
    }
    if (i == 0) {
      if (ct[i + p] != 0 && ct[i + p + 1] != 0)
        res[i] = 16 * getNum(ct[i + p]) + getNum(ct[i + p + 1]);
      if (ct[i + p] != 0 && ct[i + p + 1] == 0)
        res[i] = 16 * getNum(ct[i + p]);
      if (ct[i + p] == 0 && ct[i + p + 1] != 0)
        res[i] = getNum(ct[i + p + 1]);
      if (ct[i + p] == 0 && ct[i + p + 1] == 0)
        res[i] = 0;
    } else {
      if (ct[i + p] != 0 && ct[i + p + 1] != 0)
        res[i / 2] = 16 * getNum(ct[i + p]) + getNum(ct[i + p + 1]);
      if (ct[i + p] != 0 && ct[i + p + 1] == 0)
        res[i / 2] = 16 * getNum(ct[i + p]);
      if (ct[i + p] == 0 && ct[i + p + 1] != 0)
        res[i / 2] = getNum(ct[i + p + 1]);
      if (ct[i + p] == 0 && ct[i + p + 1] == 0)
        res[i / 2] = 0;
    }
  }

  for (int i = 0; i < 32; i += 2) {
    if (i + p - 32 > ct_len - 1) {
      br = true;
      break;
    }
    if (i == 0) {
      if (ct[i + p - 32] != 0 && ct[i + p - 32 + 1] != 0)
        prev_res[i] = 16 * getNum(ct[i + p - 32]) + getNum(ct[i + p - 32 + 1]);
      if (ct[i + p - 32] != 0 && ct[i + p - 32 + 1] == 0)
        prev_res[i] = 16 * getNum(ct[i + p - 32]);
      if (ct[i + p - 32] == 0 && ct[i + p - 32 + 1] != 0)
        prev_res[i] = getNum(ct[i + p - 32 + 1]);
      if (ct[i + p - 32] == 0 && ct[i + p - 32 + 1] == 0)
        prev_res[i] = 0;
    } else {
      if (ct[i + p - 32] != 0 && ct[i + p - 32 + 1] != 0)
        prev_res[i / 2] = 16 * getNum(ct[i + p - 32]) + getNum(ct[i + p - 32 + 1]);
      if (ct[i + p - 32] != 0 && ct[i + p - 32 + 1] == 0)
        prev_res[i / 2] = 16 * getNum(ct[i + p - 32]);
      if (ct[i + p - 32] == 0 && ct[i + p - 32 + 1] != 0)
        prev_res[i / 2] = getNum(ct[i + p - 32 + 1]);
      if (ct[i + p - 32] == 0 && ct[i + p - 32 + 1] == 0)
        prev_res[i / 2] = 0;
    }
  }
  
  if (br == false) {
    if(decract1 > 16){
      for (int i = 0; i < 16; i++){
        array_for_CBC_mode[i] = prev_res[i];
      }
    }
    uint8_t ret_text[16];
    uint8_t cipher_text[16];
    for(int i = 0; i<16; i++){
      cipher_text[i] = res[i];
    }
    int i = 0;
    aes_context ctx;
    set_aes_key(&ctx, aes_key, aes_mode[m]);
    aes_decrypt_block(&ctx, ret_text, cipher_text);
    incr_aes_key();
    if (decract1 > 2) {
      for (int i = 0; i < 16; i++){
        ret_text[i] ^= array_for_CBC_mode[i];
      }
      if (decrypt_hash == true){
        for (int j = 0; j < 16; j++) {
          delay(1);
          if (ret_text[j] < 16)
            dec_tag += "0";
          dec_tag += String(ret_text[j], HEX);
        }
      }
      if (decrypt_hash == false){
        for (int j = 0; j < 16; j++) {
          if (ret_text[j] > 0){
            dec_st += char(ret_text[j]);
          }
        }
      }
    }

    if (decract1 == -1){
      for (i = 0; i < 16; ++i) {
        array_for_CBC_mode[i] = int(ret_text[i]);
      }
    }
  }
}

void encrypt_string_with_aes_in_cbc(String input) {
  back_key();
  clear_variables();
  encrypt_iv_for_aes();
  int str_len = input.length() + 1;
  char input_arr[str_len];
  input.toCharArray(input_arr, str_len);
  int p = 0;
  while (str_len > p + 1) {
    split_by_sixteen_for_encryption(input_arr, p, str_len);
    p += 16;
  }
  rest_key();
}

void decrypt_string_with_aes_in_cbc(String ct) {
  back_key();
  clear_variables();
  decrypt_hash = false;
  int ct_len = ct.length() + 1;
  char ct_array[ct_len];
  ct.toCharArray(ct_array, ct_len);
  int ext = 0;
  int decract1 = -1;
  while (ct_len > ext) {
    split_for_decr(ct_array, ct_len, 0 + ext, decract1);
    ext += 32;
    decract1 += 10;
  }
  rest_key();
}

void decrypt_tag_with_aes_in_cbc(String ct) {
  back_key();
  clear_variables();
  decrypt_hash = true;
  int ct_len = ct.length() + 1;
  char ct_array[ct_len];
  ct.toCharArray(ct_array, ct_len);
  int ext = 0;
  int decract1 = -1;
  while (ct_len > ext) {
    split_for_decr(ct_array, ct_len, 0 + ext, decract1);
    ext += 32;
    decract1 += 10;
  }
  rest_key();
}

void encrypt_tag_with_aes_in_cbc(String input) {
  back_key();
  clear_variables();
  encrypt_iv_for_aes();
  SHA256HMAC hmac(hmackey, sizeof(hmackey));
  int str_len = input.length() + 1;
  char input_arr[str_len];
  input.toCharArray(input_arr, str_len);
  hmac.doUpdate(input_arr);
  byte authCode[SHA256HMAC_SIZE];
  hmac.doFinal(authCode);
  int p = 0;
  char hmacchar[32];
  for (int i = 0; i < 32; i++) {
    hmacchar[i] = char(authCode[i]);
  }

  for (int i = 0; i < 2; i++) {
    split_by_sixteen_for_encryption(hmacchar, p, 100);
    p += 16;
  }
  rest_key();
}

bool verify_integrity() {
  int str_lentg = dec_st.length() + 1;
  char char_arraytg[str_lentg];
  dec_st.toCharArray(char_arraytg, str_lentg);
  SHA256HMAC hmac(hmackey, sizeof(hmackey));
  hmac.doUpdate(char_arraytg);
  byte authCode[SHA256HMAC_SIZE];
  hmac.doFinal(authCode);
  String res_hash;

  for (byte i = 0; i < SHA256HMAC_SIZE; i++) {
    if (authCode[i] < 0x10) {
      res_hash += 0;
    } {
      res_hash += String(authCode[i], HEX);
    }
  }
  /*
  Serial.println(dec_st);
  Serial.println(dec_tag);
  Serial.println(res_hash);
  */
  return dec_tag.equals(res_hash);
}

// AES in CBC Mode (Above)

void press_any_key_to_continue() {
  rec_d = false;
  delay(2);
  while (rec_d == false) {
    delay(4);
    get_key_from_ps_keyb();
  }
  delay(12);
  data_from_keyboard = 0;
}

void get_key_from_ps_keyb(){
   code = keyboard.available();
   if (code > 0) {
     code = keyboard.read();
     //Serial.print("Value ");
     //Serial.print(code, HEX);
     if (code == 277) { // Leftwards Arrow
       data_from_keyboard = 129;
       rec_d = true;
     }
     if (code == 278) { // Rightwards Arrow
       data_from_keyboard = 130;
       rec_d = true;
     }
     if (code == 279) { // Upwards Arrow
       data_from_keyboard = 131;
       rec_d = true;
     }
     if (code == 280) { // Downwards Arrow
       data_from_keyboard = 132;
       rec_d = true;
     }
     code = keymap.remapKey(code);
     if (code > 0) {
       if ((code & 0xFF)) {
         if ((code & 0xFF) == 27) { // Esc
           data_from_keyboard = 27;
           rec_d = true;
         } else if ((code & 0xFF) == 13) { // Enter
            data_from_keyboard = 13;
            rec_d = true;
         } else if ((code & 0xFF) == 8) { // Backspace
           data_from_keyboard = 8;
           rec_d = true;
         } else {
           data_from_keyboard = code & 0xFF;
           rec_d = true;
         }
       }
   }
  }
}

byte get_PS2_keyboard_input() {
  byte data_from_cntrl_and_keyb;
  while (rec_d == false) {
    delay(4);
    get_key_from_ps_keyb();
  }
  if (rec_d == true) {
    data_from_cntrl_and_keyb = data_from_keyboard;
    rec_d = false;
  }
  return data_from_cntrl_and_keyb;
}

byte get_PS2_keyboard_input_mvn_bcg() {
  byte data_from_cntrl_and_keyb;
  while (rec_d == false) {
    mvn_bcg();
    delayMicroseconds(400);
    get_key_from_ps_keyb();
  }
  if (rec_d == true) {
    data_from_cntrl_and_keyb = data_from_keyboard;
    rec_d = false;
  }
  return data_from_cntrl_and_keyb;
}

void set_stuff_for_input(String blue_inscr) {
  act = true;
  curr_key = 65;
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0xffff);
  tft.setCursor(2, 0);
  tft.print("Char'");
  tft.setCursor(74, 0);
  tft.print("'");
  disp();
  tft.setCursor(0, 24);
  tft.setTextSize(2);
  tft.setTextColor(current_inact_clr);
  tft.print(blue_inscr);
  tft.fillRect(312, 0, 8, 240, current_inact_clr);
  tft.setTextColor(0x07e0);
  tft.setCursor(216, 0);
  tft.print("ASCII:");

}

void check_bounds_and_change_char() {
  if (curr_key < 32)
    curr_key = 126;

  if (curr_key > 126)
    curr_key = 32;
  curr_key = input_from_the_ps2_keyboard.charAt(input_from_the_ps2_keyboard.length() - 1);
}

void disp() {
  //gfx->fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0xffff);
  tft.fillRect(62, 0, 10, 16, 0x0000);
  tft.setCursor(62, 0);
  tft.print(char(curr_key));
  tft.fillRect(288, 0, 22, 14, 0x0000);
  tft.setCursor(288, 0);
  String hexstr;
  if (curr_key < 16)
    hexstr += 0;
  hexstr += String(curr_key, HEX);
  hexstr.toUpperCase();
  tft.setTextColor(0x07e0);
  tft.print(hexstr);
  tft.setTextColor(0xffff);
  tft.setTextSize(2);
  tft.setCursor(0, 48);
  tft.print(input_from_the_ps2_keyboard);
}

void disp_stars() {
  //gfx->fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0xffff);
  tft.fillRect(62, 0, 10, 16, 0x0000);
  tft.setCursor(62, 0);
  tft.print(char(curr_key));
  tft.fillRect(288, 0, 22, 14, 0x0000);
  tft.setCursor(288, 0);
  String hexstr;
  if (curr_key < 16)
    hexstr += 0;
  hexstr += String(curr_key, HEX);
  hexstr.toUpperCase();
  tft.setTextColor(0x07e0);
  tft.print(hexstr);
  int plnt = input_from_the_ps2_keyboard.length();
  String stars = "";
  for (int i = 0; i < plnt; i++) {
    stars += "*";
  }
  tft.setTextColor(0xffff);
  tft.setTextSize(2);
  tft.setCursor(0, 48);
  tft.print(stars);
}

void get_to_ps2_keyboard_input_tab() {
  finish_input = false;
  rec_d = false;
  byte inp_frm_cntr_ps2_kbrd = 0;
  while (finish_input == false) {
    inp_frm_cntr_ps2_kbrd = get_PS2_keyboard_input();
    if (inp_frm_cntr_ps2_kbrd > 0) {
      if (inp_frm_cntr_ps2_kbrd > 31 && inp_frm_cntr_ps2_kbrd < 127) {
        curr_key = inp_frm_cntr_ps2_kbrd;
        input_from_the_ps2_keyboard += char(curr_key);
        //Serial.println(input_from_the_ps2_keyboard);
        disp();
      }

      if (inp_frm_cntr_ps2_kbrd == 27) {
        act = false;
        finish_input = true;
      }

      if (inp_frm_cntr_ps2_kbrd == 13) {
        finish_input = true;
      }

      if (inp_frm_cntr_ps2_kbrd == 130) {
        curr_key++;
        disp();
        if (curr_key < 32)
          curr_key = 126;

        if (curr_key > 126)
          curr_key = 32;
      }

      if (inp_frm_cntr_ps2_kbrd == 129) {
        curr_key--;
        disp();
        if (curr_key < 32)
          curr_key = 126;

        if (curr_key > 126)
          curr_key = 32;
      }

      if (inp_frm_cntr_ps2_kbrd == 131 || inp_frm_cntr_ps2_kbrd == 133) {
        input_from_the_ps2_keyboard += char(curr_key);
        //Serial.println(input_from_the_ps2_keyboard);
        disp();
      }

      if (inp_frm_cntr_ps2_kbrd == 132 || inp_frm_cntr_ps2_kbrd == 8) {
        if (input_from_the_ps2_keyboard.length() > 0)
          input_from_the_ps2_keyboard.remove(input_from_the_ps2_keyboard.length() - 1, 1);
        //Serial.println(input_from_the_ps2_keyboard);
        tft.fillRect(0, 48, 312, 192, 0x0000);
        //Serial.println(input_from_the_ps2_keyboard);
        disp();

      }
      //Serial.println(inp_frm_cntr_ps2_kbrd);
      inp_frm_cntr_ps2_kbrd = 0;
    }
    delayMicroseconds(400);
  }
}

void get_to_starred_ps2_keyboard_input_tab() {
  finish_input = false;
  rec_d = false;
  byte inp_frm_cntr_ps2_kbrd = 0;
  while (finish_input == false) {
    inp_frm_cntr_ps2_kbrd = get_PS2_keyboard_input();
    if (inp_frm_cntr_ps2_kbrd > 0) {
      if (inp_frm_cntr_ps2_kbrd > 31 && inp_frm_cntr_ps2_kbrd < 127) {
        curr_key = inp_frm_cntr_ps2_kbrd;
        input_from_the_ps2_keyboard += char(curr_key);
        //Serial.println(input_from_the_ps2_keyboard);
        disp_stars();
      }

      if (inp_frm_cntr_ps2_kbrd == 27) {
        act = false;
        finish_input = true;
      }

      if (inp_frm_cntr_ps2_kbrd == 13) {
        finish_input = true;
      }

      if (inp_frm_cntr_ps2_kbrd == 130) {
        curr_key++;
        disp_stars();
        if (curr_key < 32)
          curr_key = 126;

        if (curr_key > 126)
          curr_key = 32;
      }

      if (inp_frm_cntr_ps2_kbrd == 129) {
        curr_key--;
        disp_stars();
        if (curr_key < 32)
          curr_key = 126;

        if (curr_key > 126)
          curr_key = 32;
      }

      if (inp_frm_cntr_ps2_kbrd == 131 || inp_frm_cntr_ps2_kbrd == 133) {
        input_from_the_ps2_keyboard += char(curr_key);
        //Serial.println(input_from_the_ps2_keyboard);
        disp_stars();
      }

      if (inp_frm_cntr_ps2_kbrd == 132 || inp_frm_cntr_ps2_kbrd == 8) {
        if (input_from_the_ps2_keyboard.length() > 0)
          input_from_the_ps2_keyboard.remove(input_from_the_ps2_keyboard.length() - 1, 1);
        //Serial.println(input_from_the_ps2_keyboard);
        tft.fillRect(0, 48, 312, 192, 0x0000);
        //Serial.println(input_from_the_ps2_keyboard);
        disp_stars();

      }
      //Serial.println(inp_frm_cntr_ps2_kbrd);
      inp_frm_cntr_ps2_kbrd = 0;
    }
    delayMicroseconds(400);
  }
}

void disp_centered_text(String text, int h) {
  if (text.length() < 27)
    tft.drawCentreString(text, 160, h, 1);
  else{
    tft.setCursor(0, h);
    tft.println(text);
  }
}

void disp_centered_text_b_w(String text, int h) {
  tft.setTextColor(0x0882);
  tft.drawCentreString(text, 160, h - 1, 1);
  tft.drawCentreString(text, 160, h + 1, 1);
  tft.drawCentreString(text, 159, h, 1);
  tft.drawCentreString(text, 161, h, 1);
  tft.setTextColor(0xf7de);
  tft.drawCentreString(text, 160, h, 1);
}

void disp_text_b_w(String text, int h) {
  tft.setTextColor(0x0882);
  tft.setCursor(5, h - 1);
  tft.print(text);
  tft.setCursor(5, h + 1);
  tft.print(text);
  tft.setCursor(4, h);
  tft.print(text);
  tft.setCursor(6, h);
  tft.print(text);
  tft.setTextColor(0xf7de);
  tft.setCursor(5, h);
  tft.print(text);
}

String read_file(String filename){
  if(Firebase.getString(fbdo, filename.c_str())){
    return (fbdo.to<String>());
  }
  else
    return "-1";
}

void write_to_file_with_overwrite(String filename, String content){
  if (!Firebase.set(fbdo, filename.c_str(), content.c_str())){
    tft.fillScreen(0x0000);
    tft.setTextColor(0xf800);
    tft.setCursor(0, 0);
    tft.print("Failed To Write Data!");
    delay(2000);
    tft.fillScreen(0x0000);    
  }
}

void delete_file(String filename){
  if(!Firebase.deleteNode(fbdo, filename.c_str())){
    tft.fillScreen(0x0000);
    tft.setTextColor(0xf800);
    tft.setCursor(0, 0);
    tft.print("Failed To Delete File!");
    delay(2000);
    tft.fillScreen(0x0000);
  }
}

void typing_inscription(){
  display_background_while_adding_record();
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  disp_text_b_w("Typing...", 5);
  disp_text_b_w("Please wait for a while.", 17);
}

// Functions for Logins (Below)

void select_login(byte what_to_do_with_it) {
  // 0 - Add login
  // 1 - Edit login
  // 2 - Delete login
  // 3 - View login
  delay(DELAY_FOR_SLOTS);
  curr_key = 1;

  header_for_select_login(what_to_do_with_it);
  display_title_from_login_without_integrity_verification();
  bool continue_to_next = false;
  while (continue_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 130)
        curr_key++;

      if (input_data == 129)
        curr_key--;

      if (curr_key < 1)
        curr_key = MAX_NUM_OF_RECS;

      if (curr_key > MAX_NUM_OF_RECS)
        curr_key = 1;

      if (input_data == 13 || input_data == 133) { // Enter
        int chsn_slot = curr_key;
        if (what_to_do_with_it == 0 && continue_to_next == false) {
          continue_to_next = true;
          add_login_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 1 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          edit_login_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 2 && continue_to_next == false) {
          continue_to_next = true;
          delete_login(chsn_slot);
        }
        if (what_to_do_with_it == 3 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          view_login(chsn_slot);
        }
        if (what_to_do_with_it == 4 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          type_login(chsn_slot);
        }
        continue_to_next = true;
        break;
      }

      if (input_data == 8 || input_data == 27) {
        call_main_menu();
        continue_to_next = true;
        break;
      }
      delay(DELAY_FOR_SLOTS);
      header_for_select_login(what_to_do_with_it);
      display_title_from_login_without_integrity_verification();
    }
    delayMicroseconds(500);
  }
  return;
}

void header_for_select_login(byte what_to_do_with_it) {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  if (what_to_do_with_it == 0) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add Login to Slot " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Edit Login " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 2) {
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("Delete Login " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation_for_del();
  }
  if (what_to_do_with_it == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("View Login " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Type Login " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
}

void display_title_from_login_without_integrity_verification() {
  tft.setTextSize(2);
  String encrypted_title = read_file("/L" + String(curr_key) + "_ttl");
  if (encrypted_title == "-1") {
    tft.setTextColor(0x07e0);
    disp_centered_text("Empty", 35);
  } else {
    clear_variables();
    decrypt_tag = false;
    decrypt_string_with_aes_in_cbc(encrypted_title);
    tft.setTextColor(0xffff);
    disp_centered_text(dec_st, 35);
  }
}

void add_login_from_nint_controller(int chsn_slot) {
  enter_title_for_login(chsn_slot);
  clear_variables();
  call_main_menu();
  return;
}

void enter_title_for_login(int chsn_slot) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Title");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_username_for_login(chsn_slot, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_username_for_login(int chsn_slot, String entered_title) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Username");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_password_for_login(chsn_slot, entered_title, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_password_for_login(int chsn_slot, String entered_title, String entered_username) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Password");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_website_for_login(chsn_slot, entered_title, entered_username, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_website_for_login(int chsn_slot, String entered_title, String entered_username, String entered_password) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Website");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    write_login_to_flash(chsn_slot, entered_title, entered_username, entered_password, input_from_the_ps2_keyboard);
  }
  return;
}

void write_login_to_flash(int chsn_slot, String entered_title, String entered_username, String entered_password, String entered_website) {
  /*
  Serial.println();
  Serial.println(chsn_slot);
  Serial.println(entered_title);
  Serial.println(entered_username);
  Serial.println(entered_password);
  Serial.println(entered_website);
  */
  display_background_while_adding_record();
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  disp_text_b_w("Adding Login To The Slot N" + String(chsn_slot) + "...", 5);
  disp_text_b_w("Please wait for a while.", 17);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_title);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_ttl", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_username);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_usn", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_password);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_psw", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_website);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_wbs", dec_st);
  clear_variables();
  encrypt_tag_with_aes_in_cbc(entered_title + entered_username + entered_password + entered_website);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void update_login_and_tag(int chsn_slot, String new_password) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Editing Login In The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");

  clear_variables();
  encrypt_string_with_aes_in_cbc(new_password);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_psw", dec_st);

  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_ttl"));
  String decrypted_title = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_usn"));
  String decrypted_username = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_wbs"));
  String decrypted_website = dec_st;

  clear_variables();
  encrypt_tag_with_aes_in_cbc(decrypted_title + decrypted_username + new_password + decrypted_website);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/L" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void edit_login_from_nint_controller(int chsn_slot) {
  if (read_file("/L" + String(chsn_slot) + "_psw") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_psw"));
    String old_password = dec_st;
    act = true;
    clear_variables();
    set_stuff_for_input("Edit Password");
    input_from_the_ps2_keyboard = old_password;
    disp();
    get_to_ps2_keyboard_input_tab();
    if (act == true) {
      update_login_and_tag(chsn_slot, input_from_the_ps2_keyboard);
    }
  }
  return;
}

void delete_login(int chsn_slot) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Deleting Login From The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  delete_file("/L" + String(chsn_slot) + "_tag");
  delete_file("/L" + String(chsn_slot) + "_ttl");
  delete_file("/L" + String(chsn_slot) + "_usn");
  delete_file("/L" + String(chsn_slot) + "_psw");
  delete_file("/L" + String(chsn_slot) + "_wbs");
  clear_variables();
  call_main_menu();
  return;
}

void view_login(int chsn_slot) {
  if (read_file("/L" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_usn"));
    String decrypted_username = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_psw"));
    String decrypted_password = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_wbs"));
    String decrypted_website = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_username + decrypted_password + decrypted_website;
    bool login_integrity = verify_integrity();

    if (login_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Username:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_username);
      tft.setTextColor(current_inact_clr);
      tft.print("Password:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_password);
      tft.setTextColor(current_inact_clr);
      tft.print("Website:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_website);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, current_inact_clr);
      disp_centered_text(succs_ver_inscr, 232);
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Username:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_username);
      tft.setTextColor(current_inact_clr);
      tft.print("Password:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_password);
      tft.setTextColor(current_inact_clr);
      tft.print("Website:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_website);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, five_six_five_red_color);
      disp_centered_text(faild_ver_inscr, 232);
    }
    act = false;
    up_or_encdr_bttn_to_print();
    if (act == true) {
      Serial.println();
      Serial.print("Title:\"");
      Serial.print(decrypted_title);
      Serial.println("\"");
      Serial.print("Username:\"");
      Serial.print(decrypted_username);
      Serial.println("\"");
      Serial.print("Password:\"");
      Serial.print(decrypted_password);
      Serial.println("\"");
      Serial.print("Website:\"");
      Serial.print(decrypted_website);
      Serial.println("\"");
      if (login_integrity == true) {
        Serial.println("Integrity Verified Successfully!\n");
      } else {
        Serial.println("Integrity Verification Failed!!!\n");
      }
    }
  }
}

void type_login(int chsn_slot) {
  if (read_file("/L" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextSize(1);
    tft.setTextColor(0xffff);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_key_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_usn"));
    String decrypted_username = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_psw"));
    String decrypted_password = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_wbs"));
    String decrypted_website = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/L" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_username + decrypted_password + decrypted_website;
    bool login_integrity = verify_integrity();

    if (login_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Website\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_website);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Username\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_username);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Password\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_password);
      }
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(five_six_five_red_color);
      disp_centered_text("Integrity", 65);
      disp_centered_text("Verification", 85);
      disp_centered_text("Failed!!!", 105);
      tft.setTextSize(1);
      tft.setTextColor(0xffff);
      disp_centered_text("Press any key to return to the main menu", 232);
      press_any_key_to_continue();
    }
  }
}

// Functions for Logins (Above)

// Functions for Credit Cards (Below)

void select_credit_card(byte what_to_do_with_it) {
  // 0 - Add credit_card
  // 1 - Edit credit_card
  // 2 - Delete credit_card
  // 3 - View credit_card
  delay(DELAY_FOR_SLOTS);
  curr_key = 1;

  header_for_select_credit_card(what_to_do_with_it);
  display_title_from_credit_card_without_integrity_verification();
  bool continue_to_next = false;
  while (continue_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 130)
        curr_key++;

      if (input_data == 129)
        curr_key--;

      if (curr_key < 1)
        curr_key = MAX_NUM_OF_RECS;

      if (curr_key > MAX_NUM_OF_RECS)
        curr_key = 1;

      if (input_data == 13 || input_data == 133) { // Enter
        int chsn_slot = curr_key;
        if (what_to_do_with_it == 0 && continue_to_next == false) {
          continue_to_next = true;
          add_credit_card_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 1 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          edit_credit_card_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 2 && continue_to_next == false) {
          continue_to_next = true;
          delete_credit_card(chsn_slot);
        }
        if (what_to_do_with_it == 3 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          view_credit_card(chsn_slot);
        }
        if (what_to_do_with_it == 4 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          type_credit_card(chsn_slot);
        }
        continue_to_next = true;
        break;
      }

      if (input_data == 8 || input_data == 27) {
        call_main_menu();
        continue_to_next = true;
        break;
      }
      delay(DELAY_FOR_SLOTS);
      header_for_select_credit_card(what_to_do_with_it);
      display_title_from_credit_card_without_integrity_verification();
    }
    delayMicroseconds(500);
  }
  return;
}

void header_for_select_credit_card(byte what_to_do_with_it) {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  if (what_to_do_with_it == 0) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add Card to Slot " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Edit Credit Card " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 2) {
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("Delete Credit Card " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation_for_del();
  }
  if (what_to_do_with_it == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("View Credit Card " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Type Credit Card " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
}

void display_title_from_credit_card_without_integrity_verification() {
  tft.setTextSize(2);
  String encrypted_title = read_file("/C" + String(curr_key) + "_ttl");
  if (encrypted_title == "-1") {
    tft.setTextColor(0x07e0);
    disp_centered_text("Empty", 35);
  } else {
    clear_variables();
    decrypt_tag = false;
    decrypt_string_with_aes_in_cbc(encrypted_title);
    tft.setTextColor(0xffff);
    disp_centered_text(dec_st, 35);
  }
}

void add_credit_card_from_nint_controller(int chsn_slot) {
  enter_title_for_credit_card(chsn_slot);
  clear_variables();
  call_main_menu();
  return;
}

void enter_title_for_credit_card(int chsn_slot) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Title");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_cardholder_for_credit_card(chsn_slot, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_cardholder_for_credit_card(int chsn_slot, String entered_title) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Cardholder Name");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_card_number_for_credit_card(chsn_slot, entered_title, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_card_number_for_credit_card(int chsn_slot, String entered_title, String entered_cardholder) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Card Number");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_expiry_for_credit_card(chsn_slot, entered_title, entered_cardholder, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_expiry_for_credit_card(int chsn_slot, String entered_title, String entered_cardholder, String entered_card_number) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Expiration Date");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_cvn_for_credit_card(chsn_slot, entered_title, entered_cardholder, entered_card_number, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_cvn_for_credit_card(int chsn_slot, String entered_title, String entered_cardholder, String entered_card_number, String entered_expiry) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter CVN");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_pin_for_credit_card(chsn_slot, entered_title, entered_cardholder, entered_card_number, entered_expiry, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_pin_for_credit_card(int chsn_slot, String entered_title, String entered_cardholder, String entered_card_number, String entered_expiry, String entered_cvn) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter PIN");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_zip_code_for_credit_card(chsn_slot, entered_title, entered_cardholder, entered_card_number, entered_expiry, entered_cvn, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_zip_code_for_credit_card(int chsn_slot, String entered_title, String entered_cardholder, String entered_card_number, String entered_expiry, String entered_cvn, String entered_pin) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter ZIP Code");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    write_credit_card_to_flash(chsn_slot, entered_title, entered_cardholder, entered_card_number, entered_expiry, entered_cvn, entered_pin, input_from_the_ps2_keyboard);
  }
  return;
}

void write_credit_card_to_flash(int chsn_slot, String entered_title, String entered_cardholder, String entered_card_number, String entered_expiry, String entered_cvn, String entered_pin, String entered_zip_code) {
  /*
  Serial.println();
  Serial.println(chsn_slot);
  Serial.println(entered_title);
  Serial.println(entered_cardholder);
  Serial.println(entered_card_number);
  Serial.println(entered_expiry);
  */
  display_background_while_adding_record();
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  disp_text_b_w("Adding Credit Card To The Slot N" + String(chsn_slot) + "...", 5);
  disp_text_b_w("Please wait for a while.", 17);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_title);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_ttl", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_cardholder);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_hld", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_card_number);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_nmr", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_expiry);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_exp", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_cvn);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_cvn", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_pin);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_pin", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_zip_code);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_zip", dec_st);
  clear_variables();
  encrypt_tag_with_aes_in_cbc(entered_title + entered_cardholder + entered_card_number + entered_expiry + entered_cvn + entered_pin + entered_zip_code);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void update_credit_card_and_tag(int chsn_slot, String new_pin) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Editing Credit Card In The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  clear_variables();
  encrypt_string_with_aes_in_cbc(new_pin);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_pin", dec_st);
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_ttl"));
  String decrypted_title = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_hld"));
  String decrypted_cardholder = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_nmr"));
  String decrypted_card_number = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_exp"));
  String decrypted_expiry = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_cvn"));
  String decrypted_cvn = dec_st;
  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_zip"));
  String decrypted_zip_code = dec_st;
  clear_variables();
  encrypt_tag_with_aes_in_cbc(decrypted_title + decrypted_cardholder + decrypted_card_number + decrypted_expiry + decrypted_cvn + new_pin + decrypted_zip_code);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/C" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void edit_credit_card_from_nint_controller(int chsn_slot) {
  if (read_file("/C" + String(chsn_slot) + "_pin") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_pin"));
    String old_pin = dec_st;
    act = true;
    clear_variables();
    set_stuff_for_input("Edit PIN");
    input_from_the_ps2_keyboard = old_pin;
    disp();
    get_to_ps2_keyboard_input_tab();
    if (act == true) {
      update_credit_card_and_tag(chsn_slot, input_from_the_ps2_keyboard);
    }
  }
  return;
}

void delete_credit_card(int chsn_slot) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Deleting Credit Card From The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  delete_file("/C" + String(chsn_slot) + "_tag");
  delete_file("/C" + String(chsn_slot) + "_ttl");
  delete_file("/C" + String(chsn_slot) + "_hld");
  delete_file("/C" + String(chsn_slot) + "_nmr");
  delete_file("/C" + String(chsn_slot) + "_exp");
  delete_file("/C" + String(chsn_slot) + "_cvn");
  delete_file("/C" + String(chsn_slot) + "_pin");
  delete_file("/C" + String(chsn_slot) + "_zip");
  clear_variables();
  call_main_menu();
  return;
}

void view_credit_card(int chsn_slot) {
  if (read_file("/C" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_hld"));
    String decrypted_cardholder = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_nmr"));
    String decrypted_card_number = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_exp"));
    String decrypted_expiry = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_cvn"));
    String decrypted_cvn = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_pin"));
    String decrypted_pin = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_zip"));
    String decrypted_zip_code = dec_st;
    decrypt_tag_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_cardholder + decrypted_card_number + decrypted_expiry + decrypted_cvn + decrypted_pin + decrypted_zip_code;
    bool credit_card_integrity = verify_integrity();

    if (credit_card_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Cardholder Name:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_cardholder);
      tft.setTextColor(current_inact_clr);
      tft.print("Card Number:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_card_number);
      tft.setTextColor(current_inact_clr);
      tft.print("Expiration Date:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_expiry);
      tft.setTextColor(current_inact_clr);
      tft.print("CVN:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_cvn);
      tft.setTextColor(current_inact_clr);
      tft.print("PIN:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_pin);
      tft.setTextColor(current_inact_clr);
      tft.print("ZIP Code:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_zip_code);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, current_inact_clr);
      disp_centered_text(succs_ver_inscr, 232);
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Cardholder Name:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_cardholder);
      tft.setTextColor(current_inact_clr);
      tft.print("Card Number:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_card_number);
      tft.setTextColor(current_inact_clr);
      tft.print("Expiration Date:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_expiry);
      tft.setTextColor(current_inact_clr);
      tft.print("CVN:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_cvn);
      tft.setTextColor(current_inact_clr);
      tft.print("PIN:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_pin);
      tft.setTextColor(current_inact_clr);
      tft.print("ZIP Code:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_zip_code);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, five_six_five_red_color);
      disp_centered_text(faild_ver_inscr, 232);
    }
    act = false;
    up_or_encdr_bttn_to_print();
    if (act == true) {
      Serial.println();
      Serial.print("Title:\"");
      Serial.print(decrypted_title);
      Serial.println("\"");
      Serial.print("Cardholder Name:\"");
      Serial.print(decrypted_cardholder);
      Serial.println("\"");
      Serial.print("Card Number:\"");
      Serial.print(decrypted_card_number);
      Serial.println("\"");
      Serial.print("Expiration Date:\"");
      Serial.print(decrypted_expiry);
      Serial.println("\"");
      Serial.print("CVN:\"");
      Serial.print(decrypted_cvn);
      Serial.println("\"");
      Serial.print("PIN:\"");
      Serial.print(decrypted_pin);
      Serial.println("\"");
      Serial.print("ZIP Code:\"");
      Serial.print(decrypted_zip_code);
      Serial.println("\"");
      if (credit_card_integrity == true) {
        Serial.println("Integrity Verified Successfully!\n");
      } else {
        Serial.println("Integrity Verification Failed!!!\n");
      }
    }
  }
}

void type_credit_card(int chsn_slot) {
  if (read_file("/C" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextSize(1);
    tft.setTextColor(0xffff);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_key_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_hld"));
    String decrypted_cardholder = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_nmr"));
    String decrypted_card_number = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_exp"));
    String decrypted_expiry = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_cvn"));
    String decrypted_cvn = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_pin"));
    String decrypted_pin = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_zip"));
    String decrypted_zip_code = dec_st;
    decrypt_tag_with_aes_in_cbc(read_file("/C" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_cardholder + decrypted_card_number + decrypted_expiry + decrypted_cvn + decrypted_pin + decrypted_zip_code;
    bool credit_card_integrity = verify_integrity();

    if (credit_card_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Cardholder Name\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_cardholder);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Card Number\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_card_number);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Expiration Date\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_expiry);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"CVN\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_cvn);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"PIN\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_pin);
      }
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"ZIP Code\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_zip_code);
      }
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(five_six_five_red_color);
      disp_centered_text("Integrity", 65);
      disp_centered_text("Verification", 85);
      disp_centered_text("Failed!!!", 105);
      tft.setTextSize(1);
      tft.setTextColor(0xffff);
      disp_centered_text("Press any key to return to the main menu", 232);
      press_any_key_to_continue();
    }
  }
}

// Functions for Credit Cards (Above)

// Functions for Notes (Below)

void select_note(byte what_to_do_with_it) {
  // 0 - Add note
  // 1 - Edit note
  // 2 - Delete note
  // 3 - View note
  delay(DELAY_FOR_SLOTS);
  curr_key = 1;

  header_for_select_note(what_to_do_with_it);
  display_title_from_note_without_integrity_verification();
  bool continue_to_next = false;
  while (continue_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 130)
        curr_key++;

      if (input_data == 129)
        curr_key--;

      if (curr_key < 1)
        curr_key = MAX_NUM_OF_RECS;

      if (curr_key > MAX_NUM_OF_RECS)
        curr_key = 1;

      if (input_data == 13 || input_data == 133) { // Enter
        int chsn_slot = curr_key;
        if (what_to_do_with_it == 0 && continue_to_next == false) {
          continue_to_next = true;
          add_note_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 1 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          edit_note_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 2 && continue_to_next == false) {
          continue_to_next = true;
          delete_note(chsn_slot);
        }
        if (what_to_do_with_it == 3 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          view_note(chsn_slot);
        }
        if (what_to_do_with_it == 4 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          type_note(chsn_slot);
        }
        continue_to_next = true;
        break;
      }

      if (input_data == 8 || input_data == 27) {
        call_main_menu();
        continue_to_next = true;
        break;
      }
      delay(DELAY_FOR_SLOTS);
      header_for_select_note(what_to_do_with_it);
      display_title_from_note_without_integrity_verification();
    }
    delayMicroseconds(500);
  }
  return;
}

void header_for_select_note(byte what_to_do_with_it) {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  if (what_to_do_with_it == 0) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add Note to Slot " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Edit Note " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 2) {
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("Delete Note " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation_for_del();
  }
  if (what_to_do_with_it == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("View Note " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Type Note " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
}

void display_title_from_note_without_integrity_verification() {
  tft.setTextSize(2);
  String encrypted_title = read_file("/N" + String(curr_key) + "_ttl");
  if (encrypted_title == "-1") {
    tft.setTextColor(0x07e0);
    disp_centered_text("Empty", 35);
  } else {
    clear_variables();
    decrypt_tag = false;
    decrypt_string_with_aes_in_cbc(encrypted_title);
    tft.setTextColor(0xffff);
    disp_centered_text(dec_st, 35);
  }
}

void add_note_from_nint_controller(int chsn_slot) {
  enter_title_for_note(chsn_slot);
  clear_variables();
  call_main_menu();
  return;
}

void enter_title_for_note(int chsn_slot) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Title");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_content_for_note(chsn_slot, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_content_for_note(int chsn_slot, String entered_title) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Content");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    write_note_to_flash(chsn_slot, entered_title, input_from_the_ps2_keyboard);
  }
  return;
}

void write_note_to_flash(int chsn_slot, String entered_title, String entered_content) {
  display_background_while_adding_record();
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  disp_text_b_w("Adding Note To The Slot N" + String(chsn_slot) + "...", 5);
  disp_text_b_w("Please wait for a while.", 17);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_title);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/N" + String(chsn_slot) + "_ttl", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_content);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/N" + String(chsn_slot) + "_cnt", dec_st);
  clear_variables();
  encrypt_tag_with_aes_in_cbc(entered_title + entered_content);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/N" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void update_note_and_tag(int chsn_slot, String new_content) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Editing Note In The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");

  clear_variables();
  encrypt_string_with_aes_in_cbc(new_content);
  write_to_file_with_overwrite("/N" + String(chsn_slot) + "_cnt", dec_st);

  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_ttl"));
  String decrypted_title = dec_st;

  clear_variables();
  encrypt_tag_with_aes_in_cbc(decrypted_title + new_content);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/N" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void edit_note_from_nint_controller(int chsn_slot) {
  if (read_file("/N" + String(chsn_slot) + "_cnt") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_cnt"));
    String old_password = dec_st;
    act = true;
    clear_variables();
    set_stuff_for_input("Edit Note");
    input_from_the_ps2_keyboard = old_password;
    disp();
    get_to_ps2_keyboard_input_tab();
    if (act == true) {
      update_note_and_tag(chsn_slot, input_from_the_ps2_keyboard);
    }
  }
  return;
}

void delete_note(int chsn_slot) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Deleting Note From The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  delete_file("/N" + String(chsn_slot) + "_tag");
  delete_file("/N" + String(chsn_slot) + "_ttl");
  delete_file("/N" + String(chsn_slot) + "_cnt");
  clear_variables();
  call_main_menu();
  return;
}

void view_note(int chsn_slot) {
  if (read_file("/N" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_cnt"));
    String decrypted_content = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_content;
    bool login_integrity = verify_integrity();

    if (login_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Content:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_content);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, current_inact_clr);
      disp_centered_text(succs_ver_inscr, 232);
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Content:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_content);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, five_six_five_red_color);
      disp_centered_text(faild_ver_inscr, 232);
    }
    act = false;
    up_or_encdr_bttn_to_print();
    if (act == true) {
      Serial.println();
      Serial.print("Title:\"");
      Serial.print(decrypted_title);
      Serial.println("\"");
      Serial.print("Content:\"");
      Serial.print(decrypted_content);
      Serial.println("\"");
      if (login_integrity == true) {
        Serial.println("Integrity Verified Successfully!\n");
      } else {
        Serial.println("Integrity Verification Failed!!!\n");
      }
    }
  }
}

void type_note(int chsn_slot) {
  if (read_file("/N" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextSize(1);
    tft.setTextColor(0xffff);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_key_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_cnt"));
    String decrypted_content = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/N" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_content;
    bool note_integrity = verify_integrity();

    if (note_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      
      disp_centered_text("to type \"Note\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_content);
      }
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(five_six_five_red_color);
      disp_centered_text("Integrity", 65);
      disp_centered_text("Verification", 85);
      disp_centered_text("Failed!!!", 105);
      tft.setTextSize(1);
      tft.setTextColor(0xffff);
      disp_centered_text("Press any key to return to the main menu", 232);
      press_any_key_to_continue();
    }
  }
}

// Functions for Notes (Above)

// Functions for Phone Numbers (Below)

void select_phone_number(byte what_to_do_with_it) {
  // 0 - Add phone_number
  // 1 - Edit phone_number
  // 2 - Delete phone_number
  // 3 - View phone_number
  delay(DELAY_FOR_SLOTS);
  curr_key = 1;

  header_for_select_phone_number(what_to_do_with_it);
  display_title_from_phone_number_without_integrity_verification();
  bool continue_to_next = false;
  while (continue_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 130)
        curr_key++;

      if (input_data == 129)
        curr_key--;

      if (curr_key < 1)
        curr_key = MAX_NUM_OF_RECS;

      if (curr_key > MAX_NUM_OF_RECS)
        curr_key = 1;

      if (input_data == 13 || input_data == 133) { // Enter
        int chsn_slot = curr_key;
        if (what_to_do_with_it == 0 && continue_to_next == false) {
          continue_to_next = true;
          add_phone_number_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 1 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          edit_phone_number_from_nint_controller(chsn_slot);
        }
        if (what_to_do_with_it == 2 && continue_to_next == false) {
          continue_to_next = true;
          delete_phone_number(chsn_slot);
        }
        if (what_to_do_with_it == 3 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          view_phone_number(chsn_slot);
        }
        if (what_to_do_with_it == 4 && continue_to_next == false) {
          continue_to_next = true;
          tft.fillScreen(0x0000);
          tft.setTextSize(1);
          tft.setTextColor(0xffff);
          tft.setCursor(0, 0);
          tft.print("Decrypting the record...");
          tft.setCursor(0, 10);
          tft.print("Please wait for a while.");
          type_phone_number(chsn_slot);
        }
        continue_to_next = true;
        break;
      }

      if (input_data == 8 || input_data == 27) {
        call_main_menu();
        continue_to_next = true;
        break;
      }
      delay(DELAY_FOR_SLOTS);
      header_for_select_phone_number(what_to_do_with_it);
      display_title_from_phone_number_without_integrity_verification();
    }
    delayMicroseconds(500);
  }
  return;
}

void header_for_select_phone_number(byte what_to_do_with_it) {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  if (what_to_do_with_it == 0) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add Phone to Slot " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Edit Phone Number " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 2) {
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("Delete Phone " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation_for_del();
  }
  if (what_to_do_with_it == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("View Phone Number " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
  if (what_to_do_with_it == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Type Phone Number " + String(curr_key) + "/" + String(MAX_NUM_OF_RECS), 5);
    disp_button_designation();
  }
}

void display_title_from_phone_number_without_integrity_verification() {
  tft.setTextSize(2);
  String encrypted_title = read_file("/P" + String(curr_key) + "_ttl");
  if (encrypted_title == "-1") {
    tft.setTextColor(0x07e0);
    disp_centered_text("Empty", 35);
  } else {
    clear_variables();
    decrypt_tag = false;
    decrypt_string_with_aes_in_cbc(encrypted_title);
    tft.setTextColor(0xffff);
    disp_centered_text(dec_st, 35);
  }
}

void add_phone_number_from_nint_controller(int chsn_slot) {
  enter_title_for_phone_number(chsn_slot);
  clear_variables();
  call_main_menu();
  return;
}

void enter_title_for_phone_number(int chsn_slot) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Title");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    enter_phone_number_for_phone_number(chsn_slot, input_from_the_ps2_keyboard);
  }
  return;
}

void enter_phone_number_for_phone_number(int chsn_slot, String entered_title) {
  act = true;
  clear_variables();
  set_stuff_for_input("Enter Phone Number");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    write_phone_number_to_flash(chsn_slot, entered_title, input_from_the_ps2_keyboard);
  }
  return;
}

void write_phone_number_to_flash(int chsn_slot, String entered_title, String entered_phone_number) {
  display_background_while_adding_record();
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  disp_text_b_w("Adding Phone Number To The Slot N" + String(chsn_slot) + "...", 5);
  disp_text_b_w("Please wait for a while.", 17);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_title);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/P" + String(chsn_slot) + "_ttl", dec_st);
  clear_variables();
  encrypt_string_with_aes_in_cbc(entered_phone_number);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/P" + String(chsn_slot) + "_cnt", dec_st);
  clear_variables();
  encrypt_tag_with_aes_in_cbc(entered_title + entered_phone_number);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/P" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void update_phone_number_and_tag(int chsn_slot, String new_phone_number) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Editing Phone Number In The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");

  clear_variables();
  encrypt_string_with_aes_in_cbc(new_phone_number);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/P" + String(chsn_slot) + "_cnt", dec_st);

  clear_variables();
  decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_ttl"));
  String decrypted_title = dec_st;

  clear_variables();
  encrypt_tag_with_aes_in_cbc(decrypted_title + new_phone_number);
  delay(DELAY_FOR_SLOTS);
  write_to_file_with_overwrite("/P" + String(chsn_slot) + "_tag", dec_st);
  return;
}

void edit_phone_number_from_nint_controller(int chsn_slot) {
  if (read_file("/P" + String(chsn_slot) + "_cnt") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_cnt"));
    String old_password = dec_st;
    act = true;
    clear_variables();
    set_stuff_for_input("Edit Phone Number");
    input_from_the_ps2_keyboard = old_password;
    disp();
    get_to_ps2_keyboard_input_tab();
    if (act == true) {
      update_phone_number_and_tag(chsn_slot, input_from_the_ps2_keyboard);
    }
  }
  return;
}

void delete_phone_number(int chsn_slot) {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Deleting Phone Number From The Slot N" + String(chsn_slot) + "...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  delete_file("/P" + String(chsn_slot) + "_tag");
  delete_file("/P" + String(chsn_slot) + "_ttl");
  delete_file("/P" + String(chsn_slot) + "_cnt");
  clear_variables();
  call_main_menu();
  return;
}

void view_phone_number(int chsn_slot) {
  if (read_file("/P" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextColor(0xffff);
    tft.setTextSize(1);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_button_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_cnt"));
    String decrypted_phone_number = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_phone_number;
    bool login_integrity = verify_integrity();

    if (login_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Phone Number:");
      tft.setTextColor(0xffff);
      tft.println(decrypted_phone_number);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, current_inact_clr);
      disp_centered_text(succs_ver_inscr, 232);
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setCursor(0, 5);
      tft.setTextColor(current_inact_clr);
      tft.print("Title:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_title);
      tft.setTextColor(current_inact_clr);
      tft.print("Phone Number:");
      tft.setTextColor(five_six_five_red_color);
      tft.println(decrypted_phone_number);
      tft.setTextSize(1);
      tft.fillRect(0, 230, 320, 14, 0x0000);
      tft.fillRect(312, 0, 8, 240, five_six_five_red_color);
      disp_centered_text(faild_ver_inscr, 232);
    }
    act = false;
    up_or_encdr_bttn_to_print();
    if (act == true) {
      Serial.println();
      Serial.print("Title:\"");
      Serial.print(decrypted_title);
      Serial.println("\"");
      Serial.print("Phone Number:\"");
      Serial.print(decrypted_phone_number);
      Serial.println("\"");
      if (login_integrity == true) {
        Serial.println("Integrity Verified Successfully!\n");
      } else {
        Serial.println("Integrity Verification Failed!!!\n");
      }
    }
  }
}

void type_phone_number(int chsn_slot) {
  if (read_file("/P" + String(chsn_slot) + "_ttl") == "-1") {
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07e0);
    tft.setTextSize(2);
    disp_centered_text("The Slot N" + String(chsn_slot) + " is Empty", 5);
    tft.setTextSize(1);
    tft.setTextColor(0xffff);
    disp_centered_text("Press any key to return to the main menu", 232);
    press_any_key_to_continue();
  } else {
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_ttl"));
    String decrypted_title = dec_st;
    clear_variables();
    decrypt_string_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_cnt"));
    String decrypted_phone_number = dec_st;
    clear_variables();
    decrypt_tag_with_aes_in_cbc(read_file("/P" + String(chsn_slot) + "_tag"));
    dec_st = decrypted_title + decrypted_phone_number;
    bool phone_number_integrity = verify_integrity();

    if (phone_number_integrity == true) {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(0xffff);
      disp_centered_text("Press the \"Upwards Arrow\"", 65);
      disp_centered_text("to type \"Phone Number\"", 85);
      act = false;
      up_or_encdr_bttn_to_print();
      if (act == true){
        typing_inscription();
        type_on_virtual_keyboard(decrypted_phone_number);
      }
    } else {
      tft.fillScreen(0x0000);
      tft.setTextSize(2);
      tft.setTextColor(five_six_five_red_color);
      disp_centered_text("Integrity", 65);
      disp_centered_text("Verification", 85);
      disp_centered_text("Failed!!!", 105);
      tft.setTextSize(1);
      tft.setTextColor(0xffff);
      disp_centered_text("Press any key to return to the main menu", 232);
      press_any_key_to_continue();
    }
  }
}

// Functions for Phone Number (Above)

void press_any_button_to_continue() {
  bool break_the_loop = false;
  while (break_the_loop == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {
      break_the_loop = true;
    }
    delay(read_keyboard_delay);
  }
}

void mvn_bcg(){
  if (display_moving_background == true) {
    if (chosen_lock_screen == 0) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Abu_Dhabi[(i + 7 + k) % 320][j + 57]);
        }
      }
    }
    
    if (chosen_lock_screen == 1) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Atlanta[(i + 7 + k) % 320][j + 57]);
        }
      }
    }
    
    if (chosen_lock_screen == 2) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Buildings[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 3) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Dallas[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 4) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Haifa[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 5) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Jakarta[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 6) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Jerusalem[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 7) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, London[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 8) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Miami[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 9) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Milan[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 10) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Pittsburgh[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 11) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Riyadh[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 12) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Singapore[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    if (chosen_lock_screen == 13) {
      for (int i = 0; i < 306; i++) {
        for (int j = 0; j < 77; j++) {
          if (mdb_icon[i][j] == 1)
            mvng_bc.drawPixel(i, j, Tel_Aviv[(i + 7 + k) % 320][j + 57]);
        }
      }
    }

    mvng_bc.pushSprite(7, 82, TFT_TRANSPARENT);
    k++;
  }
}

void show_moving_background() {
  mvng_bc.createSprite(306, 77);
  mvng_bc.setColorDepth(16);
  mvng_bc.fillSprite(TFT_TRANSPARENT);
  bool break_the_loop = false;
  while (break_the_loop == false) {
    byte input_data = get_PS2_keyboard_input_mvn_bcg();
    if (input_data > 0) {
      break_the_loop = true;
    }
  }
}

void up_or_encdr_bttn_to_print() {
  bool break_the_loop = false;
  while (break_the_loop == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 131 || input_data == 133) {
        act = true;
        break_the_loop = true;
      } else
        break_the_loop = true;
    }
    delayMicroseconds(read_keyboard_delay);
  }
}

void draw_full_height_pattern(){
  tft.fillScreen(65434);
  for (int i = 0; i < 120; i++) {
    for (int j = 0; j < 238; j++) {
      tft.drawPixel(i + 40, j + 1, half_pattern[i][j]);
    }
  }
  for (int i = 0; i < 120; i++) {
    for (int j = 0; j < 238; j++) {
      tft.drawPixel(i + 160, j + 1, half_pattern[119 - i][j]);
    }
  } 
}

void display_background_while_adding_record(){
  if (display_pattern_while_adding_and_printing_record == true){
    byte chosen_pattern = generate_random_number() % 4;
    if (chosen_pattern == 0){
      for (int n = 0; n < 240; n += 80) { // Columns
        for (int m = 0; m < 320; m += 80) { // Rows
          for (int i = 0; i < 80; i++) {
            for (int j = 0; j < 80; j++) {
              tft.drawPixel(i + m, j + n, pattern[i][j]);
            }
          }
        }
      }
    }
    if (chosen_pattern == 1){
      for (int n = 0; n < 240; n += 80) { // Columns
        for (int m = 0; m < 320; m += 80) { // Rows
          for (int i = 0; i < 80; i++) {
            for (int j = 0; j < 80; j++) {
              tft.drawPixel(i + m, j + n, pattern1[i][j]);
            }
          }
        }
      }
    }
    if (chosen_pattern == 2){
      for (int n = 0; n < 240; n += 80) { // Columns
        for (int m = 0; m < 320; m += 80) { // Rows
          for (int i = 0; i < 80; i++) {
            for (int j = 0; j < 80; j++) {
              tft.drawPixel(i + m, j + n, pattern2[i][j]);
            }
          }
        }
      }
    }
    if (chosen_pattern == 3){
      draw_full_height_pattern();
    }
  }
  else
    display_background_while_adding_record();
}

void display_lock_screen() {
  chosen_lock_screen = esp_random() % 14;

  if (chosen_lock_screen == 0){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Abu_Dhabi[i][j]);
      }
    }
  }
  
  if (chosen_lock_screen == 1){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Atlanta[i][j]);
      }
    }
  }
  
  if (chosen_lock_screen == 2){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Buildings[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 3){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Dallas[i][j]);
      }
    }
  }


  if (chosen_lock_screen == 4){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Haifa[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 5){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Jakarta[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 6){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Jerusalem[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 7){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, London[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 8){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Miami[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 9){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Milan[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 10){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Pittsburgh[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 11){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Riyadh[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 12){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Singapore[i][j]);
      }
    }
  }

  if (chosen_lock_screen == 13){
    for (int i = 0; i < 320; i++){
      for (int j = 0; j < 190; j++){
        tft.drawPixel(i, j + 25, Tel_Aviv[i][j]);
      }
    }
  }

}

void continue_to_unlock() {
  if (read_file("/mpass").equals("-1"))
    set_pass();
  else
    unlock_midbar();
  return;
}

void set_pass() {
  clear_variables();
  tft.fillScreen(0x0000);
  tft.setTextColor(0xffff);
  tft.setTextSize(1);
  set_stuff_for_input("Set Master Password");
  get_to_ps2_keyboard_input_tab();
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  draw_47px_midbar_inscription();
  tft.setTextColor(0xffff);
  disp_centered_text("Setting Master Password", 75);
  disp_centered_text("Please wait", 95);
  disp_centered_text("for a while", 115);
  //Serial.println(input_from_the_ps2_keyboard);
  input_from_the_ps2_keyboard;
  set_psswd();
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  draw_47px_midbar_inscription();
  tft.setTextColor(0xffff);
  disp_centered_text("Master Password Set", 75);
  disp_centered_text("Successfully", 95);
  disp_centered_text("Press Any Key", 115);
  disp_centered_text("To Continue", 135);
  press_any_key_to_continue();
  call_main_menu();
  return;
}

void set_psswd() {
  unsigned char salt[16];
  String string_salt;
  for (int i = 0; i < 16; i++) {
    salt[i] = generate_random_number();
    if (salt[i] < 16)
      string_salt += "0";
    string_salt += String(salt[i], HEX);
  }
  //Serial.println("Salt");
  //Serial.println(string_salt);
  char * password = new char[input_from_the_ps2_keyboard.length() + 1]; // Allocate memory for the C-style string
  strcpy(password, input_from_the_ps2_keyboard.c_str());
  size_t password_len = strlen(password);
  size_t salt_len = sizeof(salt);

  const size_t key_len = 96;
  unsigned char output_key[key_len];

  // Derive key using PBKDF2
  int ret = derive_key_with_pbkdf2(password, password_len, salt, salt_len, iterations, key_len, output_key);

  if (ret == 0) {
    char bytes_for_mp[32];
    for (int i = 0; i < 32; i++) {
      aes_key[i] = uint8_t(int(output_key[i]));
      hmackey[i] = byte(int(output_key[i + 32]));
      bytes_for_mp[i] = char(int(output_key[i + 64]));
    }
    write_to_file_with_overwrite("/salt", string_salt);
    //Serial.println("bytes_for_mp");
    //for (int i = 0; i < 32; i++) {
      //if (bytes_for_mp[i] < 16)
        //Serial.print("0");
      //Serial.print(String(bytes_for_mp[i], HEX));
    //}
    back_key();
    clear_variables();
    encrypt_iv_for_aes();
    int p = 0;
    for (int i = 0; i < 2; i++) {
      split_by_sixteen_for_encryption(bytes_for_mp, p, 1000);
      p += 16;
    }
    rest_key();
    //Serial.print("\nEncrypted Master Password Hash: ");
    //Serial.println(dec_st);
    write_to_file_with_overwrite("/mpass", dec_st);

    //Serial.println("AES key");
    //for (int i = 0; i < 32; i++) {
      //if (aes_key[i] < 16)
        //Serial.print("0");
      //Serial.print(String(aes_key[i], HEX));
    //}

    //Serial.println("HMAC key");
    //for (int i = 0; i < 32; i++) {
      //if (hmackey[i] < 16)
        //Serial.print("0");
      //Serial.print(String(hmackey[i], HEX));
    //}

  } else {
    tft.fillScreen(0x0000);
    tft.setTextSize(2);
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("System Error!!!", 55);
    disp_centered_text("Key Derivation Failed!", 75);
    tft.setTextColor(0xffff);
    disp_centered_text("Please reboot", 110);
    disp_centered_text("the device", 130);
    disp_centered_text("and try again", 150);
    for (;;)
      delay(1000);
  }

}

bool try_to_unl() {
  bool tags_match = false;
  
  String extr_salt = read_file("/salt");
  byte byte_salt[16];
  for (int i = 0; i < 32; i += 2) {
    if (i == 0) {
      if (extr_salt.charAt(i) != 0 && extr_salt.charAt(i + 1) != 0)
        byte_salt[i] = 16 * getNum(extr_salt.charAt(i)) + getNum(extr_salt.charAt(i + 1));
      if (extr_salt.charAt(i) != 0 && extr_salt.charAt(i + 1) == 0)
        byte_salt[i] = 16 * getNum(extr_salt.charAt(i));
      if (extr_salt.charAt(i) == 0 && extr_salt.charAt(i + 1) != 0)
        byte_salt[i] = getNum(extr_salt.charAt(i + 1));
      if (extr_salt.charAt(i) == 0 && extr_salt.charAt(i + 1) == 0)
        byte_salt[i] = 0;
    } else {
      if (extr_salt.charAt(i) != 0 && extr_salt.charAt(i + 1) != 0)
        byte_salt[i / 2] = 16 * getNum(extr_salt.charAt(i)) + getNum(extr_salt.charAt(i + 1));
      if (extr_salt.charAt(i) != 0 && extr_salt.charAt(i + 1) == 0)
        byte_salt[i / 2] = 16 * getNum(extr_salt.charAt(i));
      if (extr_salt.charAt(i) == 0 && extr_salt.charAt(i + 1) != 0)
        byte_salt[i / 2] = getNum(extr_salt.charAt(i + 1));
      if (extr_salt.charAt(i) == 0 && extr_salt.charAt(i + 1) == 0)
        byte_salt[i / 2] = 0;
    }
  }
  unsigned char salt[16];
  for (int i = 0; i < 16; i++){
    salt[i] = (unsigned char)byte_salt[i];
  }
  char * password = new char[input_from_the_ps2_keyboard.length() + 1]; // Allocate memory for the C-style string
  strcpy(password, input_from_the_ps2_keyboard.c_str());
  size_t password_len = strlen(password);
  size_t salt_len = sizeof(salt);

  const size_t key_len = 96;
  unsigned char output_key[key_len];

  // Derive key using PBKDF2
  int ret = derive_key_with_pbkdf2(password, password_len, salt, salt_len, iterations, key_len, output_key);

  if (ret == 0) {
    char bytes_for_mp[32];
    for (int i = 0; i < 32; i++) {
      aes_key[i] = uint8_t(int(output_key[i]));
      hmackey[i] = byte(int(output_key[i + 32]));
      bytes_for_mp[i] = char(int(output_key[i + 64]));
    }
    String computed_mp_hash;
    for (int i = 0; i < 32; i++) {
      if (bytes_for_mp[i] < 16)
        computed_mp_hash += "0";
      computed_mp_hash += String(bytes_for_mp[i], HEX);
    }
    
    decrypt_tag_with_aes_in_cbc(read_file("/mpass"));
    //Serial.println("Decrypted");
    //Serial.println(dec_tag);
    //Serial.println("Computed");
    //Serial.println(computed_mp_hash);
    if (dec_tag.equals(computed_mp_hash))
      tags_match = true;

  } else {
    tft.fillScreen(0x0000);
    tft.setTextSize(2);
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("System Error!!!", 55);
    disp_centered_text("Key Derivation Failed!", 75);
    tft.setTextColor(0xffff);
    disp_centered_text("Please reboot", 110);
    disp_centered_text("the device", 130);
    disp_centered_text("and try again", 150);
    for (;;)
      delay(1000);
  }
  return tags_match;
}


void unlock_midbar() {
  clear_variables();
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  set_stuff_for_input("Enter Master Password");
  get_to_starred_ps2_keyboard_input_tab();
  tft.fillScreen(0x0000);
  draw_47px_midbar_inscription();
  tft.setTextSize(2);
  tft.setTextColor(0xffff);
  disp_centered_text("Unlocking Midbar", 75);
  disp_centered_text("Please wait", 95);
  disp_centered_text("for a while", 115);
  //Serial.println(input_from_the_ps2_keyboard);
  bool next_act = try_to_unl();
  clear_variables();
  tft.fillScreen(0x0000);
  draw_47px_midbar_inscription();
  if (next_act == true) {
    tft.setTextSize(2);
    disp_centered_text("Midbar Unlocked", 75);
    disp_centered_text("Successfully", 95);
    disp_centered_text("Press Any Key", 115);
    disp_centered_text("To Continue", 135);
    press_any_key_to_continue();
    call_main_menu();
    return;
  } else {
    tft.setTextSize(2);
    tft.setTextColor(five_six_five_red_color);
    disp_centered_text("Wrong Password!", 75);
    tft.setTextColor(0xffff);
    disp_centered_text("Please reboot", 110);
    disp_centered_text("the device", 130);
    disp_centered_text("and try again", 150);
    for (;;)
      delay(1000);
  }
}

// Menu (below)

void disp_paste_smth_inscr(String what_to_pst) {
  tft.fillScreen(0x0000);
  tft.setTextColor(0xffff);
  tft.setTextSize(1);
  disp_centered_text("Paste", 10);
  disp_centered_text(what_to_pst, 20);
  disp_centered_text("To The Serial Terminal", 30);
  tft.setTextColor(five_six_five_red_color);
  disp_centered_text("Press Any Button", 110);
  disp_centered_text("To Cancel", 120);
}

void disp_button_designation() {
  tft.setTextSize(1);
  tft.setTextColor(0x07e0);
  tft.setCursor(0, 232);
  tft.print(" Enter - Continue                       ");
  tft.setTextColor(five_six_five_red_color);
  tft.print("Esc - Cancel");
}

void disp_button_designation_for_del() {
  tft.setTextSize(1);
  tft.setTextColor(five_six_five_red_color);
  tft.setCursor(0, 232);
  tft.print(" Enter - Continue                       ");
  tft.setTextColor(0x07e0);
  tft.print("Esc - Cancel");
}

void draw_47px_midbar_inscription() {
  for (int i = 0; i < 192; i++) {
    for (int j = 0; j < 47; j++) {
      if (midbar_inscr_47px_high[i][j] == 1)
        tft.drawPixel(i + 64, j + 5, current_inact_clr);
    }
  }
}

void call_main_menu() {
  rec_d = false;
  tft.fillScreen(0x0000);
  draw_47px_midbar_inscription();
  curr_pos = 0;
  main_menu();
}

void main_menu() {
  tft.setTextSize(2);
  byte sdown = 60;
  if (curr_pos == 0) {
    tft.setTextColor(0xffff);
    disp_centered_text("Logins", sdown + 10);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Credit Cards", sdown + 30);
    disp_centered_text("Notes", sdown + 50);
    disp_centered_text("Phone Numbers", sdown + 70);
    disp_centered_text("Hash Functions", sdown + 90);
    disp_centered_text("Factory Reset", sdown + 110);
  }
  if (curr_pos == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Logins", sdown + 10);
    tft.setTextColor(0xffff);
    disp_centered_text("Credit Cards", sdown + 30);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Notes", sdown + 50);
    disp_centered_text("Phone Numbers", sdown + 70);
    disp_centered_text("Hash Functions", sdown + 90);
    disp_centered_text("Factory Reset", sdown + 110);
  }
  if (curr_pos == 2) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Logins", sdown + 10);
    disp_centered_text("Credit Cards", sdown + 30);
    tft.setTextColor(0xffff);
    disp_centered_text("Notes", sdown + 50);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Phone Numbers", sdown + 70);
    disp_centered_text("Hash Functions", sdown + 90);
    disp_centered_text("Factory Reset", sdown + 110);
  }
  if (curr_pos == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Logins", sdown + 10);
    disp_centered_text("Credit Cards", sdown + 30);
    disp_centered_text("Notes", sdown + 50);
    tft.setTextColor(0xffff);
    disp_centered_text("Phone Numbers", sdown + 70);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Hash Functions", sdown + 90);
    disp_centered_text("Factory Reset", sdown + 110);
  }
  if (curr_pos == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Logins", sdown + 10);
    disp_centered_text("Credit Cards", sdown + 30);
    disp_centered_text("Notes", sdown + 50);
    disp_centered_text("Phone Numbers", sdown + 70);
    tft.setTextColor(0xffff);
    disp_centered_text("Hash Functions", sdown + 90);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Factory Reset", sdown + 110);
  }
  if (curr_pos == 5) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Logins", sdown + 10);
    disp_centered_text("Credit Cards", sdown + 30);
    disp_centered_text("Notes", sdown + 50);
    disp_centered_text("Phone Numbers", sdown + 70);
    disp_centered_text("Hash Functions", sdown + 90);
    tft.setTextColor(0xffff);
    disp_centered_text("Factory Reset", sdown + 110);
  }
}

void action_for_data_in_flash_menu(int curr_pos) {
  tft.setTextSize(2);
  byte sdown = 60;
  if (curr_pos == 0) {
    tft.setTextColor(0xffff);
    disp_centered_text("Add", sdown + 10);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Edit", sdown + 30);
    disp_centered_text("Delete", sdown + 50);
    disp_centered_text("View", sdown + 70);
    disp_centered_text("Type", sdown + 90);
  }
  if (curr_pos == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add", sdown + 10);
    tft.setTextColor(0xffff);
    disp_centered_text("Edit", sdown + 30);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Delete", sdown + 50);
    disp_centered_text("View", sdown + 70);
    disp_centered_text("Type", sdown + 90);
  }
  if (curr_pos == 2) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add", sdown + 10);
    disp_centered_text("Edit", sdown + 30);
    tft.setTextColor(0xffff);
    disp_centered_text("Delete", sdown + 50);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("View", sdown + 70);
    disp_centered_text("Type", sdown + 90);
  }
  if (curr_pos == 3) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add", sdown + 10);
    disp_centered_text("Edit", sdown + 30);
    disp_centered_text("Delete", sdown + 50);
    tft.setTextColor(0xffff);
    disp_centered_text("View", sdown + 70);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Type", sdown + 90);
  }
  if (curr_pos == 4) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("Add", sdown + 10);
    disp_centered_text("Edit", sdown + 30);
    disp_centered_text("Delete", sdown + 50);
    disp_centered_text("View", sdown + 70);
    tft.setTextColor(0xffff);
    disp_centered_text("Type", sdown + 90);
  }
}

void action_for_data_in_flash(String menu_title, byte record_type) {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(current_inact_clr);
  disp_centered_text(menu_title, 10);
  curr_key = 0;

  action_for_data_in_flash_menu(curr_key);
  disp_button_designation();
  bool cont_to_next = false;
  while (cont_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {

      if (input_data == 129 || input_data == 131)
        curr_key--;

      if (input_data == 130 || input_data == 132)
        curr_key++;

      if (curr_key < 0)
        curr_key = 4;

      if (curr_key > 4)
        curr_key = 0;

      if (input_data == 13 || input_data == 133) {
        if (curr_key == 0) {
          if (record_type == 0)
            select_login(0);
          if (record_type == 1)
            select_credit_card(0);
          if (record_type == 2)
            select_note(0);
          if (record_type == 3)
            select_phone_number(0);
          cont_to_next = true;
        }

        if (curr_key == 1 && cont_to_next == false) {
          if (record_type == 0)
            select_login(1);
          if (record_type == 1)
            select_credit_card(1);
          if (record_type == 2)
            select_note(1);
          if (record_type == 3)
            select_phone_number(1);
          cont_to_next = true;
        }

        if (curr_key == 2 && cont_to_next == false) {
          if (record_type == 0)
            select_login(2);
          if (record_type == 1)
            select_credit_card(2);
          if (record_type == 2)
            select_note(2);
          if (record_type == 3)
            select_phone_number(2);
          cont_to_next = true;
        }

        if (curr_key == 3 && cont_to_next == false) {
          if (record_type == 0)
            select_login(3);
          if (record_type == 1)
            select_credit_card(3);
          if (record_type == 2)
            select_note(3);
          if (record_type == 3)
            select_phone_number(3);
          cont_to_next = true;
        }

        if (curr_key == 4 && cont_to_next == false) {
          if (record_type == 0)
            select_login(4);
          if (record_type == 1)
            select_credit_card(4);
          if (record_type == 2)
            select_note(4);
          if (record_type == 3)
            select_phone_number(4);
          cont_to_next = true;
        }
      }
      
      if (input_data == 8 || input_data == 27) {
        cont_to_next = true;
      }
      action_for_data_in_flash_menu(curr_key);

    }
  }
  call_main_menu();
}

void hash_functions_menu(int curr_pos) {
  tft.setTextSize(2);
  byte sdown = 60;
  if (curr_pos == 0) {
    tft.setTextColor(0xffff);
    disp_centered_text("SHA-256", sdown + 10);
    tft.setTextColor(current_inact_clr);
    disp_centered_text("SHA-512", sdown + 30);
  }
  if (curr_pos == 1) {
    tft.setTextColor(current_inact_clr);
    disp_centered_text("SHA-256", sdown + 10);
    tft.setTextColor(0xffff);
    disp_centered_text("SHA-512", sdown + 30);
  }
}

void hash_functions() {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(current_inact_clr);
  disp_centered_text("Hash Functions", 10);
  curr_key = 0;
  hash_functions_menu(curr_key);
  disp_button_designation();
  bool cont_to_next = false;
  while (cont_to_next == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data > 0) {
      if (input_data == 129 || input_data == 131)
        curr_key--;

      if (input_data == 130 || input_data == 132)
        curr_key++;

      if (curr_key < 0)
        curr_key = 1;

      if (curr_key > 1)
        curr_key = 0;

      if (input_data == 13 || input_data == 133) {
        if (curr_key == 0) {
          hash_string_with_sha(false);
          cont_to_next = true;
        }

        if (curr_key == 1 && cont_to_next == false) {
          hash_string_with_sha(true);
          cont_to_next = true;
        }
      }
      if (input_data == 8 || input_data == 27) {
        cont_to_next = true;
      }
      hash_functions_menu(curr_key);

    }
  }
  call_main_menu();
}

// Menu (Above)

void Factory_Reset() {
  tft.fillScreen(0x0000);
  tft.setTextColor(five_six_five_red_color);
  disp_centered_text("Factory Reset", 10);
  delay(500);
  disp_centered_text("Attention!!!", 50);
  tft.setTextColor(0xffff);
  delay(500);
  disp_centered_text("All your data", 90);
  delay(500);
  disp_centered_text("will be lost!", 110);
  delay(500);
  tft.setTextColor(0x1557);
  disp_centered_text("Are you sure you want", 150);
  disp_centered_text("to continue?", 170);
  tft.setTextSize(1);
  delay(5000);
  disp_button_designation_for_del();
  finish_input = false;
  while (finish_input == false) {
    byte input_data = get_PS2_keyboard_input();
    if (input_data == 13 || input_data == 133) {
      perform_factory_reset();
      finish_input = true;
    }

    if (input_data == 8 || input_data == 27) {
      finish_input = true;
    }
    delayMicroseconds(read_keyboard_delay);
  }
  clear_variables();
  call_main_menu();
  return;
}

void perform_factory_reset() {
  tft.fillScreen(0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.print("Performing Factory Reset...");
  tft.setCursor(0, 10);
  tft.print("Please wait for a while.");
  delay(1000);
  delete_file("/mpass");
  delete_file("/salt");
  for (int i = 0; i < MAX_NUM_OF_RECS; i++) {
    delete_file("/L" + String(i + 1) + "_tag");
    delete_file("/L" + String(i + 1) + "_ttl");
    delete_file("/L" + String(i + 1) + "_usn");
    delete_file("/L" + String(i + 1) + "_psw");
    delete_file("/L" + String(i + 1) + "_wbs");
    delete_file("/C" + String(i + 1) + "_tag");
    delete_file("/C" + String(i + 1) + "_ttl");
    delete_file("/C" + String(i + 1) + "_hld");
    delete_file("/C" + String(i + 1) + "_nmr");
    delete_file("/C" + String(i + 1) + "_exp");
    delete_file("/C" + String(i + 1) + "_cvn");
    delete_file("/C" + String(i + 1) + "_pin");
    delete_file("/C" + String(i + 1) + "_zip");
    delete_file("/N" + String(i + 1) + "_tag");
    delete_file("/N" + String(i + 1) + "_ttl");
    delete_file("/N" + String(i + 1) + "_cnt");
    delete_file("/P" + String(i + 1) + "_tag");
    delete_file("/P" + String(i + 1) + "_ttl");
    delete_file("/P" + String(i + 1) + "_cnt");
    tft.fillRect(0, 10, 160, 10, 0x0000);
    tft.setCursor(0, 10);
    tft.print("Progress " + String((float(i + 1) / float(MAX_NUM_OF_RECS)) * 100) + "%");
  }
  tft.fillScreen(0x0000);
  tft.setTextColor(0xffff);
  disp_centered_text("DONE!", 10);
  disp_centered_text("Please Reboot", 30);
  disp_centered_text("The Device", 40);
  delay(1000);
  for (;;) {}
}

void hash_string_with_sha(bool vrsn) {
  act = true;
  clear_variables();
  tft.fillScreen(0x0000);
  tft.setTextColor(0xffff);
  tft.setTextSize(1);
  set_stuff_for_input("Enter string to hash:");
  get_to_ps2_keyboard_input_tab();
  if (act == true) {
    if (vrsn == false)
      hash_with_sha256();
    else
      hash_with_sha512();
  }
  clear_variables();
  curr_key = 0;
  call_main_menu();
  return;
}

void hash_with_sha256() {
  int str_len = input_from_the_ps2_keyboard.length() + 1;
  char keyb_inp_arr[str_len];
  input_from_the_ps2_keyboard.toCharArray(keyb_inp_arr, str_len);
  SHA256 hasher;
  hasher.doUpdate(keyb_inp_arr, strlen(keyb_inp_arr));
  byte authCode[SHA256_SIZE];
  hasher.doFinal(authCode);

  String res_hash;
  for (byte i = 0; i < SHA256HMAC_SIZE; i++) {
    if (authCode[i] < 0x10) {
      res_hash += 0;
    } {
      res_hash += String(authCode[i], HEX);
    }
  }
  tft.fillScreen(0x0000);
  tft.setTextColor(current_inact_clr);
  tft.setTextSize(2);
  disp_centered_text("Resulted hash", 10);
  tft.fillRect(312, 0, 8, 240, current_inact_clr);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 30);
  tft.println(res_hash);
  press_any_button_to_continue();
}

void hash_with_sha512() {
  int str_len = input_from_the_ps2_keyboard.length() + 1;
  char keyb_inp_arr[str_len];
  input_from_the_ps2_keyboard.toCharArray(keyb_inp_arr, str_len);
  std::string str = "";
  if (str_len > 1) {
    for (int i = 0; i < str_len - 1; i++) {
      str += keyb_inp_arr[i];
    }
  }
  String h = sha512(str).c_str();
  tft.fillScreen(0x0000);
  tft.setTextColor(current_inact_clr);
  tft.setTextSize(2);
  disp_centered_text("Resulted hash", 10);
  tft.fillRect(312, 0, 8, 240, current_inact_clr);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 30);
  tft.println(h);
  press_any_button_to_continue();
}

void Wifi_Init() {
  tft.setTextSize(1);
  tft.setTextColor(0xffff);
  tft.setCursor(0, 0);
  tft.println("Connecting to Wi-Fi");
  tft.setCursor(0, 12);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    tft.print("#");
  }
}

void firebase_init() {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  display_lock_screen();
  for (int i = 0; i < 306; i++) {
    for (int j = 0; j < 77; j++) {
      if (mdb_per[i][j] == 1)
        tft.drawPixel(i + 7, j + 82, 0xf7de);
    }
  }
  tft.setTextColor(0xf7de);
  disp_centered_text("Initializing Firebase", 4);
  disp_centered_text("Press Wait For A While", 220);
  // configure firebase API Key
  config.api_key = API_KEY;
  // configure firebase realtime database url
  config.database_url = DATABASE_URL;
  // Enable WiFi reconnection 
  Firebase.reconnectWiFi(true);
  if (Firebase.signUp( & config, & auth, "", "")) {
    isAuthenticated = true;
    fuid = auth.token.uid.c_str();
  } else {
    delay(40);
    isAuthenticated = false;
    tft.fillScreen(0x0000);
    tft.setTextColor(0xf800);
    tft.setTextSize(1);
    disp_centered_text("Failed To Initialize", 5);
    disp_centered_text("FIrebase", 20);
    disp_centered_text("Please reboot", sdown + 30);
    disp_centered_text("the device", sdown + 40);
    disp_centered_text("and try again", sdown + 50);
    while(1){
      delay(1000);
    }
  }
  // Assign the callback function for the long running token generation task, see addons/TokenHelper.h
  config.token_status_callback = tokenStatusCallback;
  // Initialise the firebase library
  Firebase.begin( & config, & auth);
}

void setup(void) {
  rec_d = false;
  k = 0;
  Wire.begin();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  
  tft.setTextSize(1);
  Serial.begin(115200);
  Wifi_Init();
  firebase_init();
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  display_lock_screen();
  keyboard.begin(DATAPIN, IRQPIN);
  keyboard.setNoBreak(1);
  keyboard.setNoRepeat(1);
  keymap.selectMap((char * )"US");

  for (int i = 0; i < 306; i++) {
    for (int j = 0; j < 77; j++) {
      if (mdb_per[i][j] == 1)
        tft.drawPixel(i + 7, j + 82, 0xf7de);
    }
  }
  m = 2; // Set AES to 256-bit mode
  clb_m = 4;
  tft.setTextSize(2);
  tft.setTextColor(0xf7de);
  disp_centered_text("Midbar Firebase Edition", 4);
  disp_centered_text("Press Any Key", 220);
  if (display_moving_background == true)
    show_moving_background();
  else
    press_any_button_to_continue();
  continue_to_unlock();
}

void loop() {
  byte input_data = get_PS2_keyboard_input();

  if (input_data == 13 || input_data == 133) { //Enter
    if (curr_pos == 0) {
      action_for_data_in_flash("Logins Menu", curr_pos);
    }
    if (curr_pos == 1) {
      action_for_data_in_flash("Credit Cards Menu", curr_pos);
    }
    if (curr_pos == 2) {
      action_for_data_in_flash("Notes Menu", curr_pos);
    }
    if (curr_pos == 3) {
      action_for_data_in_flash("Phone Numbers Menu", curr_pos);
    }
    if (curr_pos == 4) {
      hash_functions();
    }
    if (curr_pos == 5) {
      Factory_Reset();
    }
  }

  if (input_data == 130 || input_data == 132) {
    curr_pos++;
    if (curr_pos > 5)
      curr_pos = 0;
    main_menu();
  }

  if (input_data == 129 || input_data == 131) {
    curr_pos--;
    if (curr_pos < 0)
      curr_pos = 5;
    main_menu();
  }
}
