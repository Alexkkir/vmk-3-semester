#!/usr/bin/python

# This is a simple echo bot using the decorator mechanism.
# It echoes any incoming text messages.

import telebot
from telebot import types, custom_filters
from compressor_api import perform_compression, perform_decompression
import os
from multiprocessing import Process

API_TOKEN = '5083861996:AAHX5hWEU3EtvtOVDCYYdggwcT1OJig7wm4'
IMAGE_DIR = './images'
BIN_DIR = './bin'
QUALITIES = [str(x) for x in [0, 20, 40, 60, 80, 100]]

user_dict = dict()
hide_board = types.ReplyKeyboardRemove()


class MyStates:
    option_select = 0

    get_image = 1
    choose_quality = 2
    compress = 3

    get_document = 4
    decompress = 5


bot = telebot.TeleBot(API_TOKEN)


class UserData:
    def __init__(self):
        self.path2image = None
        self.path2bin = None
        self.quality = 0
        self.attempts = 3

    def set_attempts(self, attempts: int) -> None:
        self.attempts = attempts

    def mark_mistake(self) -> None:
        self.attempts -= 1

    def not_blocked(self) -> bool:
        return self.attempts > 0

    def set_paths(self, path2image: str, path2bin: str) -> None:
        self.path2image = path2image
        self.path2bin = path2bin


def listener(messages):
    """
    When new messages arrive TeleBot will call this function.
    """
    for m in messages:
        if m.content_type == 'text':
            # print the sent message to the console
            print(str(m.chat.first_name) + " [" + str(m.chat.id) + "]: " + m.text)


@bot.message_handler(commands=['help'])
def help_msg(message):
    usr = message.from_user.id
    bot.send_message(usr, """\
This bot cat compress image using fractal compressor. Try it! Press /start. 
If you want to interrupt processing type /stop.""")


@bot.message_handler(commands=['start'])
def start(message):
    usr = message.from_user.id
    user_dict[usr] = UserData()

    choose_mode = types.ReplyKeyboardMarkup(one_time_keyboard=True)  # create the image selection keyboard
    choose_mode.add('compress', 'decompress')

    bot.send_message(usr, 'For help type /help.')
    bot.send_message(usr, "Please choose what you want to do: compress of decompress",
                     reply_markup=choose_mode)  # show the keyboard
    bot.set_state(usr, MyStates.option_select)


@bot.message_handler(state="*", commands=['stop'])
def stop(message):
    usr = message.from_user.id
    bot.send_message(usr, 'Received stop, but nothing happened (((')
    bot.delete_state(usr)
    pass


@bot.message_handler(state=MyStates.option_select, func=lambda msg: msg.text in ['compress', 'decompress'])
def option_select(message):
    usr = message.from_user.id
    text = message.text

    if text == 'compress':  # send the appropriate image based on the reply to the "/getImage" command
        user_dict[usr].set_attempts(3)
        bot.send_message(usr, 'Now send image, that you want to compress')
        bot.set_state(usr, MyStates.get_image)
    elif text == 'decompress':
        user_dict[usr].set_attempts(3)
        bot.send_message(usr, 'Now send binary file, that represents compressed image', reply_markup=hide_board)
        bot.set_state(usr, MyStates.get_document)


@bot.message_handler(state=MyStates.option_select, func=lambda msg: msg.text not in ['compress', 'decompress'])
def choose_option_err(message):
    usr = message.from_user.id
    if user_dict[usr].not_blocked():
        user_dict[usr].mark_mistake()
        bot.send_message(usr, "Please, use the predefined keyboard!")
        bot.send_message(usr, "Please try again")
        bot.register_next_step_handler(message, option_select)
    else:
        bot.delete_state(usr)


@bot.message_handler(state=MyStates.get_image, content_types='photo')
def get_image(message):
    usr = message.from_user.id
    user_dict[usr].set_attempts(3)

    fileID = message.photo[-1].file_id
    file_info = bot.get_file(fileID)
    downloaded_file = bot.download_file(file_info.file_path)

    filename = f'{usr}_{message.id}'
    path2image = f"{IMAGE_DIR}/{filename}.jpg"
    path2bin = f"{BIN_DIR}/{filename}.bin"
    with open(path2image, 'wb') as new_file:
        new_file.write(downloaded_file)

    user_dict[usr].set_paths(path2image=path2image, path2bin=path2bin)

    choose_quality_keyboard = types.ReplyKeyboardMarkup(one_time_keyboard=True)  # create the image selection keyboard
    choose_quality_keyboard.add(*QUALITIES)
    bot.send_message(usr, 'Choose quality', reply_markup=choose_quality_keyboard)
    bot.set_state(usr, MyStates.choose_quality)


@bot.message_handler(state=MyStates.get_image, func=lambda msg: msg.photo is None)
def get_image_err(message):
    usr = message.from_user.id
    if user_dict[usr].not_blocked():
        bot.send_message(usr, 'Please, send photo!')
        user_dict[usr].mark_mistake()
    else:
        bot.send_message(usr, 'Begin form /start')
        bot.delete_state(usr)


@bot.message_handler(state=MyStates.choose_quality, func=lambda msg: msg.text in QUALITIES)
def choose_quality(message):
    usr = message.from_user.id
    user_dict[usr].quality = int(message.text)

    user_dict[usr].set_attempts(3)
    user_data = user_dict[usr]
    bot.set_state(usr, MyStates.compress)
    compress(usr)


@bot.message_handler(state=MyStates.choose_quality, func=lambda msg: msg.text not in QUALITIES)
def choose_quality_err(message):
    usr = message.from_user.id
    if user_dict[usr].not_blocked():
        bot.send_message(usr, 'Please, use keyboard!')
        user_dict[usr].mark_mistake()
    else:
        bot.send_message(usr, 'Begin form /start')
        bot.delete_state(usr)


def compress(usr):
    user_data = user_dict[usr]
    path2image, path2bin, quality = user_data.path2image, user_data.path2bin, user_data.quality

    # process = Process(target=perform_compression, args = (path2image, path2bin, quality, API_TOKEN, usr))
    perform_compression(path2image, path2bin, quality, API_TOKEN, usr)
    # process.start()
    # process.join()

    bot.send_photo(usr, open(path2image, 'rb'))
    bot.send_document(usr, open(path2bin, 'rb'))
    os.remove(path2image)
    os.remove(path2bin)
    bot.send_message(usr, 'If you want to use bot again press /start')
    bot.delete_state(usr)


@bot.message_handler(state=MyStates.get_document, content_types=['document'])
def get_document(message):
    usr = message.from_user.id
    user_dict[usr].set_attempts(3)
    bot.set_state(usr, MyStates.decompress)
    decompress(message)


@bot.message_handler(state=MyStates.get_document, func=lambda msg: msg.document is None)
def get_document_err(message):
    usr = message.from_user.id
    if user_dict[usr].not_blocked():
        bot.send_message(usr, 'Please, send document!')
        user_dict[usr].mark_mistake()
    else:
        bot.send_message(usr, 'Begin form /start')
        bot.delete_state(usr)


def decompress(message):
    usr = message.from_user.id
    user_dict[usr].set_attempts(3)
    fileID = message.document.file_id
    file_info = bot.get_file(fileID)
    downloaded_file = bot.download_file(file_info.file_path)
    filename = f'{usr}_{message.id}'
    path2image = f"{IMAGE_DIR}/{filename}.jpg"
    path2bin = f"{BIN_DIR}/{filename}.bin"

    with open(path2bin, 'wb') as new_file:
        new_file.write(downloaded_file)

    perform_decompression(path2bin, path2image, API_TOKEN, usr)

    bot.send_photo(usr, open(path2image, 'rb'))
    os.remove(path2image)
    os.remove(path2bin)
    bot.send_message(usr, 'If you want to use bot again press /start')
    bot.delete_state(usr)


bot.add_custom_filter(custom_filters.StateFilter(bot))
bot.add_custom_filter(custom_filters.IsDigitFilter())
bot.set_update_listener(listener)
bot.infinity_polling()
