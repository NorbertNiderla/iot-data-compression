# IoT Data Compression
Library of IoT intended, data compression algorithms implementations.

### Features
Repository contains IoT intended, C implementations of:
- ALDC
- LEC
- SLEC
- FA-LEC
- MinDiff
- ND
- RAKE
- RGC
- Rice
- STOJ
- Sprintz
- Huffman
- tANS
- TMT

### Repository
`main` branch contains C library with compression and decompression functions implemented, while `tests` branch contains almost the same code, but rewritten as RiotOs RTOS (https://github.com/RIOT-OS/RIOT) application module. 

### Embedded system
Library was tested on *b-l072z-lrwan1 (ARM Cortex-M0+)*, *SLWSTK6221a (ARM Cortex-M4)* and *Nucleo-722ze (ARM Cortex-M7)* single-board microcontrollers, as module of RiotOs application.

