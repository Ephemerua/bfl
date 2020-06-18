import angr
import sys

action = """ -A="addr=%s:call [clean]_afl_maybe_log(instrAddr)@afl" """

def gen_cfg(target):
    p = angr.Project(target, auto_load_libs = False)
    cfg = p.analyses.CFGFast()
    return cfg

def addr_map(cfg):
    return [i.addr for i in cfg.graph.nodes]

def make_cmdline(addr):
    cmdline = ""
    for i in addr:
        cmdline +=  hex(i)+"|"
    cmdline=cmdline[:-1]
    return action % cmdline

if __name__=="__main__":
    target = sys.argv[1]
    cfg = gen_cfg(target)
    print(make_cmdline(addr_map(cfg)))