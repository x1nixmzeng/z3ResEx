z3ResEx
=======

Extracts the filesystem from **RaiderZ** and **GunZ: The Second Duel** clients.


## Download

Latest build: ![z3ResEx.exe](https://raw.github.com/x1nixmzeng/z3ResEx/master/src/Release/z3ResEx.exe)

## Usage

    z3ResEx.exe [client_path] [-v|l]

To begin immediate extraction from the current directory, run from the command line without any arguments:

    z3ResEx.exe

To run without extracting any file data:

    z3ResEx.exe . -v

To list all files in the filesystem:

    z3ResEx.exe . -l

### Advanced Usage

Save all the filenames from the `Sound` archives to file:

    z3ResEx.exe "C:\Perfect World Entertainment\RaiderZ" -l |find "datadump/Data/Sound" > sounds.txt


## Compiling

The project was compiled with Microsoft Visual C++ 2010, and requires the Crypto++ library.

To compile z3ResEx, you need to first generate the Crypto++ `.lib` files, which is done by rebuilding the **cryptolib** project.


## Credits

Researched and coded by x1nixmzeng.

Thanks to Sir Kane who identified the Crypto++ method, to the members of the XeNTaX community, and those who shared download links to various clients.


## Usage Terms

This tool was written for personal use only. Please respect that the content you extract is owned by somebody else and should not be re-distributed without legal permissions from the copyright owner.
