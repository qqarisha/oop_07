#include "npc.h"
#include "visitor/visitor.h"

std::string Bear::type() const { return "Bear"; }
void Bear::accept(FightVisitor& v) { v.visit(*this); }