#include "core/arith.h"
// All functions are header-only inlines; this TU exists so wizcore has a
// stable object file for the arith module.
namespace wiz::core { volatile int arith_module_link_anchor = 0; }
