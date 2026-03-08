# Steganography Project (C)

This project implements image steganography using the Least Significant Bit (LSB) technique to hide secret text inside BMP images.

## Features
- Encode secret message into an image
- Decode hidden message from stego image
- Uses BMP image format
- Command-line based execution

## Technologies Used
- C Programming
- File Handling
- Bit Manipulation
- Structures

## How to Compile
gcc *.c 

## How to Run

Encode:
./steganography -e beautiful.bmp secret.txt stego.bmp

Decode:
./steganography -d stego.bmp decoded.txt

## Author
Indira Krupadarshini Midde
