# save as draw_test.py
opcodes = [
    0x00E0,  # Clear screen
    0x6000,  # Set V0 to 0 (X coordinate)
    0x6100,  # Set V1 to 0 (Y coordinate)
    0xA000,  # Set I to 0x000 (fontset location for '0')
    0xD015,  # Draw sprite at (V0, V1), height 5
    0x1208,  # Jump to 0x208 (infinite loop to keep window open)
]

with open('draw_test.c8', 'wb') as f:
    for op in opcodes:
        f.write(op.to_bytes(2, byteorder='big'))

print("Created draw_test.c8")