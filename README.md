# web-pac-man

The goal with this project is to fully recreate the original Pacman experience using a C++ server and Solid frontend.

## Legal Disclaimer
Trademark & Copyright Notice

PAC-MAN is a registered trademark of Bandai Namco Entertainment Inc.

All original game assets, character designs, maze layouts, and sound effects are the intellectual property of Bandai Namco Entertainment Inc.

This project is an independent implementation created for educational purposes and is not affiliated with, endorsed by, or sponsored by Bandai Namco.

## Server 

- multithreaded C++ web server using boost::Beast, ASIO, and websockets to send Protobuf messages
- establishes game sessions and feeds in received user inputs

## Client

- Typescript Solid application with Tailwind
- sends user inputs, receives Protobuf game state messages and renders them reactively

