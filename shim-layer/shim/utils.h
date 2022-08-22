/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_UTILS_H
#define CM_EMU_SHIM_UTILS_H

namespace shim {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <class Action> class FinalAction {
public:
  FinalAction(Action a) : action_(a) {}
  ~FinalAction() { action_(); }

private:
  Action action_;
};

template <class Action> FinalAction<Action> finally(Action a) { return a; }

} // namespace shim

#endif // CM_EMU_SHIM_UTILS_H
