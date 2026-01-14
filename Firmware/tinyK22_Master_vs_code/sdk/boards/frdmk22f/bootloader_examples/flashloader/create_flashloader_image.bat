rem Pass in full path and Release|Debug to iar project directory
cd /d %1
ielftool --bin %2\flashloader.out flashloader.bin
python bin\create_fl_image.py %2\flashloader.out flashloader.bin %2\flashloader_image.c
