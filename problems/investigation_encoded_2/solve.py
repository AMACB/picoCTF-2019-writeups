import sys

d = {'111011101110101000': 'h', '1011101110111011101000': 'u', '10111011101110111000': '9', '111010101010111000': '3', '1110101110111010111000': 'w', '101011101110111000': 'a', '1010101000': 'l', '1010111000': 'o', '1110101110101110111000': '1', '10101010111000': 'd', '11101010111000': 'r', '1110101011101000': '7', '101010111000': 'p', '1011101011101000': '4', '111010101000': 'i', '101110101011101000': '5', '1010101110111000': 'b', '1110111011101110111000': '8', '11101110111010101000': 'y', '101110111000': 'q', '11101011101000': 'j', '10111010111010111000': '2', '1110111010101110111000': 'z', '10111011101011101000': 'v', '1110101000': 'k', '11101010101000': 'f', '10111000': 'c', '101010101000': 'e', '101011101110101000': '6', '1011101110111000': 'n', '1110101010111000': '0', '1110111010101000': 'g', '111010111011101000': 'x', '1000': 's', '101000': 'm', '10111010101000': 't'}
buf = ""
# `xxd -b -c1 output | cut -d" " -f2 | tr -d "\n"`
secret = "1011101010100011101011101011101110001010001110101010101110001110101010100011101011101011101110001110101010001110101010101110001011101010111010001110101010111000111010101011100011101010101110001110101010111000111010101011100011101010101110001110101010111000111010101011100011101010101110001110101010111000111010101011100011101010111010001110101110101110111000101010101000101010101110001011101011101000101110001110101010111000111011101110111011100000"
while len(secret) > 0:
    buf += secret[0]
    secret = secret[1:]
    if buf in d:
        sys.stdout.write(d[buf])
        buf = ""

sys.stdout.write('\n')