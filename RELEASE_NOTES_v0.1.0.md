# Astreus v0.1.0

Initial public release of Astreus - Package Downloader.

## Highlights
- Qt + QML desktop GUI for `.deb`, `.rpm`, `.rhel`
- Gruvbox themed responsive interface
- Browse-first flow (no manual command typing)
- Install package / Download dependencies
- Check dependencies after install
- Uninstall package
- Live command output log

## Build from source
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/home/g0x4d1cf/Qt/6.10.2/gcc_64
cmake --build build -j
./build/astreus-package-downloader
```
