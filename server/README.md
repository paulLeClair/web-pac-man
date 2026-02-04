# Pacman Server

## Generating Protobuf files 

### Win32

Run the following command in the `web-pac-man/server` directory:

~~~~
protoc --proto_path=./protobuf --cpp_out=./protobuf/gen .\protobuf\*.proto
~~~~