#include <bits/stdc++.h>
#include "codegen/java.hpp"

namespace picker { namespace codegen {

    namespace java_ns {
        static const std::string xdata_declare_template =
            "    public XData {{pin_func_name}};\n";
        static const std::string xdata_init_template =
            "        this.{{pin_func_name}} = new XData({{logic_pin_length}}, XData.{{logic_pin_type}});\n";
        static const std::string xdata_bindrw_template =
            "        this.{{pin_func_name}}.BindDPIRW(libUT_{{moudle_name}}Constants.DPIR{{pin_func_name}}, libUT_{{moudle_name}}Constants.DPIW{{pin_func_name}});\n";
        static const std::string xport_add_template =
            "        this.port.Add(\"{{pin_func_name}}\", this.{{pin_func_name}});\n";
        static const std::string swig_constantr_template =
            "\%constant {{read_func_type}} =  get_{{pin_func_name}};\n";
        static const std::string swig_constantw_template =
            "\%constant {{write_func_type}} = set_{{pin_func_name}};\n";
        static const std::string swig_constant_fake_template =
            "\%constant {{write_func_type}} = NULL_DPI_{{null_write_func}};\n";

#define BIND_DPI_RW                                                            \
    if (pin[i].logic_pin_hb == -1) {                                           \
        data["logic_pin_length"] = 0;                                          \
        data["read_func_type"]   = "void (*DPIR{{pin_func_name}})(void*)";     \
        data["write_func_type"] =                                              \
            "void (*DPIW{{pin_func_name}})(const unsigned char)";              \
        data["null_write_func"] = "LW";                                        \
    } else {                                                                   \
        data["logic_pin_length"] =                                             \
            pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;                     \
        data["read_func_type"] = "void (*DPIR{{pin_func_name}})(void*)";       \
        data["write_func_type"] =                                              \
            "void (*DPIW{{pin_func_name}})(const void*)";                      \
        data["null_write_func"] = "VW";                                        \
    }

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin,
                                 std::string &moudle_name,
                                 std::string &xdata_decl,
                                 std::string &xdata_init,
                                 std::string &xdata_bindrw,
                                 std::string &xport_add,
                                 std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"] = pin[i].logic_pin;
                data["logic_pin_type"] =
                    (pin[i].logic_pin_type[0] == 'i') ? "In" : "Out";
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                BIND_DPI_RW;
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_decl = xdata_decl + env.render(xdata_declare_template, data);
                xdata_init = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bindrw_template, data);
                swig_constant =
                    swig_constant
                    + env.render(env.render(swig_constantr_template, data),
                                 data)
                    + env.render(env.render(swig_constantw_template, data),
                                 data);
                xport_add = xport_add + env.render(xport_add_template, data);
            }
        }

        /// @brief Export internal signal for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_internal_signal(std::vector<picker::sv_signal_define> pin,
                                    std::string &moudle_name,
                                    std::string &xdata_decl,
                                    std::string &xdata_init,
                                    std::string &xdata_bindrw,
                                    std::string &xport_add,
                                    std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                // Set 0 for 1bit singal or hb-lb+1 for vector signal for cpp
                // render
                BIND_DPI_RW;
                data["logic_pin_type"] = "Out";

                xdata_decl = xdata_decl + env.render(xdata_declare_template, data);
                xdata_init = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add = xport_add + env.render(xport_add_template, data);
                swig_constant =
                    swig_constant
                    + env.render(env.render(swig_constantr_template, data),
                                 data)
                    + env.render(env.render(swig_constant_fake_template, data),
                                 data);
            }
        };

    } // namespace java

    void java(picker::export_opts &opts,
                std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal)
    {
        std::string src_dir         = opts.source_dir + "/java";
        std::string dst_dir         = opts.target_dir + "/java";
        std::string src_module_name = opts.source_module_name;
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_decl, xdata_init, xdata_bindrw, xport_add, swig_constant;

        // Generate External Pin
        java_ns::render_external_pin(external_pin, dst_module_name, xdata_decl, xdata_init, xdata_bindrw,
                                xport_add, swig_constant);
        // Generate Internal Signal
        java_ns::render_internal_signal(internal_signal, dst_module_name, xdata_decl, xdata_init, xdata_bindrw,
                                   xport_add, swig_constant);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto java_location = picker::get_xcomm_lib("java/xspcomm-java.jar", erro_message);
        if (java_location.empty()) {
            FATAL("%s\n", erro_message.c_str());
        }
        data["__XSPCOMM_JAR__"] = java_location;
        data["__SOURCE_MODULE_NAME__"] = src_module_name;
        data["__TOP_MODULE_NAME__"]    = dst_module_name;

        data["__XDATA_DECL__"]    = xdata_decl;
        data["__XDATA_INIT__"]    = xdata_init;
        data["__XDATA_BIND__"]    = xdata_bindrw;
        data["__XPORT_ADD__"]     = xport_add;
        data["__SWIG_CONSTANT__"] = swig_constant;
        data["__USE_SIMULATOR__"] = "USE_" + simulator;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
}} // namespace picker::codegen
