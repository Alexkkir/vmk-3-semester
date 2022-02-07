import telebot
from telebot import types
from compressor_api import perform_compression, perform_decompression
import os
from time import sleep

API_TOKEN = '5016396417:AAHB3i1BIEd5mBGUGBuH1GiTKjtv9JEV2Bs'

bot = telebot.TeleBot(API_TOKEN)


@bot.message_handler(content_types=['text'])
def edit(message):
    usr = message.from_user.id
    res_mes = bot.send_message(usr, '111')
    sleep(5)
    bot.edit_message_text(chat_id=usr, message_id=res_mes.id, text='222')
    bot.send_message(usr, 'Edited')


bot.infinity_polling()
