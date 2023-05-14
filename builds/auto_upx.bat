cd Release
upx --ultra-brute --no-align --best --compress-resources=1 ghost_terminal.exe
copy ghost_terminal.exe ghost_terminal_NUPX.exe
upx -d ghost_terminal_NUPX.exe
cd ..
