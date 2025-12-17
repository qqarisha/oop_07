#include "npc.h"
#include "visitor/visitor.h"

std::string Robber::type() const { return "Robber"; }
void Robber::accept(FightVisitor& v) { v.visit(*this); }