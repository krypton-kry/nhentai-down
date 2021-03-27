# Nhentai Down
 A small single threaded application to download from nhentai written in C++
 
## Usage:
    ./nhentaidown sourcenumber
    Example : ./nhentaidown 177013
It downloads and converts it into a .cbz file (which is lit)

(_Windows Users :_ This application can only be used in linux.)

P.S : Why not windows? Windows Suck!

## How to build manually
### Requirments:
In Debian use

```sudo apt install libcurlpp-dev libcurl4-openssl-dev```

to install dependencies

* [curlPP](https://github.com/jpbarrette/curlpp/)
* [RapidJson](https://github.com/Tencent/rapidjson)

#### Installing RapidJson
    git clone https://github.com/Tencent/rapidjson
    cd rapidjson
    sudo cp -r include/* /usr/local/include/

#### Compiling 
    git clone https://github.com/krypton-kry/nhentai-down
    cd nhentai-down
    chmod +x build.sh 
    ./bulid.sh

## Todo.
* Multiple Downloads at a time
* Proper Error Handling
* Code Cleanup
* Async

