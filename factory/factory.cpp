#include "factory.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>

std::shared_ptr<NPC> NPCFactory::create(const std::string& type, const std::string& name, int x, int y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> attack_dist(15, 35);
    std::uniform_int_distribution<> defense_dist(10, 30);
    
    if (type == "bear") {
        return std::make_shared<Bear>(name, x, y, attack_dist(gen), defense_dist(gen));
    }
    if (type == "elf") {
        return std::make_shared<Elf>(name, x, y, attack_dist(gen), defense_dist(gen));
    }
    if (type == "robber") {
        return std::make_shared<Robber>(name, x, y, attack_dist(gen), defense_dist(gen));
    }
    return nullptr;
}

std::shared_ptr<NPC> NPCFactory::createRandom(const std::string& name, int x, int y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type_dist(0, 2);
    
    switch(type_dist(gen)) {
        case 0: return create("bear", name, x, y);
        case 1: return create("elf", name, x, y);
        case 2: return create("robber", name, x, y);
        default: return create("elf", name, x, y);
    }
}

std::vector<std::shared_ptr<NPC>> NPCFactory::loadFromFile(const std::string& filename) {
    std::vector<std::shared_ptr<NPC>> npcs;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return npcs;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type, name;
        int x, y, attack, defense;
        
        if (iss >> type >> name >> x >> y >> attack >> defense) {
            if (type == "bear") {
                npcs.push_back(std::make_shared<Bear>(name, x, y, attack, defense));
            } else if (type == "elf") {
                npcs.push_back(std::make_shared<Elf>(name, x, y, attack, defense));
            } else if (type == "robber") {
                npcs.push_back(std::make_shared<Robber>(name, x, y, attack, defense));
            }
        }
    }
    
    std::cout << "Загружено " << npcs.size() << " NPC из файла " << filename << std::endl;
    return npcs;
}

void NPCFactory::saveToFile(const std::string& filename, const std::vector<std::shared_ptr<NPC>>& npcs) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Не удалось создать файл: " << filename << std::endl;
        return;
    }
    
    for (const auto& npc : npcs) {
        file << npc->toString() << "\n";
    }
    
    std::cout << "Сохранено " << npcs.size() << " NPC в файл " << filename << std::endl;
    file.close();
}