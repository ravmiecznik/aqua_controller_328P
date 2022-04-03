#!/usr/bin/python3


import sys
import pyperclip
from subprocess import Popen, PIPE

p = Popen(['klipper'])		#works on Ubuntu/Kubuntu, allow to copy content to clipboard

deco_title = ' '.join(sys.argv[1:]).upper()
deco_len = 60
deco_char = '*'
comment_str = '//'
indent = 4

if len(deco_title) >= deco_len:
	deco_len = len(deco_title) + 2*indent

line = '\n' + comment_str + deco_char * (deco_len-len(comment_str)) + comment_str

formatted_title = comment_str + ' '*indent +deco_title + ' '*(deco_len-len(deco_title) - (2+1)*len(comment_str)) + comment_str

output = line + '\n' + formatted_title + line + '\n'

print(output)
pyperclip.copy(output)

print("Deco title in clipboard, CTR+V to paste...")
