import logging

import angr
import claripy

START_ADDR = 0x4010F4
HOOK_ADDR = 0x401106
HOOK_LENGTH = 5
SUCCESS_ADDR = 0x401203
ERROR_ADDR = 0x40143A

logging.getLogger("angr.sim_manager").setLevel(logging.DEBUG)
p = angr.Project("./bomb", auto_load_libs=False)
sym_vars = []


def bypass_parser(state):
    global sym_vars
    sym_vars = []

    for i in range(6):
        sym_var = claripy.BVS(f"num_{i}", 32)

        offset = i * 4
        state.memory.store(
            state.regs.rsp + offset, sym_var, endness=p.arch.memory_endness
        )

        state.solver.add(sym_var >= 1)
        state.solver.add(sym_var <= 6)

        for existing_var in sym_vars:
            state.solver.add(sym_var != existing_var)

        sym_vars.append(sym_var)


p.hook(HOOK_ADDR, bypass_parser, length=HOOK_LENGTH)


state = p.factory.blank_state(
    addr=START_ADDR,
    add_options={
        angr.options.ZERO_FILL_UNCONSTRAINED_MEMORY,
        angr.options.ZERO_FILL_UNCONSTRAINED_REGISTERS,
    },
)

sm = p.factory.simulation_manager(state)
sm.use_technique(angr.exploration_techniques.DFS())
sm.explore(find=SUCCESS_ADDR, avoid=ERROR_ADDR)

if sm.found:
    found_state = sm.found[0]
    print("[+] Path found!")

    solution = []
    for sym_var in sym_vars:
        val = found_state.solver.eval(sym_var)
        solution.append(str(val))

    print(f"Solution numbers: {' '.join(solution)}")
else:
    print("[-] No solution found.")
