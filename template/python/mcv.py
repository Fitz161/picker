#coding=utf-8
#python wrapper of mcv


{{python_dut_types}}


INVAL_UINT   = 123456789           # Magic value
INVAL_INT    = -12345678           # Magic value
INVAL_UINT64 = 12345678987654321   # Magic value
INVAL_BOOL   = -1                  # 0 and >0 is validate


class VerilatedCovContext:
    def defaultFilename(self)->str:
        raise NotImplementedError()
    def forcePerInstance(self, v:bool):
        raise NotImplementedError()
    def clear(self):
        raise NotImplementedError()
    def zero(self):
        raise NotImplementedError()
    def write(self, fname:str=""):
        raise NotImplementedError()
    def clearNonMatch(self, matchp:str):
        raise NotImplementedError()


class VerilatedVcdFile:
    def __init__(self):
        raise NotImplementedError()


class VerilatedVcdC:
    def __init__(self, f:VerilatedVcdFile = None):
        raise NotImplementedError()
    def isOpen(self)->bool:
        raise NotImplementedError()
    def openNext(self, incFilename:bool =True):
        raise NotImplementedError()
    def rolloverSize(self, size:int):
        raise NotImplementedError()
    def flush(self):
        raise NotImplementedError()
    def set_time_unit(self, unit:str):
        raise NotImplementedError()
    def set_time_resolution(self, unit:str):
        raise NotImplementedError()
    def dumpvars(self, level:int):
        raise NotImplementedError()
    def dump(self, timeui:int):
        raise NotImplementedError()
    def open(self, filename:str):
        raise NotImplementedError()
    def close(self):
        raise NotImplementedError()


class VerilatedContext:
    def __init__(self):
        raise NotImplementedError()
    def if_compile_trace(self)->bool:
        raise NotImplementedError()
    def commandArgs(self, cmd: str):
        raise NotImplementedError()
    def gotFinish(self) -> bool:
        raise NotImplementedError()
    def randReset(self, v:int=INVAL_INT) -> int:
        raise NotImplementedError()
    def randSeed(self, v:int=INVAL_INT) -> int:
        raise NotImplementedError()
    def assertOn(self, v:int=INVAL_BOOL) -> bool:
        raise NotImplementedError()
    def calcUnusedSigs(self, v:int=INVAL_BOOL) -> bool:
        raise NotImplementedError()
    def traceEverOn(self, v: bool):
        raise NotImplementedError()
    def timeInc(self, add:int):
        raise NotImplementedError()
    def internalsDump(self):
        raise NotImplementedError()
    def scopesDump(self):
        raise NotImplementedError()
    def dumpfileCheck(self) -> str:
        raise NotImplementedError()
    def dumpfile(self) -> str:
        raise NotImplementedError()
    def time(self, v: int=INVAL_UINT64) -> int:
        raise NotImplementedError()
    def timeunit(self, v:int=INVAL_INT) -> int:
        raise NotImplementedError()
    def timeprecision(self, v: int) -> int:
        raise NotImplementedError()
    def timeunitString(self) -> str:
        raise NotImplementedError()
    def timeprecisionString(self)->str:
        raise NotImplementedError()
    def threads(self, v: int=INVAL_UINT) -> int:
        raise NotImplementedError()
    def coveragep_write(self, fname: str):
        raise NotImplementedError()
    def commandArgsPlusMatch(self, prefixp:str)->str:
        raise NotImplementedError()
    def coveragep(self)->VerilatedCovContext:
        raise NotImplementedError()
    def errorCountInc(self,):
        raise NotImplementedError()
    def errorCount(self, v:int=INVAL_INT)->int:
        raise NotImplementedError()
    def errorLimit(self, v:int=INVAL_INT)->int:
        raise NotImplementedError()
    def fatalOnError(self, v:int=INVAL_BOOL)->bool:
        raise NotImplementedError()
    def fatalOnVpiError(self, v:int=INVAL_BOOL)->bool:
        raise NotImplementedError()
    def gotError(self, v:int=INVAL_BOOL)->bool:
        raise NotImplementedError()


class {{dut_name}}:
    {{python_dut_members}}
    def __init__(self, ctx:VerilatedContext, top:str="top"):
        raise NotImplementedError()
    {{python_dut_funcs}}


# overwrite the abow class
from _{{dut_name}} import *


def DUT(*args, top:str="top") -> (VerilatedContext, {{dut_name}}):
    ctx = VerilatedContext()
    ctx.commandArgs(" ".join(args))
    dut = {{dut_name}}(ctx, top)
    return ctx, dut


def default_test():
    ctx, dut = DUT()
    step = 0
    while not ctx.gotFinish():
        print("step: ", step, end="\r")
        dut.eval_step()
        step += 1
    print("test complete")
    del dut
    del ctx


if __name__ == "__main__":
    default_test()
