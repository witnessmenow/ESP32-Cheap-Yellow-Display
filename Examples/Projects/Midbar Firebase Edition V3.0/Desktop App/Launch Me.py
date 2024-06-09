"""
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
Credit:
https://www.pexels.com/photo/gray-and-black-hive-printed-textile-691710/
https://github.com/nishantprj/custom_tkinter_login
https://github.com/TomSchimansky/CustomTkinter/tree/master
"""
import tkinter as tk
from tkinter import *
import customtkinter
from PIL import ImageTk, Image
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import firebase_admin
from firebase_admin import db, credentials, initialize_app, storage
import random
import string
import numpy as np
import os
import time
import hmac
import hashlib
import secrets
from tkinter import messagebox
import textwrap

customtkinter.set_appearance_mode("dark")  # Modes: system (default), light, dark
customtkinter.set_default_color_theme("blue")  # Themes: blue (default), dark-blue, green

iterations = 20451
MAX_NUM_OF_RECS = 999

pointer_for_logins = 0
pointer_for_credit_cards = 0
pointer_for_notes = 0
pointer_for_phone_numbers = 0
login_title_preview_text = ""
credit_card_title_preview_text = ""
note_title_preview_text = ""
phone_number_title_preview_text = ""

string_for_data = ""
array_for_CBC_mode = bytearray(16)
back_aes_key = bytearray(32)
decract = 0

aes_key = bytearray([
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
])

hmackey = bytearray([
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
])

def back_aes_k():
    global back_aes_key
    back_aes_key = bytearray(aes_key)

def rest_aes_k():
    global aes_key
    aes_key = bytearray(back_aes_key)

def incr_aes_key():
    global aes_key
    aes_key = bytearray(aes_key)
    i = 15
    while i >= 0:
        if aes_key[i] == 255:
            aes_key[i] = 0
            i -= 1
        else:
            aes_key[i] += 1
            break

def encrypt_iv_for_aes(iv):
    global array_for_CBC_mode
    array_for_CBC_mode = bytearray(iv)
    encrypt_with_aes(bytearray(iv))

def encrypt_with_aes(to_be_encrypted):
    global string_for_data
    global decract
    to_be_encrypted = bytearray(to_be_encrypted)  # Convert to mutable bytearray
    if decract > 0:
        for i in range(16):
            to_be_encrypted[i] ^= array_for_CBC_mode[i]
            
    cipher = AES.new(aes_key, AES.MODE_ECB)
    encrypted_data = cipher.encrypt(pad(to_be_encrypted, AES.block_size))
    incr_aes_key()
    if decract > 0:
        for i in range(16):
            if i < 16:
                array_for_CBC_mode[i] = int(encrypted_data[i])
    
    for i in range(16):
        if encrypted_data[i] < 16:
            string_for_data += "0"
        string_for_data += hex(encrypted_data[i])[2:]
    
    decract += 11
    
def decrypt_string_with_aes_in_cbc(ct):
    global decract
    global array_for_CBC_mode
    global string_for_data
    back_aes_k()
    clear_variables()
    ct_bytes = bytes.fromhex(ct)
    ext = 0
    decract = -1
    while len(ct) > ext:
        split_for_decr(ct_bytes, ext)
        ext += 16
        decract += 10

    rest_aes_k()

def split_for_decr(ct, p):
    global decract
    global array_for_CBC_mode
    global string_for_data

    res = bytearray(16)
    prev_res = bytearray(16)
    br = False

    for i in range(0, 16):
        if i + p > len(ct) - 1:
            br = True
            break
        res[i] = ct[i + p]

    for i in range(0, 16):
        if i + p - 16 > len(ct) - 1:
            break  # Skip if index is out of bounds
        prev_res[i] = ct[i + p - 16]

    if not br:
        if decract > 16:
            array_for_CBC_mode = prev_res[:]

        cipher_text = res
        ret_text = bytearray(16)

        cipher = AES.new(bytes(aes_key), AES.MODE_ECB)
        ret_text = bytearray(cipher.decrypt(bytes(cipher_text)))

        incr_aes_key()

        if decract > 2:
            for i in range(16):
                ret_text[i] ^= array_for_CBC_mode[i]

            for byte in ret_text:
                if byte > 0:
                    string_for_data += chr(byte)
                    

        if decract == -1:
            array_for_CBC_mode = ret_text[:]

        decract += 1

def decrypt_tag_with_aes_in_cbc(ct):
    global decract
    global array_for_CBC_mode
    global string_for_data
    back_aes_k()
    clear_variables()
    ct_bytes = bytes.fromhex(ct)
    ext = 0
    decract = -1
    while len(ct) > ext:
        split_for_decr_hash(ct_bytes, ext)
        ext += 16
        decract += 10

    rest_aes_k()

def split_for_decr_hash(ct, p):
    global decract
    global array_for_CBC_mode
    global string_for_data

    res = bytearray(16)
    prev_res = bytearray(16)
    br = False

    for i in range(0, 16):
        if i + p > len(ct) - 1:
            br = True
            break
        res[i] = ct[i + p]

    for i in range(0, 16):
        if i + p - 16 > len(ct) - 1:
            break  # Skip if index is out of bounds
        prev_res[i] = ct[i + p - 16]

    if not br:
        if decract > 16:
            array_for_CBC_mode = prev_res[:]

        cipher_text = res
        ret_text = bytearray(16)

        cipher = AES.new(bytes(aes_key), AES.MODE_ECB)
        ret_text = bytearray(cipher.decrypt(bytes(cipher_text)))

        incr_aes_key()

        if decract > 2:
            for i in range(16):
                ret_text[i] ^= array_for_CBC_mode[i]

            string_for_data += ''.join(format(byte, '02x') for byte in ret_text)
                    

        if decract == -1:
            array_for_CBC_mode = ret_text[:]

        decract += 1

def clear_variables():
    global string_for_data
    global decract
    string_for_data = ""
    decract = 0

def encr_str_with_aes():
    global string_for_data
    global decract
    back_aes_k()
    string_for_data = ""
    decract = 0
    
    iv = [secrets.randbelow(256) for _ in range(16)]  # Initialization vector
    encrypt_iv_for_aes(iv)

def encrypt_string_with_aes_in_cbc(input_string):
    global string_for_data
    global decract
    back_aes_k()
    string_for_data = ""
    decract = 0
    
    iv = [secrets.randbelow(256) for _ in range(16)]  # Initialization vector
    encrypt_iv_for_aes(iv)
    padded_length = (len(input_string) + 15) // 16 * 16
    padded_string = input_string.ljust(padded_length, '\x00')
    byte_arrays = [bytearray(padded_string[i:i+16], 'utf-8') for i in range(0, len(padded_string), 16)]
    
    for i, byte_array in enumerate(byte_arrays):
        encrypt_with_aes(byte_array)
    
    rest_aes_k()
    
def encrypt_tag_with_aes_in_cbc(input_string):
    global string_for_data
    global decract
    back_aes_k()
    string_for_data = ""
    decract = 0
    
    iv = [secrets.randbelow(256) for _ in range(16)]  # Initialization vector
    encrypt_iv_for_aes(iv)
    
    byte_array = bytes.fromhex(compute_hmac_tag(input_string))
    array1 = byte_array[:16]
    array2 = byte_array[16:]
    encrypt_with_aes(array1)
    encrypt_with_aes(array2)
    rest_aes_k()

def compute_hmac_tag(input_string):
    input_bytes = input_string.encode('utf-8')
    hmac_tag = hmac.new(hmackey, input_bytes, hashlib.sha256).digest()
    hmac_tag_hex = hmac_tag.hex()
    return hmac_tag_hex

class ViewLoginForm:
    def __init__(self):
        self.view_record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, slot_number, title_text, login_text, password_text, website_text, integrity_label_text):
        if self.view_record_form is None:
            self.view_record_form = customtkinter.CTkToplevel()
            self.view_record_form.wm_attributes("-topmost", 1)
            self.view_record_form.geometry("520x680")
            self.view_record_form.title("Login From Slot N" + str(slot_number))
            vfl1 = customtkinter.CTkLabel(master=self.view_record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=500, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            global ttlentry
            ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            ttlentry.place(x=25, y=137)
            global loginentry
            loginlb = customtkinter.CTkLabel(master=frame, text="Login", width=300, font=('Segoe UI Semibold', 20))
            loginlb.place(x=20, y=170)
            loginentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            loginentry.place(x=25, y=207)
            global passwordentry
            passwordlb = customtkinter.CTkLabel(master=frame, text="Password", width=300, font=('Segoe UI Semibold', 20))
            passwordlb.place(x=20, y=250)
            passwordentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            passwordentry.place(x=25, y=287)
            global websiteentry
            websitelb = customtkinter.CTkLabel(master=frame, text="Website", width=300, font=('Segoe UI Semibold', 20))
            websitelb.place(x=20, y=320)
            websiteentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            websiteentry.place(x=25, y=357)
            button1 = customtkinter.CTkButton(master=frame, width=260, text="Ok", font=('Segoe UI Semibold', 20), corner_radius=6)
            button1.place(x=40, y=410)
            intvrlb = customtkinter.CTkLabel(master=frame, text=integrity_label_text, width=300, font=('Segoe UI Semibold', 16))
            intvrlb.place(x=20, y=460)

            # Bind the button to destroy the instance of the form
            button1.configure(command=self.on_ok_pressed)
            
            # Update text of all entry widgets
            ttlentry.delete(0, 'end')
            loginentry.delete(0, 'end')
            passwordentry.delete(0, 'end')
            websiteentry.delete(0, 'end')

            ttlentry.insert(0, title_text)
            loginentry.insert(0, login_text)
            passwordentry.insert(0, password_text)
            websiteentry.insert(0, website_text)

            ttlentry.configure(state='readonly')
            loginentry.configure(state='readonly')
            passwordentry.configure(state='readonly')
            websiteentry.configure(state='readonly')
            
            self.view_record_form.mainloop()

    def on_ok_pressed(self):
        # Destroy the form when the OK button is pressed
        self.view_record_form.destroy()   

class EditLoginForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln, login_ttl_lbl_text, usn_text, psw_text, wbs_text):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x620")
            self.record_form.title("Edit Login In Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=460, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            loginlb = customtkinter.CTkLabel(master=frame, text="Login", width=300, font=('Segoe UI Semibold', 20))
            loginlb.place(x=20, y=170)
            self.loginentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.loginentry.place(x=25, y=207)

            passwordlb = customtkinter.CTkLabel(master=frame, text="Password", width=300, font=('Segoe UI Semibold', 20))
            passwordlb.place(x=20, y=250)
            self.passwordentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.passwordentry.place(x=25, y=287)

            websitelb = customtkinter.CTkLabel(master=frame, text="Website", width=300, font=('Segoe UI Semibold', 20))
            websitelb.place(x=20, y=320)
            self.websiteentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.websiteentry.place(x=25, y=357)
            
            # Set the arguments to the text entries
            self.ttlentry.delete(0, 'end')
            self.loginentry.delete(0, 'end')
            self.passwordentry.delete(0, 'end')
            self.websiteentry.delete(0, 'end')

            self.ttlentry.insert(0, login_ttl_lbl_text)
            self.loginentry.insert(0, usn_text)
            self.passwordentry.insert(0, psw_text)
            self.websiteentry.insert(0, wbs_text)

            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Edit", "Clear", "Cancel"]
            functions = [self.on_edit_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Edit" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_edit_pressed(self):
        title = self.ttlentry.get()
        login = self.loginentry.get()
        password = self.passwordentry.get()
        website = self.websiteentry.get()
        return title, login, password, website

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.loginentry.delete(0, 'end')
        self.passwordentry.delete(0, 'end')
        self.websiteentry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None, None, None

class AddLoginForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x620")
            self.record_form.title("Add Login To Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=460, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            loginlb = customtkinter.CTkLabel(master=frame, text="Login", width=300, font=('Segoe UI Semibold', 20))
            loginlb.place(x=20, y=170)
            self.loginentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.loginentry.place(x=25, y=207)

            passwordlb = customtkinter.CTkLabel(master=frame, text="Password", width=300, font=('Segoe UI Semibold', 20))
            passwordlb.place(x=20, y=250)
            self.passwordentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.passwordentry.place(x=25, y=287)

            websitelb = customtkinter.CTkLabel(master=frame, text="Website", width=300, font=('Segoe UI Semibold', 20))
            websitelb.place(x=20, y=320)
            self.websiteentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.websiteentry.place(x=25, y=357)
            
            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Add", "Clear", "Cancel"]
            functions = [self.on_add_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Add" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_add_pressed(self):
        title = self.ttlentry.get()
        login = self.loginentry.get()
        password = self.passwordentry.get()
        website = self.websiteentry.get()
        return title, login, password, website

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.loginentry.delete(0, 'end')
        self.passwordentry.delete(0, 'end')
        self.websiteentry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None, None, None

class EditCreditCardForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln, card_title_lbl_text, cardholder_name_text, card_number_text, expiration_date_text, cvn_text, pin_text, zip_code_text):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x880")
            self.record_form.title("Edit Credit Card In Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=700, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            cardholder_lb = customtkinter.CTkLabel(master=frame, text="Cardholder Name", width=300, font=('Segoe UI Semibold', 20))
            cardholder_lb.place(x=20, y=170)
            self.cardholder_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.cardholder_entry.place(x=25, y=207)

            card_number_lb = customtkinter.CTkLabel(master=frame, text="Card Number", width=300, font=('Segoe UI Semibold', 20))
            card_number_lb.place(x=20, y=250)
            self.card_number_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.card_number_entry.place(x=25, y=287)

            expiration_date_lb = customtkinter.CTkLabel(master=frame, text="Expiration Date", width=300, font=('Segoe UI Semibold', 20))
            expiration_date_lb.place(x=20, y=320)
            self.expiration_date_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.expiration_date_entry.place(x=25, y=357)

            cvn_lb = customtkinter.CTkLabel(master=frame, text="CVN", width=300, font=('Segoe UI Semibold', 20))
            cvn_lb.place(x=20, y=390)
            self.cvn_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.cvn_entry.place(x=25, y=427)

            pin_lb = customtkinter.CTkLabel(master=frame, text="PIN", width=300, font=('Segoe UI Semibold', 20))
            pin_lb.place(x=20, y=460)
            self.pin_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.pin_entry.place(x=25, y=497)

            zip_code_lb = customtkinter.CTkLabel(master=frame, text="ZIP Code", width=300, font=('Segoe UI Semibold', 20))
            zip_code_lb.place(x=20, y=530)
            self.zip_code_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.zip_code_entry.place(x=25, y=567)
            
            # Set the arguments to the text entries
            self.ttlentry.delete(0, 'end')
            self.cardholder_entry.delete(0, 'end')
            self.card_number_entry.delete(0, 'end')
            self.expiration_date_entry.delete(0, 'end')
            self.cvn_entry.delete(0, 'end')
            self.pin_entry.delete(0, 'end')
            self.zip_code_entry.delete(0, 'end')

            self.ttlentry.insert(0, card_title_lbl_text)
            self.cardholder_entry.insert(0, cardholder_name_text)
            self.card_number_entry.insert(0, card_number_text)
            self.expiration_date_entry.insert(0, expiration_date_text)
            self.cvn_entry.insert(0, cvn_text)
            self.pin_entry.insert(0, pin_text)
            self.zip_code_entry.insert(0, zip_code_text)

            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Edit", "Clear", "Cancel"]
            functions = [self.on_edit_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Edit" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_edit_pressed(self):
        title = self.ttlentry.get()
        cardholder_name = self.cardholder_entry.get()
        card_number = self.card_number_entry.get()
        expiration_date = self.expiration_date_entry.get()
        cvn = self.cvn_entry.get()
        pin = self.pin_entry.get()
        zip_code = self.zip_code_entry.get()
        return title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.cardholder_entry.delete(0, 'end')
        self.card_number_entry.delete(0, 'end')
        self.expiration_date_entry.delete(0, 'end')
        self.cvn_entry.delete(0, 'end')
        self.pin_entry.delete(0, 'end')
        self.zip_code_entry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None, None, None, None, None, None

class ViewCreditCardForm:
    def __init__(self):
        self.view_record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, slot_number, title_text, cardholder_name_text, card_number_text, expiration_date_text, cvn_text, pin_text, zip_code_text, integrity_label_text):
        if self.view_record_form is None:
            self.view_record_form = customtkinter.CTkToplevel()
            self.view_record_form.wm_attributes("-topmost", 1)
            self.view_record_form.geometry("520x900")
            self.view_record_form.title("Credit Card From Slot N" + str(slot_number))
            vfl1 = customtkinter.CTkLabel(master=self.view_record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=720, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            global ttlentry
            ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            ttlentry.place(x=25, y=137)
            global cardholder_entry
            cardholder_lb = customtkinter.CTkLabel(master=frame, text="Cardholder Name", width=300, font=('Segoe UI Semibold', 20))
            cardholder_lb.place(x=20, y=170)
            cardholder_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            cardholder_entry.place(x=25, y=207)
            global card_number_entry
            card_number_lb = customtkinter.CTkLabel(master=frame, text="Card Number", width=300, font=('Segoe UI Semibold', 20))
            card_number_lb.place(x=20, y=250)
            card_number_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            card_number_entry.place(x=25, y=287)
            global expiration_date_entry
            expiration_date_lb = customtkinter.CTkLabel(master=frame, text="Expiration Date", width=300, font=('Segoe UI Semibold', 20))
            expiration_date_lb.place(x=20, y=320)
            expiration_date_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            expiration_date_entry.place(x=25, y=357)
            global cvn_entry
            cvn_lb = customtkinter.CTkLabel(master=frame, text="CVN", width=300, font=('Segoe UI Semibold', 20))
            cvn_lb.place(x=20, y=390)
            cvn_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            cvn_entry.place(x=25, y=427)
            global pin_entry
            pin_lb = customtkinter.CTkLabel(master=frame, text="PIN", width=300, font=('Segoe UI Semibold', 20))
            pin_lb.place(x=20, y=460)
            pin_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            pin_entry.place(x=25, y=497)
            global zip_code_entry
            zip_code_lb = customtkinter.CTkLabel(master=frame, text="ZIP Code", width=300, font=('Segoe UI Semibold', 20))
            zip_code_lb.place(x=20, y=530)
            zip_code_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            zip_code_entry.place(x=25, y=567)
            button1 = customtkinter.CTkButton(master=frame, width=260, text="Ok", font=('Segoe UI Semibold', 20), corner_radius=6)
            button1.place(x=40, y=620)
            intvrlb = customtkinter.CTkLabel(master=frame, text=integrity_label_text, width=300, font=('Segoe UI Semibold', 16))
            intvrlb.place(x=20, y=670)

            # Bind the button to destroy the instance of the form
            button1.configure(command=self.on_ok_pressed)
            
            # Update text of all entry widgets
            ttlentry.delete(0, 'end')
            cardholder_entry.delete(0, 'end')
            card_number_entry.delete(0, 'end')
            expiration_date_entry.delete(0, 'end')
            cvn_entry.delete(0, 'end')
            pin_entry.delete(0, 'end')
            zip_code_entry.delete(0, 'end')

            ttlentry.insert(0, title_text)
            cardholder_entry.insert(0, cardholder_name_text)
            card_number_entry.insert(0, card_number_text)
            expiration_date_entry.insert(0, expiration_date_text)
            cvn_entry.insert(0, cvn_text)
            pin_entry.insert(0, pin_text)
            zip_code_entry.insert(0, zip_code_text)

            ttlentry.configure(state='readonly')
            cardholder_entry.configure(state='readonly')
            card_number_entry.configure(state='readonly')
            expiration_date_entry.configure(state='readonly')
            cvn_entry.configure(state='readonly')
            pin_entry.configure(state='readonly')
            zip_code_entry.configure(state='readonly')
            
            self.view_record_form.mainloop()

    def on_ok_pressed(self):
        # Destroy the form when the OK button is pressed
        self.view_record_form.destroy()  

class AddCreditCardForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x880")
            self.record_form.title("Add Credit Card To Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=700, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            cardholder_lb = customtkinter.CTkLabel(master=frame, text="Cardholder Name", width=300, font=('Segoe UI Semibold', 20))
            cardholder_lb.place(x=20, y=170)
            self.cardholder_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.cardholder_entry.place(x=25, y=207)

            card_number_lb = customtkinter.CTkLabel(master=frame, text="Card Number", width=300, font=('Segoe UI Semibold', 20))
            card_number_lb.place(x=20, y=250)
            self.card_number_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.card_number_entry.place(x=25, y=287)

            expiration_date_lb = customtkinter.CTkLabel(master=frame, text="Expiration Date", width=300, font=('Segoe UI Semibold', 20))
            expiration_date_lb.place(x=20, y=320)
            self.expiration_date_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.expiration_date_entry.place(x=25, y=357)

            cvn_lb = customtkinter.CTkLabel(master=frame, text="CVN", width=300, font=('Segoe UI Semibold', 20))
            cvn_lb.place(x=20, y=390)
            self.cvn_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.cvn_entry.place(x=25, y=427)

            pin_lb = customtkinter.CTkLabel(master=frame, text="PIN", width=300, font=('Segoe UI Semibold', 20))
            pin_lb.place(x=20, y=460)
            self.pin_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.pin_entry.place(x=25, y=497)

            zip_code_lb = customtkinter.CTkLabel(master=frame, text="ZIP Code", width=300, font=('Segoe UI Semibold', 20))
            zip_code_lb.place(x=20, y=530)
            self.zip_code_entry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.zip_code_entry.place(x=25, y=567)
            
            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Add", "Clear", "Cancel"]
            functions = [self.on_add_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Add" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_add_pressed(self):
        title = self.ttlentry.get()
        cardholder_name = self.cardholder_entry.get()
        card_number = self.card_number_entry.get()
        expiration_date = self.expiration_date_entry.get()
        cvn = self.cvn_entry.get()
        pin = self.pin_entry.get()
        zip_code = self.zip_code_entry.get()
        return title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.cardholder_entry.delete(0, 'end')
        self.card_number_entry.delete(0, 'end')
        self.expiration_date_entry.delete(0, 'end')
        self.cvn_entry.delete(0, 'end')
        self.pin_entry.delete(0, 'end')
        self.zip_code_entry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None, None, None, None, None, None
    
class ViewNoteForm:
    def __init__(self):
        self.view_record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, slot_number, title_text, content_text, integrity_label_text):
        if self.view_record_form is None:
            self.view_record_form = customtkinter.CTkToplevel()
            self.view_record_form.wm_attributes("-topmost", 1)
            self.view_record_form.geometry("520x530")
            self.view_record_form.title("Note From Slot N" + str(slot_number))
            vfl1 = customtkinter.CTkLabel(master=self.view_record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=350, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            global ttlentry
            ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            ttlentry.place(x=25, y=137)
            global contententry
            contentlb = customtkinter.CTkLabel(master=frame, text="Content", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            contententry.place(x=25, y=207)
            button1 = customtkinter.CTkButton(master=frame, width=260, text="Ok", font=('Segoe UI Semibold', 20), corner_radius=6)
            button1.place(x=40, y=260)
            intvrlb = customtkinter.CTkLabel(master=frame, text=integrity_label_text, width=300, font=('Segoe UI Semibold', 16))
            intvrlb.place(x=20, y=310)

            # Bind the button to destroy the instance of the form
            button1.configure(command=self.on_ok_pressed)
            
            # Update text of all entry widgets
            ttlentry.delete(0, 'end')
            contententry.delete(0, 'end')

            ttlentry.insert(0, title_text)
            contententry.insert(0, content_text)

            ttlentry.configure(state='readonly')
            contententry.configure(state='readonly')
            
            self.view_record_form.mainloop()

    def on_ok_pressed(self):
        # Destroy the form when the OK button is pressed
        self.view_record_form.destroy()   

class EditNoteForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln, note_ttl_lbl_text, content_text):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x490")
            self.record_form.title("Edit Note In Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=310, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            contentlb = customtkinter.CTkLabel(master=frame, text="Content", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            self.contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.contententry.place(x=25, y=207)
            
            # Set the arguments to the text entries
            self.ttlentry.delete(0, 'end')
            self.contententry.delete(0, 'end')

            self.ttlentry.insert(0, note_ttl_lbl_text)
            self.contententry.insert(0, content_text)

            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Edit", "Clear", "Cancel"]
            functions = [self.on_edit_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Edit" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_edit_pressed(self):
        title = self.ttlentry.get()
        content = self.contententry.get()
        return title, content

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.contententry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None

class AddNoteForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x490")
            self.record_form.title("Add Note To Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=310, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            contentlb = customtkinter.CTkLabel(master=frame, text="Content", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            self.contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.contententry.place(x=25, y=207)
            
            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Add", "Clear", "Cancel"]
            functions = [self.on_add_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Add" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_add_pressed(self):
        title = self.ttlentry.get()
        content = self.contententry.get()
        return title, content

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.contententry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None
    
class ViewPhoneNumberForm:
    def __init__(self):
        self.view_record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, slot_number, title_text, content_text, integrity_label_text):
        if self.view_record_form is None:
            self.view_record_form = customtkinter.CTkToplevel()
            self.view_record_form.wm_attributes("-topmost", 1)
            self.view_record_form.geometry("520x530")
            self.view_record_form.title("Phone Number From Slot N" + str(slot_number))
            vfl1 = customtkinter.CTkLabel(master=self.view_record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=350, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            global ttlentry
            ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            ttlentry.place(x=25, y=137)
            global contententry
            contentlb = customtkinter.CTkLabel(master=frame, text="Phone Number", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            contententry.place(x=25, y=207)
            button1 = customtkinter.CTkButton(master=frame, width=260, text="Ok", font=('Segoe UI Semibold', 20), corner_radius=6)
            button1.place(x=40, y=260)
            intvrlb = customtkinter.CTkLabel(master=frame, text=integrity_label_text, width=300, font=('Segoe UI Semibold', 16))
            intvrlb.place(x=20, y=310)

            # Bind the button to destroy the instance of the form
            button1.configure(command=self.on_ok_pressed)
            
            # Update text of all entry widgets
            ttlentry.delete(0, 'end')
            contententry.delete(0, 'end')

            ttlentry.insert(0, title_text)
            contententry.insert(0, content_text)

            ttlentry.configure(state='readonly')
            contententry.configure(state='readonly')
            
            self.view_record_form.mainloop()

    def on_ok_pressed(self):
        # Destroy the form when the OK button is pressed
        self.view_record_form.destroy()   


class EditPhoneNumberForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln, phone_number_lbl_text, content_text):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x490")
            self.record_form.title("Edit Phone Number In Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=310, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            contentlb = customtkinter.CTkLabel(master=frame, text="Phone Number", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            self.contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.contententry.place(x=25, y=207)
            
            # Set the arguments to the text entries
            self.ttlentry.delete(0, 'end')
            self.contententry.delete(0, 'end')

            self.ttlentry.insert(0, phone_number_lbl_text)
            self.contententry.insert(0, content_text)

            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Edit", "Clear", "Cancel"]
            functions = [self.on_edit_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Edit" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_edit_pressed(self):
        title = self.ttlentry.get()
        content = self.contententry.get()
        return title, content

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.contententry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None


class AddPhoneNumberForm:
    def __init__(self):
        self.result = None
        self.record_form = None
        self.img12 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))

    def show_form(self, sln):
        if self.record_form is None:
            self.record_form = customtkinter.CTkToplevel()
            self.record_form.wm_attributes("-topmost", 1)
            self.record_form.geometry("520x490")
            self.record_form.title("Add Phone Number To Slot N" + str(sln))

            # Add the background image
            vfl1 = customtkinter.CTkLabel(master=self.record_form, image=self.img12)
            vfl1.pack()
            vfl1.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Add the frame for form elements
            frame = customtkinter.CTkFrame(master=vfl1, width=340, height=310, corner_radius=15)
            frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

            # Title label
            hl = customtkinter.CTkLabel(master=frame, text="מדבר", width=320, anchor="center", font=("Arial", 54, "bold"))
            hl.place(x=10, y=10)

            # Other form elements
            ttllb = customtkinter.CTkLabel(master=frame, text="Title", width=300, font=('Segoe UI Semibold', 20))
            ttllb.place(x=20, y=100)
            self.ttlentry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.ttlentry.place(x=25, y=137)

            contentlb = customtkinter.CTkLabel(master=frame, text="Phone Number", width=300, font=('Segoe UI Semibold', 20))
            contentlb.place(x=20, y=170)
            self.contententry = customtkinter.CTkEntry(master=frame, width=290, font=('Segoe UI Semibold', 16), placeholder_text="")
            self.contententry.place(x=25, y=207)
            
            # Add container with buttons at the bottom
            button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
            button_container.place(x=54, rely=1, y=-20, anchor="sw")

            button_names = ["Add", "Clear", "Cancel"]
            functions = [self.on_add_pressed, self.on_clear_pressed, self.on_cancel_pressed]  # Functions defined within the class

            for idx, (name, func) in enumerate(zip(button_names, functions)):
                if name == "Add" or name == "Cancel":
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=lambda f=func: self._close_and_return(f))
                else:
                    btn = customtkinter.CTkButton(button_container, width=70, text=name, command=func)
                btn.grid(row=0, column=idx * 2, padx=(0, 11), pady=0)

            self.record_form.grab_set()  # Make the form modal
            self.record_form.wait_window()  # Wait for the window to be closed
            return self.result

    def _close_and_return(self, func):
        self.result = func()
        self.record_form.destroy()

    def on_add_pressed(self):
        title = self.ttlentry.get()
        content = self.contententry.get()
        return title, content

    def on_clear_pressed(self):
        self.ttlentry.delete(0, 'end')
        self.contententry.delete(0, 'end')

    def on_cancel_pressed(self):
        return None, None

class App(customtkinter.CTk):
    def __init__(self):
        super().__init__()

        self.title_label_font = customtkinter.CTkFont(family="Arial", size=54, weight="bold")

        self.title("Midbar | מדבר")
        self.geometry("700x540")
        self.resizable(False, False)  # Prevent resizing
        
        left_arrow_unicode = "\u2190"
        right_arrow_unicode = "\u2192"
        
        # set grid layout 1x2
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)

        # create navigation frame
        self.navigation_frame = customtkinter.CTkFrame(self, corner_radius=0)
        self.navigation_frame.grid(row=0, column=0, sticky="nsew")
        self.navigation_frame.grid_rowconfigure(6, weight=1)

        self.navigation_frame_label = customtkinter.CTkLabel(self.navigation_frame, text="מדבר", font=self.title_label_font)
        self.navigation_frame_label.grid(row=0, column=0, padx=20, pady=20)

        self.frames = ["Logins", "Credit Cards", "Notes", "Phone Numbers", "About"]
        self.frame_buttons = []

        for idx, frame_name in enumerate(self.frames, start=1):
            button = customtkinter.CTkButton(self.navigation_frame, corner_radius=0, height=40, border_spacing=10,
                                             text=frame_name, fg_color="transparent",
                                             text_color=("gray10", "gray90"), hover_color=("gray70", "gray30"),
                                             anchor="w", command=lambda name=frame_name: self.select_frame_by_name(name))
            button.grid(row=idx, column=0, sticky="ew")
            self.frame_buttons.append(button)

        self.appearance_mode_menu = customtkinter.CTkOptionMenu(self.navigation_frame, values=["Light", "Dark", "System"],
                                                                command=self.change_appearance_mode_event)
        self.appearance_mode_menu.grid(row=7, column=0, padx=20, pady=20, sticky="s")

        # create frames
        self.frames_dict = {}
        for frame_name in self.frames:
            frame = customtkinter.CTkFrame(self, corner_radius=0, fg_color="transparent")
            self.frames_dict[frame_name] = frame
            frame.grid(row=0, column=1, sticky="nsew")

            if frame_name == "Logins":
                button1 = customtkinter.CTkButton(master=frame, width=28, text=left_arrow_unicode, command=move_left_login, corner_radius=6)
                button1.place(x=10, y=40)

                global login_slotlbl
                login_slotlbl = customtkinter.CTkLabel(master=frame, text="Press Any Arrow", anchor="center", width=420, font=('Century Gothic', 20))
                login_slotlbl.place(x=50, y=40)
                
                global login_ttl_lbl
                wrapped_text = textwrap.fill(login_title_preview_text, width=420)
                login_ttl_lbl = customtkinter.CTkLabel(master=frame, text=wrapped_text, anchor="center", font=('Century Gothic', 20), wraplength=440)
                login_ttl_lbl.place(x=50, y=110)

                button2 = customtkinter.CTkButton(master=frame, width=28, text=right_arrow_unicode, command=move_right_login, corner_radius=6)
                button2.place(x=482, y=40)
                
                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=12, rely=1, y=-20, anchor="sw")

                button_width = 91  # Width of each button
                space_width = 13  # Width of the space between buttons

                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=59, rely=1, y=-20, anchor="sw")

                button_names = ["Add_login", "Edit_login", "Delete_login", "View_login"]
                functions = [add_login, edit_login, delete_login, view_login]  # Assuming you have functions defined for each action

                for idx, (name, func) in enumerate(zip(button_names, functions)):
                    btn = customtkinter.CTkButton(button_container, width=button_width, text=name[:-6], command=lambda f=func: f())
                    btn.grid(row=0, column=idx * 2, padx=(0, space_width), pady=0)
                    
            if frame_name == "Credit Cards":
                button1 = customtkinter.CTkButton(master=frame, width=28, text=left_arrow_unicode, command=move_left_credit_card, corner_radius=6)
                button1.place(x=10, y=40)

                global credit_card_slotlbl
                credit_card_slotlbl = customtkinter.CTkLabel(master=frame, text="Press Any Arrow", anchor="center", width=420, font=('Century Gothic', 20))
                credit_card_slotlbl.place(x=50, y=40)
                
                global credit_card_ttl_lbl
                wrapped_text = textwrap.fill(credit_card_title_preview_text, width=420)
                credit_card_ttl_lbl = customtkinter.CTkLabel(master=frame, text=wrapped_text, anchor="center", font=('Century Gothic', 20), wraplength=440)
                credit_card_ttl_lbl.place(x=50, y=110)

                button2 = customtkinter.CTkButton(master=frame, width=28, text=right_arrow_unicode, command=move_right_credit_card, corner_radius=6)
                button2.place(x=482, y=40)
                
                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=12, rely=1, y=-20, anchor="sw")

                button_width = 91  # Width of each button
                space_width = 13  # Width of the space between buttons

                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=59, rely=1, y=-20, anchor="sw")

                button_names = ["Add_credit_card", "Edit_credit_card", "Delete_credit_card", "View_credit_card"]
                functions = [add_credit_card, edit_credit_card, delete_credit_card, view_credit_card]  # Assuming you have functions defined for each action

                for idx, (name, func) in enumerate(zip(button_names, functions)):
                    btn = customtkinter.CTkButton(button_container, width=button_width, text=name[:-12], command=lambda f=func: f())
                    btn.grid(row=0, column=idx * 2, padx=(0, space_width), pady=0)
                    
            if frame_name == "Notes":
                button1 = customtkinter.CTkButton(master=frame, width=28, text=left_arrow_unicode, command=move_left_note, corner_radius=6)
                button1.place(x=10, y=40)

                global note_slotlbl
                note_slotlbl = customtkinter.CTkLabel(master=frame, text="Press Any Arrow", anchor="center", width=420, font=('Century Gothic', 20))
                note_slotlbl.place(x=50, y=40)

                global note_ttl_lbl
                wrapped_text = textwrap.fill(note_title_preview_text, width=420)
                note_ttl_lbl = customtkinter.CTkLabel(master=frame, text=wrapped_text, anchor="center", font=('Century Gothic', 20), wraplength=440)
                note_ttl_lbl.place(x=50, y=110)

                button2 = customtkinter.CTkButton(master=frame, width=28, text=right_arrow_unicode, command=move_right_note, corner_radius=6)
                button2.place(x=482, y=40)

                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=12, rely=1, y=-20, anchor="sw")

                button_width = 91  # Width of each button
                space_width = 13  # Width of the space between buttons

                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=59, rely=1, y=-20, anchor="sw")

                button_names = ["Add_note", "Edit_note", "Delete_note", "View_note"]
                functions = [add_note, edit_note, delete_note, view_note]  # Assuming you have functions defined for each action

                for idx, (name, func) in enumerate(zip(button_names, functions)):
                    btn = customtkinter.CTkButton(button_container, width=button_width, text=name[:-5], command=lambda f=func: f())
                    btn.grid(row=0, column=idx * 2, padx=(0, space_width), pady=0)

            if frame_name == "Phone Numbers":
                button1 = customtkinter.CTkButton(master=frame, width=28, text=left_arrow_unicode, command=move_left_phone_number, corner_radius=6)
                button1.place(x=10, y=40)

                global phone_number_slotlbl
                phone_number_slotlbl = customtkinter.CTkLabel(master=frame, text="Press Any Arrow", anchor="center", width=420, font=('Century Gothic', 20))
                phone_number_slotlbl.place(x=50, y=40)

                global phone_number_lbl
                wrapped_text = textwrap.fill(phone_number_title_preview_text, width=420)
                phone_number_lbl = customtkinter.CTkLabel(master=frame, text=wrapped_text, anchor="center", font=('Century Gothic', 20), wraplength=440)
                phone_number_lbl.place(x=50, y=110)

                button2 = customtkinter.CTkButton(master=frame, width=28, text=right_arrow_unicode, command=move_right_phone_number, corner_radius=6)
                button2.place(x=482, y=40)

                # Add container with buttons at the bottom
                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=12, rely=1, y=-20, anchor="sw")

                button_width = 91  # Width of each button
                space_width = 13  # Width of the space between buttons

                button_container = customtkinter.CTkFrame(frame, corner_radius=0, fg_color="transparent")
                button_container.place(x=59, rely=1, y=-20, anchor="sw")

                button_names = ["Add_phone_number", "Edit_phone_number", "Delete_phone_number", "View_phone_number"]
                functions = [add_phone_number, edit_phone_number, delete_phone_number, view_phone_number]  # Assuming you have functions defined for each action

                for idx, (name, func) in enumerate(zip(button_names, functions)):
                    btn = customtkinter.CTkButton(button_container, width=button_width, text=name[:-13], command=lambda f=func: f())
                    btn.grid(row=0, column=idx * 2, padx=(0, space_width), pady=0)

            if frame_name == "About":
                
                abl1 = customtkinter.CTkLabel(master=frame, text="Midbar Firebase Edition V3.0", anchor="center", width=420, font=('Century Gothic', 20))
                abl1.place(x=50, y=40)
                abl2 = customtkinter.CTkLabel(master=frame, text="Desktop App", anchor="center", width=420, font=('Century Gothic', 20))
                abl2.place(x=50, y=90)
                abl3 = customtkinter.CTkLabel(master=frame, text="SOURCE CODE IS AVAILABLE AT:", anchor="center", width=420, font=('Century Gothic', 16))
                abl3.place(x=50, y=190)
                abet1 = customtkinter.CTkEntry(master=frame, width=420, font=('Century Gothic', 16), placeholder_text="github.com/Northstrix/Midbar-Firebase-Edition")
                abet1.place(x=50, y=240)
                abet1.configure(state='readonly')
                abet2 = customtkinter.CTkEntry(master=frame, width=420, font=('Century Gothic', 16), placeholder_text="sourceforge.net/projects/midbar-firebase-edition/")
                abet2.place(x=50, y=290)
                abet2.configure(state='readonly')
                abl3 = customtkinter.CTkLabel(master=frame, text="Copyright \u00A9 2024 Maxim Bortnikov", anchor="center", width=420, font=('Century Gothic', 20))
                abl3.place(x=50, y=490)

        # select default frame
        self.select_frame_by_name("Logins")

    def select_frame_by_name(self, name):
        # set button color for selected button
        for button in self.frame_buttons:
            if button.cget("text") == name:
                button.configure(fg_color=("gray75", "gray25"))
            else:
                button.configure(fg_color="transparent")

        # show selected frame
        for frame_name, frame in self.frames_dict.items():
            if frame_name == name:
                frame.grid(row=0, column=1, sticky="nsew")
            else:
                frame.grid_forget()

    def change_appearance_mode_event(self, new_appearance_mode):
        customtkinter.set_appearance_mode(new_appearance_mode)

    def select_frame_by_name(self, name):
        # set button color for selected button
        for button in self.frame_buttons:
            if button.cget("text") == name:
                button.configure(fg_color=("gray75", "gray25"))
            else:
                button.configure(fg_color="transparent")

        # show selected frame
        for frame_name, frame in self.frames_dict.items():
            if frame_name == name:
                frame.grid(row=0, column=1, sticky="nsew")
            else:
                frame.grid_forget()

    def change_appearance_mode_event(self, new_appearance_mode):
        customtkinter.set_appearance_mode(new_appearance_mode)

def move_left_note():
    global pointer_for_notes
    global MAX_NUM_OF_RECS
    pointer_for_notes -= 1
    if pointer_for_notes < 1:
        pointer_for_notes = MAX_NUM_OF_RECS
    update_cred_values_note()

def move_right_note():
    global pointer_for_notes
    global MAX_NUM_OF_RECS
    pointer_for_notes += 1
    if pointer_for_notes > MAX_NUM_OF_RECS:
        pointer_for_notes = 1
    update_cred_values_note()
    
def update_cred_values_note():
    global note_ttl_lbl
    note_ttl_lbl.configure(text='Loading...')
    global pointer_for_notes
    global MAX_NUM_OF_RECS
    note_slotlbl.configure(text="Note {}/{}".format(pointer_for_notes, MAX_NUM_OF_RECS))
    get_and_decrypt_note_title()
    
def get_and_decrypt_note_title():
    global note_ttl_lbl
    extr_encr_ttl = db.reference("/N{}_ttl".format(pointer_for_notes)).get()
    if extr_encr_ttl is None:
        note_ttl_lbl.configure(text='Empty')
    else:
        decrypt_string_with_aes_in_cbc(extr_encr_ttl)
        decrypted_ttl = string_for_data
        wrapped_text = textwrap.fill(string_for_data, width=420)
        note_ttl_lbl.configure(text=wrapped_text)
        
def get_and_decrypt_phone_number_title():
    global phone_number_lbl
    extr_encr_ttl = db.reference("/P{}_ttl".format(pointer_for_phone_numbers)).get()
    if extr_encr_ttl is None:
        phone_number_lbl.configure(text='Empty')
    else:
        decrypt_string_with_aes_in_cbc(extr_encr_ttl)
        decrypted_ttl = string_for_data
        wrapped_text = textwrap.fill(string_for_data, width=420)
        phone_number_lbl.configure(text=wrapped_text)

def move_left_phone_number():
    global pointer_for_phone_numbers
    global MAX_NUM_OF_RECS
    pointer_for_phone_numbers -= 1
    if pointer_for_phone_numbers < 1:
        pointer_for_phone_numbers = MAX_NUM_OF_RECS
    update_cred_values_phone_number()

def move_right_phone_number():
    global pointer_for_phone_numbers
    global MAX_NUM_OF_RECS
    pointer_for_phone_numbers += 1
    if pointer_for_phone_numbers > MAX_NUM_OF_RECS:
        pointer_for_phone_numbers = 1
    update_cred_values_phone_number()
    
def update_cred_values_phone_number():
    global phone_number_lbl
    phone_number_lbl.configure(text='Loading...')
    global pointer_for_phone_numbers
    global MAX_NUM_OF_RECS
    phone_number_slotlbl.configure(text="Phone Number {}/{}".format(pointer_for_phone_numbers, MAX_NUM_OF_RECS))
    get_and_decrypt_phone_number_title()

def move_left_login(event=None):
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    pointer_for_logins -= 1
    if pointer_for_logins < 1:
        pointer_for_logins = MAX_NUM_OF_RECS
    update_cred_values_login()

def move_right_login(event=None):
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    pointer_for_logins += 1
    if pointer_for_logins > MAX_NUM_OF_RECS:
        pointer_for_logins = 1
    update_cred_values_login()
    
def update_cred_values_login():
    global login_ttl_lbl
    login_ttl_lbl.configure(text='Loading...')
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    login_slotlbl.configure(text="Login {}/{}".format(pointer_for_logins, MAX_NUM_OF_RECS))
    get_and_decrypt_login_credential_title()
    
def get_and_decrypt_login_credential_title():
    global login_ttl_lbl
    extr_encr_ttl = db.reference("/L{}_ttl".format(pointer_for_logins)).get()
    if extr_encr_ttl is None:
        login_ttl_lbl.configure(text='Empty')
    else:
        decrypt_string_with_aes_in_cbc(extr_encr_ttl)
        decrypted_ttl = string_for_data
        wrapped_text = textwrap.fill(string_for_data, width=420)
        login_ttl_lbl.configure(text=wrapped_text)

def move_left_login():
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    pointer_for_logins -= 1
    if pointer_for_logins < 1:
        pointer_for_logins = MAX_NUM_OF_RECS
    update_cred_values_login()

def move_right_login():
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    pointer_for_logins += 1
    if pointer_for_logins > MAX_NUM_OF_RECS:
        pointer_for_logins = 1
    update_cred_values_login()
    
def update_cred_values_login():
    global login_ttl_lbl
    login_ttl_lbl.configure(text='Loading...')
    global pointer_for_logins
    global MAX_NUM_OF_RECS
    login_slotlbl.configure(text="Login {}/{}".format(pointer_for_logins, MAX_NUM_OF_RECS))
    get_and_decrypt_login_credential_title()
    
def get_and_decrypt_login_credential_title():
    global login_ttl_lbl
    extr_encr_ttl = db.reference("/L{}_ttl".format(pointer_for_logins)).get()
    if extr_encr_ttl is None:
        login_ttl_lbl.configure(text='Empty')
    else:
        decrypt_string_with_aes_in_cbc(extr_encr_ttl)
        decrypted_ttl = string_for_data
        wrapped_text = textwrap.fill(string_for_data, width=420)
        login_ttl_lbl.configure(text=wrapped_text)

def move_left_credit_card():
    global pointer_for_credit_cards
    global MAX_NUM_OF_RECS
    pointer_for_credit_cards -= 1
    if pointer_for_credit_cards < 1:
        pointer_for_credit_cards = MAX_NUM_OF_RECS
    update_cred_values_credit_card()

def move_right_credit_card():
    global pointer_for_credit_cards
    global MAX_NUM_OF_RECS
    pointer_for_credit_cards += 1
    if pointer_for_credit_cards > MAX_NUM_OF_RECS:
        pointer_for_credit_cards = 1
    update_cred_values_credit_card()
    
def update_cred_values_credit_card():
    global credit_card_ttl_lbl
    credit_card_ttl_lbl.configure(text='Loading...')
    global pointer_for_credit_cards
    global MAX_NUM_OF_RECS
    credit_card_slotlbl.configure(text="Credit Card {}/{}".format(pointer_for_credit_cards, MAX_NUM_OF_RECS))
    get_and_decrypt_credit_card_credential_title()
    
def get_and_decrypt_credit_card_credential_title():
    global credit_card_ttl_lbl
    extr_encr_ttl = db.reference("/C{}_ttl".format(pointer_for_credit_cards)).get()
    if extr_encr_ttl is None:
        credit_card_ttl_lbl.configure(text='Empty')
    else:
        decrypt_string_with_aes_in_cbc(extr_encr_ttl)
        decrypted_ttl = string_for_data
        wrapped_text = textwrap.fill(string_for_data, width=420)
        credit_card_ttl_lbl.configure(text=wrapped_text)

def unlock_app(entry_text, extr_encr_hash):
    app.destroy()
    if extr_encr_hash is None:
        set_password(entry_text)
    else:
        check_password(entry_text, extr_encr_hash)
    global aes_key
  
def set_password(entry_text):
    salt = [secrets.randbelow(256) for _ in range(16)]
    string_salt = ''.join(f'{x:02x}' for x in salt)
    db.reference("/").update({"salt" : string_salt})
    derived_key = derive_key_with_pbkdf2(entry_text, salt, 96)
    global aes_key
    global hmackey
    aes_key = derived_key[:32]
    hmackey = derived_key[32:64]
    bytes_for_mp = derived_key[64:]
    global string_for_data
    global decract
    back_aes_k()
    string_for_data = ""
    decract = 0
    iv = [secrets.randbelow(256) for _ in range(16)]  # Initialization vector
    encrypt_iv_for_aes(iv)
    array1 = bytes_for_mp[:16]
    array2 = bytes_for_mp[16:]
    encrypt_with_aes(array1)
    encrypt_with_aes(array2)
    rest_aes_k()
    db.reference("/").update({"mpass" : string_for_data})
    create_main_window()
    
def check_password(entry_text, extr_encr_hash):
    string_salt = db.reference("/salt").get()
    salt = [int(string_salt[i:i+2], 16) for i in range(0, len(string_salt), 2)]
    db.reference("/").update({"salt" : string_salt})
    derived_key = derive_key_with_pbkdf2(entry_text, salt, 96)
    global aes_key
    global hmackey
    aes_key = derived_key[:32]
    hmackey = derived_key[32:64]
    bytes_for_mp = derived_key[64:]
    extr_encr_pass_h = db.reference("/mpass").get()
    decrypt_tag_with_aes_in_cbc(extr_encr_pass_h)
    derived_value = ''.join(f'{x:02x}' for x in bytes_for_mp)
            
    if string_for_data == derived_value:
        create_main_window()
    else:
        messagebox.showerror("Error", "Wrong password!")

def derive_key_with_pbkdf2(password, salt, keylen):
    password_bytes = password.encode('utf-8')  # Encode password string to bytes
    salt_bytes = bytes(salt)  # Convert salt list to bytes
    dk = hashlib.pbkdf2_hmac('sha256', password_bytes, salt_bytes, iterations, dklen=keylen)
    return dk

def create_main_window():
    customtkinter.set_appearance_mode("dark")  # Modes: system (default), light, dark
    app = App()
    app.mainloop()
    
def add_login():
    if (pointer_for_logins == 0):
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        record_form_instance = AddLoginForm()
        result = record_form_instance.show_form(pointer_for_logins)
        title, login, password, website = result
        if any(field is None for field in [title, login, password, website]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_login_to_firebase(title, login, password, website)
            
    
def edit_login():
    if pointer_for_logins == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/L{}_ttl".format(pointer_for_logins)).get()
        extr_encr_usn = db.reference("/L{}_usn".format(pointer_for_logins)).get()
        extr_encr_psw = db.reference("/L{}_psw".format(pointer_for_logins)).get()
        extr_encr_wbs = db.reference("/L{}_wbs".format(pointer_for_logins)).get()
        extr_encr_tag = db.reference("/L{}_tag".format(pointer_for_logins)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            login_ttl_lbl_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            login_ttl_lbl_text = string_for_data

        if extr_encr_usn is None:
            usn_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_usn)
            usn_text = string_for_data

        if extr_encr_psw is None:
            psw_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_psw)
            psw_text = string_for_data

        if extr_encr_wbs is None:
            wbs_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_wbs)
            wbs_text = string_for_data
        record_form_instance = EditLoginForm()
        result = record_form_instance.show_form(pointer_for_logins, login_ttl_lbl_text, usn_text, psw_text, wbs_text)
        title, login, password, website = result
        if any(field is None for field in [title, login, password, website]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_login_to_firebase(title, login, password, website)
    
def set_login_to_firebase(entered_title, entered_username, entered_password, entered_website):
    encrypt_string_with_aes_in_cbc(entered_title)
    db.reference("/").update({"/L{}_ttl".format(pointer_for_logins) : string_for_data})
    encrypt_string_with_aes_in_cbc(entered_username)
    db.reference("/").update({"/L{}_usn".format(pointer_for_logins) : string_for_data})
    encrypt_string_with_aes_in_cbc(entered_password)
    db.reference("/").update({"/L{}_psw".format(pointer_for_logins) : string_for_data})
    encrypt_string_with_aes_in_cbc(entered_website)
    db.reference("/").update({"/L{}_wbs".format(pointer_for_logins) : string_for_data})
    title_username_password_website = entered_title + entered_username + entered_password + entered_website
    encrypt_tag_with_aes_in_cbc(title_username_password_website)
    db.reference("/").update({"/L{}_tag".format(pointer_for_logins) : string_for_data})
    update_cred_values_login()
    messagebox.showinfo("Midbar | מדבר", "Slot content modified successfully!")
    
def delete_login():
    if (pointer_for_logins == 0):
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        confirm_delete = messagebox.askyesno("Delete Login From Slot N{}".format(pointer_for_logins), "Are you sure you want to delete that record?")
        if confirm_delete:
            # Delete the files
            db.reference("/L{}_ttl".format(pointer_for_logins)).delete()
            db.reference("/L{}_usn".format(pointer_for_logins)).delete()
            db.reference("/L{}_psw".format(pointer_for_logins)).delete()
            db.reference("/L{}_wbs".format(pointer_for_logins)).delete()
            db.reference("/L{}_tag".format(pointer_for_logins)).delete()
            update_cred_values_login()
            messagebox.showinfo("Midbar | מדבר", "Record deleted successfully!")
        else:
            messagebox.showinfo("Midbar | מדבר", "Operation has been cancelled by user")
    
def view_login():
    if pointer_for_logins == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/L{}_ttl".format(pointer_for_logins)).get()
        extr_encr_usn = db.reference("/L{}_usn".format(pointer_for_logins)).get()
        extr_encr_psw = db.reference("/L{}_psw".format(pointer_for_logins)).get()
        extr_encr_wbs = db.reference("/L{}_wbs".format(pointer_for_logins)).get()
        extr_encr_tag = db.reference("/L{}_tag".format(pointer_for_logins)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            login_ttl_lbl_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            login_ttl_lbl_text = string_for_data

        if extr_encr_usn is None:
            usn_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_usn)
            usn_text = string_for_data

        if extr_encr_psw is None:
            psw_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_psw)
            psw_text = string_for_data

        if extr_encr_wbs is None:
            wbs_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_wbs)
            wbs_text = string_for_data
        
        decrypt_tag_with_aes_in_cbc(extr_encr_tag)
        intvst = ""
        if string_for_data == compute_hmac_tag(login_ttl_lbl_text + usn_text + psw_text + wbs_text):
            intvst = "Integrity Verified Successfully"
        else:
            intvst = "Integrity Verification Failed"
    view_record_form_instance = ViewLoginForm()
    view_record_form_instance.show_form(pointer_for_logins, login_ttl_lbl_text, usn_text, psw_text, wbs_text, intvst)

def add_credit_card():
    if pointer_for_credit_cards == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        record_form_instance = AddCreditCardForm()
        result = record_form_instance.show_form(pointer_for_credit_cards)
        title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code = result
        if any(field is None for field in [title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_credit_card_to_firebase(title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code)            
    
def edit_credit_card():
    if pointer_for_credit_cards == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/C{}_ttl".format(pointer_for_credit_cards)).get()
        extr_encr_hld = db.reference("/C{}_hld".format(pointer_for_credit_cards)).get()
        extr_encr_nmr = db.reference("/C{}_nmr".format(pointer_for_credit_cards)).get()
        extr_encr_exp = db.reference("/C{}_exp".format(pointer_for_credit_cards)).get()
        extr_encr_cvn = db.reference("/C{}_cvn".format(pointer_for_credit_cards)).get()
        extr_encr_pin = db.reference("/C{}_pin".format(pointer_for_credit_cards)).get()
        extr_encr_zip = db.reference("/C{}_zip".format(pointer_for_credit_cards)).get()
        extr_encr_tag = db.reference("/C{}_tag".format(pointer_for_credit_cards)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            card_title_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            card_title_text = string_for_data

        if extr_encr_hld is None:
            cardholder_name_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_hld)
            cardholder_name_text = string_for_data

        if extr_encr_nmr is None:
            card_number_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_nmr)
            card_number_text = string_for_data

        if extr_encr_exp is None:
            expiration_date_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_exp)
            expiration_date_text = string_for_data

        if extr_encr_cvn is None:
            cvn_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_cvn)
            cvn_text = string_for_data

        if extr_encr_pin is None:
            pin_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_pin)
            pin_text = string_for_data

        if extr_encr_zip is None:
            zip_code_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_zip)
            zip_code_text = string_for_data

        record_form_instance = EditCreditCardForm()
        result = record_form_instance.show_form(pointer_for_credit_cards, card_title_text, cardholder_name_text, card_number_text, expiration_date_text, cvn_text, pin_text, zip_code_text)
        title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code = result
        if any(field is None for field in [title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_credit_card_to_firebase(title, cardholder_name, card_number, expiration_date, cvn, pin, zip_code)
    
def set_credit_card_to_firebase(entered_title, entered_cardholder, entered_card_number, entered_expiry, entered_cvn, entered_pin, entered_zip_code):
    encrypt_string_with_aes_in_cbc(entered_title)
    db.reference("/").update({"/C{}_ttl".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_cardholder)
    db.reference("/").update({"/C{}_hld".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_card_number)
    db.reference("/").update({"/C{}_nmr".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_expiry)
    db.reference("/").update({"/C{}_exp".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_cvn)
    db.reference("/").update({"/C{}_cvn".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_pin)
    db.reference("/").update({"/C{}_pin".format(pointer_for_credit_cards): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_zip_code)
    db.reference("/").update({"/C{}_zip".format(pointer_for_credit_cards): string_for_data})
    card_title_cardholder_card_number_expiry_cvn_pin_zip_code = entered_title + entered_cardholder + entered_card_number + entered_expiry + entered_cvn + entered_pin + entered_zip_code
    encrypt_tag_with_aes_in_cbc(card_title_cardholder_card_number_expiry_cvn_pin_zip_code)
    db.reference("/").update({"/C{}_tag".format(pointer_for_credit_cards): string_for_data})
    update_cred_values_credit_card()
    messagebox.showinfo("Midbar | מדבר", "Slot content modified successfully!")

def delete_credit_card():
    if pointer_for_credit_cards == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        confirm_delete = messagebox.askyesno("Delete Credit Card From Slot N{}".format(pointer_for_credit_cards), "Are you sure you want to delete that record?")
        if confirm_delete:
            # Delete the files
            db.reference("/C{}_ttl".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_hld".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_nmr".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_exp".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_cvn".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_pin".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_zip".format(pointer_for_credit_cards)).delete()
            db.reference("/C{}_tag".format(pointer_for_credit_cards)).delete()
            update_cred_values_credit_card()
            messagebox.showinfo("Midbar | מדבר", "Record deleted successfully!")
        else:
            messagebox.showinfo("Midbar | מדבר", "Operation has been cancelled by user")

def view_credit_card():
    if pointer_for_credit_cards == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/C{}_ttl".format(pointer_for_credit_cards)).get()
        extr_encr_hld = db.reference("/C{}_hld".format(pointer_for_credit_cards)).get()
        extr_encr_nmr = db.reference("/C{}_nmr".format(pointer_for_credit_cards)).get()
        extr_encr_exp = db.reference("/C{}_exp".format(pointer_for_credit_cards)).get()
        extr_encr_cvn = db.reference("/C{}_cvn".format(pointer_for_credit_cards)).get()
        extr_encr_pin = db.reference("/C{}_pin".format(pointer_for_credit_cards)).get()
        extr_encr_zip = db.reference("/C{}_zip".format(pointer_for_credit_cards)).get()
        extr_encr_tag = db.reference("/C{}_tag".format(pointer_for_credit_cards)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            card_title_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            card_title_text = string_for_data

        if extr_encr_hld is None:
            cardholder_name_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_hld)
            cardholder_name_text = string_for_data

        if extr_encr_nmr is None:
            card_number_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_nmr)
            card_number_text = string_for_data

        if extr_encr_exp is None:
            expiration_date_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_exp)
            expiration_date_text = string_for_data

        if extr_encr_cvn is None:
            cvn_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_cvn)
            cvn_text = string_for_data

        if extr_encr_pin is None:
            pin_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_pin)
            pin_text = string_for_data

        if extr_encr_zip is None:
            zip_code_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_zip)
            zip_code_text = string_for_data

        decrypt_tag_with_aes_in_cbc(extr_encr_tag)
        integrity_label_text = ""
        if string_for_data == compute_hmac_tag(card_title_text + cardholder_name_text + card_number_text + expiration_date_text + cvn_text + pin_text + zip_code_text):
            integrity_label_text = "Integrity Verified Successfully"
        else:
            integrity_label_text = "Integrity Verification Failed"

        view_credit_card_form_instance = ViewCreditCardForm()
        view_credit_card_form_instance.show_form(pointer_for_credit_cards, card_title_text, cardholder_name_text, card_number_text, expiration_date_text, cvn_text, pin_text, zip_code_text, integrity_label_text)

def add_note():
    if pointer_for_notes == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        record_form_instance = AddNoteForm()
        result = record_form_instance.show_form(pointer_for_notes)
        title, content = result
        if any(field is None for field in [title, content]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_note_to_firebase(title, content)
    
def edit_note():
    if pointer_for_notes == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/N{}_ttl".format(pointer_for_notes)).get()
        extr_encr_cnt = db.reference("/N{}_cnt".format(pointer_for_notes)).get()
        extr_encr_tag = db.reference("/N{}_tag".format(pointer_for_notes)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            note_ttl_lbl_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            note_ttl_lbl_text = string_for_data

        if extr_encr_cnt is None:
            content_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_cnt)
            content_text = string_for_data

        record_form_instance = EditNoteForm()
        result = record_form_instance.show_form(pointer_for_notes, note_ttl_lbl_text, content_text)
        title, content = result
        if any(field is None for field in [title, content]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_note_to_firebase(title, content)
    
def set_note_to_firebase(entered_title, entered_content):
    encrypt_string_with_aes_in_cbc(entered_title)
    db.reference("/").update({"/N{}_ttl".format(pointer_for_notes): string_for_data})
    encrypt_string_with_aes_in_cbc(entered_content)
    db.reference("/").update({"/N{}_cnt".format(pointer_for_notes): string_for_data})
    title_content = entered_title + entered_content
    encrypt_tag_with_aes_in_cbc(title_content)
    db.reference("/").update({"/N{}_tag".format(pointer_for_notes): string_for_data})
    update_cred_values_note()
    messagebox.showinfo("Midbar | מדבר", "Slot content modified successfully!")
    
def delete_note():
    if pointer_for_notes == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        confirm_delete = messagebox.askyesno("Delete Note From Slot N{}".format(pointer_for_notes), "Are you sure you want to delete that record?")
        if confirm_delete:
            # Delete the files
            db.reference("/N{}_ttl".format(pointer_for_notes)).delete()
            db.reference("/N{}_cnt".format(pointer_for_notes)).delete()
            db.reference("/N{}_tag".format(pointer_for_notes)).delete()
            update_cred_values_note()
            messagebox.showinfo("Midbar | מדבר", "Record deleted successfully!")
        else:
            messagebox.showinfo("Midbar | מדבר", "Operation has been cancelled by user")

def view_note():
    if pointer_for_notes == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_ttl = db.reference("/N{}_ttl".format(pointer_for_notes)).get()
        extr_encr_cnt = db.reference("/N{}_cnt".format(pointer_for_notes)).get()
        extr_encr_tag = db.reference("/N{}_tag".format(pointer_for_notes)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_ttl is None:
            note_ttl_lbl_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_ttl)
            note_ttl_lbl_text = string_for_data

        if extr_encr_cnt is None:
            content_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_cnt)
            content_text = string_for_data
        
        decrypt_tag_with_aes_in_cbc(extr_encr_tag)
        intvst = ""
        if string_for_data == compute_hmac_tag(note_ttl_lbl_text + content_text):
            intvst = "Integrity Verified Successfully"
        else:
            intvst = "Integrity Verification Failed"
        view_record_form_instance = ViewNoteForm()
        view_record_form_instance.show_form(pointer_for_notes, note_ttl_lbl_text, content_text, intvst)

def add_phone_number():
    if pointer_for_phone_numbers == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        record_form_instance = AddPhoneNumberForm()
        result = record_form_instance.show_form(pointer_for_phone_numbers)
        number, contact_name = result
        if any(field is None for field in [number, contact_name]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_phone_number_to_firebase(number, contact_name)

def edit_phone_number():
    if pointer_for_phone_numbers == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_num = db.reference("/P{}_ttl".format(pointer_for_phone_numbers)).get()
        extr_encr_name = db.reference("/P{}_cnt".format(pointer_for_phone_numbers)).get()
        extr_encr_tag = db.reference("/P{}_tag".format(pointer_for_phone_numbers)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_num is None:
            number_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_num)
            number_text = string_for_data

        if extr_encr_name is None:
            name_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_name)
            name_text = string_for_data

        record_form_instance = EditPhoneNumberForm()
        result = record_form_instance.show_form(pointer_for_phone_numbers, number_text, name_text)
        number, contact_name = result
        if any(field is None for field in [number, contact_name]):
            messagebox.showwarning("Midbar | מדבר", "Operation was cancelled by user.")
        else:
            set_phone_number_to_firebase(number, contact_name)

def set_phone_number_to_firebase(entered_number, entered_contact_name):
    encrypt_string_with_aes_in_cbc(entered_number)
    db.reference("/").update({"/P{}_ttl".format(pointer_for_phone_numbers): string_for_data})
    clear_variables()
    encrypt_string_with_aes_in_cbc(entered_contact_name)
    db.reference("/").update({"/P{}_cnt".format(pointer_for_phone_numbers): string_for_data})
    clear_variables()
    encrypt_tag_with_aes_in_cbc(entered_number + entered_contact_name)
    db.reference("/").update({"/P{}_tag".format(pointer_for_phone_numbers): string_for_data})
    update_cred_values_phone_number()
    messagebox.showinfo("Midbar | מדבר", "Slot content modified successfully!")

def delete_phone_number():
    if pointer_for_phone_numbers == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        confirm_delete = messagebox.askyesno("Delete Phone Number From Slot N{}".format(pointer_for_phone_numbers), "Are you sure you want to delete that record?")
        if confirm_delete:
            # Delete the files
            db.reference("/P{}_ttl".format(pointer_for_phone_numbers)).delete()
            db.reference("/P{}_cnt".format(pointer_for_phone_numbers)).delete()
            db.reference("/P{}_tag".format(pointer_for_phone_numbers)).delete()
            update_cred_values_phone_number()
            messagebox.showinfo("Midbar | מדבר", "Record deleted successfully!")
        else:
            messagebox.showinfo("Midbar | מדבר", "Operation has been cancelled by user")

def view_phone_number():
    if pointer_for_phone_numbers == 0:
        messagebox.showwarning("Warning", "Select the slot to continue.")
    else:
        # Extract encrypted data from the database
        extr_encr_num = db.reference("/P{}_ttl".format(pointer_for_phone_numbers)).get()
        extr_encr_name = db.reference("/P{}_cnt".format(pointer_for_phone_numbers)).get()
        extr_encr_tag = db.reference("/P{}_tag".format(pointer_for_phone_numbers)).get()

        # Check if the extracted data is None and set "Failed To Retrieve!!!" text
        if extr_encr_num is None:
            number_text = 'Failed To Retrieve!!!'
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_num)
            number_text = string_for_data

        if extr_encr_name is None:
            name_text = "Failed To Retrieve!!!"
        else:
            decrypt_string_with_aes_in_cbc(extr_encr_name)
            name_text = string_for_data

        decrypt_tag_with_aes_in_cbc(extr_encr_tag)
        intvst = ""
        if string_for_data == compute_hmac_tag(number_text + name_text):
            intvst = "Integrity Verified Successfully"
        else:
            intvst = "Integrity Verification Failed"
        view_record_form_instance = ViewPhoneNumberForm()
        view_record_form_instance.show_form(pointer_for_phone_numbers, number_text, name_text, intvst)

if __name__ == "__main__":
    customtkinter.set_appearance_mode("light")  # Modes: system (default), light, dark
    db_url_file_name = open("db_url.txt", "r")
    db_url = db_url_file_name.read()
    db_url_file_name.close()
    cred = credentials.Certificate("firebase key.json")
    firebase_admin.initialize_app(cred, {"databaseURL": db_url})
    #db.reference("/").update({"Test File" : "Content"})
    extr_encr_hash = db.reference("/mpass").get()
    if extr_encr_hash is None:
        entry_hint = 'Set Your Master Password'
    else:
        entry_hint = 'Enter Your Master Password'
    app = customtkinter.CTk()  # creating custom tkinter window
    app.geometry("700x540")
    app.title("Midbar | מדבר")
    img1 = ImageTk.PhotoImage(Image.open("./assets/pattern.jpg"))
    l1 = customtkinter.CTkLabel(master=app, image=img1)
    l1.pack()
    # creating custom frame
    frame = customtkinter.CTkFrame(master=l1, width=300, height=220, corner_radius=15)
    frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

    l2 = customtkinter.CTkLabel(master=frame, text="Unlock Midbar", font=('Century Gothic', 20))
    l2.place(x=40, y=45)

    mpentry = customtkinter.CTkEntry(master=frame, width=220, placeholder_text=entry_hint, show="#")
    mpentry.place(x=40, y=95)

    # Create custom button
    button1 = customtkinter.CTkButton(master=frame, width=220, text="Continue", command=lambda: unlock_app(mpentry.get(), extr_encr_hash), corner_radius=6)
    button1.place(x=40, y=145)

    mpentry.bind("<Return>", lambda event: unlock_app(mpentry.get(), extr_encr_hash))

    app.mainloop()