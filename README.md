# fix-3gp

Tries to fix a 3GP file using another file with the same format

For some reason we are still using audio/video formats that write important metadata when *finishing* the recording. So running out of battery, running out of disk space, a system hang, an app crash, or other million things can render all your recording useless.

untrunc (https://github.com/ponchio/untrunc) is a great tool to fix MP4/3GP files. If you tried to use untrunc and it didn't work because of an unsupported codec, this program may work. It's just a quick hack and there may be bugs, **backup** your files first.

It will read the broken file and another OK file with the same format, and write a new file using the recording data from the broken file, and the rest of the data from the OK file.

Only tested with 3GP + amr_wb (sawb) audio codec, it may work with other file formats like MP4, M4A, MOV, ... and with other codecs. Compiles fine with GCC on Linux

Reads both input files into memory, so it will refuse to load big files

