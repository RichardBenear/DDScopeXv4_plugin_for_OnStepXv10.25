# WiFi TFT Screen Mirror (WiFi Hand Controller)

### Author: Richard Benear  
### Last Modified: May 19, 2025

## Overview

This project adds a **WiFi-based TFT Screen Mirror** for telescope control to the existing DDScopeX telescope. The primary motivation was to control the telescope from an **iPhone** while retaining access to the existing TFT display screens. There are 17 different screens for controlling the DDScopeX which include astronomical catalogs and the Motor Control screen.

OnStep already provides a webpage interface, but it uses a separate screen rendering system and communicates using LX200 commands and catalog and motor control support are limited. This project provides a real-time mirror of the **actual TFT screen content** rendered by DDScopeX. In otherwords, what you see on the wired TFT Hand Controller is wirelessly sent to any web page capable device and displayed pixel-for-pixel. Additionally, touch or mouse clicks are sent back to the DDScope controller.

## Architecture

- **ESP32-S3**
  - Add an ESP32-S3 to act as a Wifi Server between the Teensy and the router.

- **Pixel Capture**:  
  - Pixels written to the TFT are "captured" via sofware and written in parallel to an **uncompressed buffer** in **PSRAM** on the Teensy 4.1 microprocessor.
- **Compression**
  - The uncompressed buffer is compressed by the Teensy with either RLE or Deflate (the default).
  
- **Memory**:  
  - A full raw frame is ~307,600 bytes.
  - The Teensy’s `DMAMEM` is limited to 512 KB, which is insufficient to hold both uncompressed and compressed images.
  - Therefore, an **8 MB PSRAM module** was added to the Teensy board.

- **Data Transfer**:  
  - Data is sent via **USB Host** on the Teensy 4.1 to the **ESP32-S3’s USB Device** port.
  - Earlier trial versions used a **1 Mbaud UART** connection to an ESP32-C3, but serial data drops due to overruns and noise made this unreliable.
  - USB provides a much **cleaner and more stable** communication channel.
  - Software-based flow control using RTS/CTS was considered but ultimately not required with USB.

- **Memory Optimization**:  
  - To fit the USB stack/library into the base Teensy RAM, large 2D arrays from the Custom Catalog were moved into `EXTMEM`.

## Compression

Various image compression strategies were tested to reduce transfer size:

| Method         | Typical Size | Ratio  | Notes                                                                 |
|----------------|--------------|--------|-----------------------------------------------------------------------|
| **Uncompressed** | ~307,600 B   | 1:1    | Full 320x480 16-bit image                                             |
| **RLE**          | ~70–90 KB    | ~4:1   | OK compression, simple implementation                                 |
| **LZ4**          | ~25–43 KB    | ~7:1   | Fast and reliable                                                     |
| **Deflate**      | ~9 KB        | ~35:1  | **Best performance** with acceptable compression size                             |
| **Difference**   | Smallest     | varies | Very compact but complex and less reliable |

## Performance

- **Update Rate**:  
  - The main TFT updates at **1 second intervals**.
  - The WiFi mirror display updates at the same rate.
  --The WiFi touch or mouse clicks have an update of 250 msec on the Teensy, although other tasks will delay the response sometimes.

- **Transfer Time**:  
  - With USB at **12 Mbit/sec**, a Deflate-compressed frame (~9 KB) transfers in **~70 ms**.

- **Default Compression**:  
  - **Deflate** is used by default due to its excellent size reduction and fast decompression.

## USB Connection ##
  - There are state machines on the Teensy and ESP32-S3 to handshake and synchronize the USB connection.
  - A WebSocket is used to send the compressed binary data to the Web Page where it is decompressed and rendered.

## Summary

This implementation offers a robust and efficient method for mirroring a Teensy-controlled TFT screen over WiFi using an ESP32-S3 and USB communication. The Deflate compression algorithm ensures minimal latency and efficient use of bandwidth while maintaining visual fidelity.

# WiFi TFT Screen Mirror (WiFi Hand Controller)

### Author: Richard Benear  
### Last Modified: May 19, 2025

## Overview

This project adds a **WiFi-based TFT Screen Mirror** for telescope control to the existing DDScopeX telescope. The primary motivation was to control the telescope from an **iPhone** while retaining access to the existing TFT display screens. There are 17 different screens for controlling the DDScopeX which include astronomical catalogs and the Motor Control screen.

OnStep already provides a webpage interface, but it uses a separate screen rendering system and communicates using LX200 commands and catalog and motor control support are limited. This project provides a real-time mirror of the **actual TFT screen content** rendered by DDScopeX. In otherwords, what you see on the wired TFT Hand Controller is wirelessly sent to any web page capable device and displayed pixel-for-pixel. Additionally, touch or mouse clicks are sent back to the DDScope controller.

## Architecture

- **ESP32-S3**
  - Add an ESP32-S3 to act as a Wifi Server between the Teensy and the router.

- **Pixel Capture**:  
  - Pixels written to the TFT are "captured" via sofware and written in parallel to an **uncompressed buffer** in **PSRAM** on the Teensy 4.1 microprocessor.
- **Compression**
  - The uncompressed buffer is compressed by the Teensy with either RLE or Deflate (the default).
  
- **Memory**:  
  - A full raw frame is ~307,600 bytes.
  - The Teensy’s `DMAMEM` is limited to 512 KB, which is insufficient to hold both uncompressed and compressed images.
  - Therefore, an **8 MB PSRAM module** was added to the Teensy board.

- **Data Transfer**:  
  - Data is sent via **USB Host** on the Teensy 4.1 to the **ESP32-S3’s USB Device** port.
  - Earlier trial versions used a **1 Mbaud UART** connection to an ESP32-C3, but serial data drops due to overruns and noise made this unreliable.
  - USB provides a much **cleaner and more stable** communication channel.
  - Software-based flow control using RTS/CTS was considered but ultimately not required with USB.

- **Memory Optimization**:  
  - To fit the USB stack/library into the base Teensy RAM, large 2D arrays from the Custom Catalog were moved into `EXTMEM`.

## Compression

Various image compression strategies were tested to reduce transfer size:

| Method         | Typical Size | Ratio  | Notes                                                                 |
|----------------|--------------|--------|-----------------------------------------------------------------------|
| **Uncompressed** | ~307,600 B   | 1:1    | Full 320x480 16-bit image                                             |
| **RLE**          | ~70–90 KB    | ~4:1   | OK compression, simple implementation                                 |
| **LZ4**          | ~25–43 KB    | ~7:1   | Fast and reliable                                                     |
| **Deflate**      | ~9 KB        | ~35:1  | **Best performance** with acceptable compression size                             |
| **Difference**   | Smallest     | varies | Very compact but complex and less reliable |

## Performance

- **Update Rate**:  
  - The main TFT updates at **1 second intervals**.
  - The WiFi mirror display updates at the same rate.
  --The WiFi touch or mouse clicks have an update of 250 msec on the Teensy, although other tasks will delay the response sometimes.

- **Transfer Time**:  
  - With USB at **12 Mbit/sec**, a Deflate-compressed frame (~9 KB) transfers in **~70 ms**.

- **Default Compression**:  
  - **Deflate** is used by default due to its excellent size reduction and fast decompression.

## USB Connection ##
  - There are state machines on the Teensy and ESP32-S3 to handshake and synchronize the USB connection.
  - A WebSocket is used to send the compressed binary data to the Web Page where it is decompressed and rendered.

## Summary

This implementation offers a robust and efficient method for mirroring a Teensy-controlled TFT screen over WiFi using an ESP32-S3 and USB communication. The Deflate compression algorithm ensures minimal latency and efficient use of bandwidth while maintaining visual fidelity.
