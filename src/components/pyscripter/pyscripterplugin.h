#ifndef PYSCRIPTERPLUGIN_H
#define PYSCRIPTERPLUGIN_H

#include "plugin_type.h"
#include "pyscriptermodel.h"

using MiscPlugin = PluginBase<typestring_is("Misc Layer"), 0x555fc3, 57>;

struct PyScripterPlugin {
    using Model = PyScripterModel;
    static constexpr const char* name = "PyScripter";
    using PluginType = MiscPlugin;
};

#endif // PYSCRIPTERPLUGIN_H
