#include "dut_base.hpp"

DutBase::DutBase()
{
    cycle = 0;
    argc  = 0;
    argv  = nullptr;
}

#if defined(USE_VCS)

DutVcsBase::DutVcsBase()
{
    // FATAL("VCS does not support no-args constructor");
    exit(-1);
}

DutVcsBase::DutVcsBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, argv);
};

DutVcsBase::DutVcsBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
};

DutVcsBase::DutVcsBase(std::initializer_list<const char *> args)
{
    int argc    = args.size();
    // Reserve heap space for VCS overflow ?
    char **argv = (char **)malloc(sizeof(char *) * (argc+128));
    memset(argv, -1, sizeof(char *) * (argc+128));
    int i       = 0;
    for (auto arg : args) {
        char *name = (char *)malloc(strlen(arg) + 1);
        strcpy(name, arg);
        argv[i++] = name;
    }
    this->init(argc, argv);
};

DutVcsBase::DutVcsBase(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;
    this->init(argc, argv);
};

void DutVcsBase::init(int argc, char **argv)
{
    // initialize VCS context
    VcsMain(argc, argv);

    // set VCS cycle to 0
    VcsInit();
    svSetScope(svGetScopeFromName("{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle       = 0;
    this->to_cycle[0] = 0;
};

DutVcsBase::~DutVcsBase(){};

int DutVcsBase::step()
{
    return DutVcsBase::step(1, 1);
};

int DutVcsBase::step_nodump()
{
    assert(0);
    return 0;
};

int DutVcsBase::step(bool dump)
{
    return DutVcsBase::step(dump, dump);
};

int DutVcsBase::step(uint64_t cycle, bool dump)
{
    // set cycle pointer
    this->cycle += cycle;
    this->to_cycle[0] = this->cycle;

    // run simulation
    VcsSimUntil(this->to_cycle);
    return 0;
};

int DutVcsBase::finalize()
{
    // finalize VCS context
    return 0;
};

#endif

#if defined(USE_VERILATOR)

DutVerilatorBase::DutVerilatorBase()
{
    this->init(0, nullptr);
}

DutVerilatorBase::DutVerilatorBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, argv);
};

DutVerilatorBase::DutVerilatorBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
};

DutVerilatorBase::DutVerilatorBase(std::initializer_list<const char *> args)
{
    int argc    = args.size();
    char **argv = (char **)malloc(sizeof(char *) * argc);
    int i       = 0;
    for (auto arg : args) {
        char *name = (char *)malloc(strlen(arg) + 1);
        strcpy(name, arg);
        argv[i++] = name;
    }
    this->init(argc, argv);
};

DutVerilatorBase::DutVerilatorBase(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;
    this->init(argc, argv);
};

void DutVerilatorBase::init(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;

    // initialize Verilator context
    Verilated::commandArgs(argc, argv);

    // create top module
    VerilatedContext *contextp = new VerilatedContext;
    contextp->randReset(2);
    contextp->debug(0);
    contextp->commandArgs(argc, argv);
    this->top = new V{{__TOP_MODULE_NAME__}} {contextp};

#if defined(VL_TRACE)
    contextp->traceEverOn(true);
#endif

    svSetScope(svGetScopeFromName("TOP.{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle = 0;
};

DutVerilatorBase::~DutVerilatorBase()
{
    // finalize Verilator context
    this->finalize();
};

int DutVerilatorBase::step()
{
    // push one more cycle
    return DutVerilatorBase::step(1, 1);
};

int DutVerilatorBase::step_nodump()
{
    return DutVerilatorBase::step(1, 0);
};

int DutVerilatorBase::step(bool dump)
{
    return DutVerilatorBase::step(1, dump);
};

int DutVerilatorBase::step(uint64_t ncycle, bool dump)
{
    // set cycle pointer
    this->cycle += ncycle;

    // run simulation
    if (dump) {
        for (int i = 0; i < ncycle; i++) {
            ((V{{__TOP_MODULE_NAME__}} *)(this->top))->eval();
            ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp()->timeInc(1);
        }
    } else {
        assert(ncycle == 1);
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->eval_step();
    }

    return 0;
};

int DutVerilatorBase::finalize()
{
    // finalize Verilator context
    if (this->top != nullptr) {
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->final();
        delete (V{{__TOP_MODULE_NAME__}} *)(this->top);
        this->top = nullptr;
    }
    return 0;
};

#endif
