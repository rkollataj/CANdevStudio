#ifndef PYSCRIPTERPLUGIN_H
#define PYSCRIPTERPLUGIN_H

#include "plugin_type.h"
#include "pyscriptermodel.h"

// Note that max typestring length is limited to 128 chars. 64 causes VS2015 internal error.
using MiscPlugin = PluginBase<typestring_is("Misc Layer"), 0x555fc3, 57>;

struct PyScripterPlugin {
    using Model = PyScripterModel;
    static constexpr const char* name = "PyScripter";
    using PluginType = MiscPlugin;
};

#endif // PYSCRIPTERPLUGIN_H
