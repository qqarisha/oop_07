#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../npc.h"

class NPCFactory {
public:
    static std::shared_ptr<NPC> create(const std::string& type, const std::string& name, int x, int y);
    static std::shared_ptr<NPC> createRandom(const std::string& name, int x, int y);
    static std::vector<std::shared_ptr<NPC>> loadFromFile(const std::string& filename);
    static void saveToFile(const std::string& filename, const std::vector<std::shared_ptr<NPC>>& npcs);
};