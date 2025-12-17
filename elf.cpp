#include "npc.h"
#include "visitor/visitor.h"

std::string Elf::type() const { return "Elf"; }
void Elf::accept(FightVisitor& v) { v.visit(*this); }