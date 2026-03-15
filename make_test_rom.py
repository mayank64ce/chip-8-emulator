opcodes = [0x6105, 0x7102, 0xA250, 0x1206]
with open('test.c8', 'wb') as f:
    for op in opcodes:
        f.write(op.to_bytes(2, byteorder='big'))