#include "game.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include "../npc.h"
#include "../factory/factory.h"

Game::Game() : running(false), stop_requested(false) {
    initializeNPCs();
}

Game::~Game() {
    stop();
}

void Game::initializeNPCs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> coord(1, MAP_WIDTH);
    
    std::unique_lock lock(npcs_mutex);
    
    for (int i = 0; i < 50; ++i) {
        int x = coord(gen);
        int y = coord(gen);
        std::string name = "NPC_" + std::to_string(i + 1);
        
        auto npc = NPCFactory::createRandom(name, x, y);
        if (npc) {
            npcs.push_back(npc);
        }
    }
    
    std::cout << "Создано " << npcs.size() << " NPC" << std::endl;
    std::cout << "Эльфы двигаются на " << ELF_MOVE_DISTANCE << " клеток" << std::endl;
    std::cout << "Дистанция боя " << KILL_DISTANCE << " клеток" << std::endl;
}

void Game::movementWorker() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> move_dir(-1, 1);
    std::uniform_int_distribution<> move_chance(0, 100);
    
    {
        std::lock_guard lock(cout_mutex);
        std::cout << "Запущен поток движения" << std::endl;
    }
    
    while (running && !stop_requested) {
        std::vector<std::shared_ptr<NPC>> npcs_copy;
        
        {
            std::shared_lock lock(npcs_mutex);
            npcs_copy = npcs;
        }
        
        for (auto& npc : npcs_copy) {
            {
                std::shared_lock lock(npcs_mutex);
                auto it = std::find(npcs.begin(), npcs.end(), npc);
                if (it == npcs.end()) continue; 
            }
            
            bool should_move = false;
            int move_distance = 0;
            
            if (npc->type() == "Elf") {
                should_move = true;
                move_distance = ELF_MOVE_DISTANCE;
            } else if (move_chance(gen) < 30) {
                should_move = true;
                move_distance = 5;
            }
            
            if (should_move) {
                int dx = move_dir(gen) * move_distance;
                int dy = move_dir(gen) * move_distance;
                
                int new_x = npc->getX() + dx;
                int new_y = npc->getY() + dy;
                
                if (new_x >= 1 && new_x <= MAP_WIDTH && 
                    new_y >= 1 && new_y <= MAP_HEIGHT) {
                    
                    std::unique_lock lock(npcs_mutex);
                    npc->move(dx, dy);
                }
            }
        }
        
        {
            std::shared_lock lock(npcs_mutex);
            for (size_t i = 0; i < npcs.size(); ++i) {
                for (size_t j = i + 1; j < npcs.size(); ++j) {
                    double dist = std::hypot(
                        npcs[i]->getX() - npcs[j]->getX(),
                        npcs[i]->getY() - npcs[j]->getY()
                    );
                    
                    if (dist <= KILL_DISTANCE) {
                        std::lock_guard fight_lock(fight_mutex);
                        fight_queue.push({npcs[i], npcs[j]});
                        fight_cv.notify_one();
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    {
        std::lock_guard lock(cout_mutex);
        std::cout << "Остановлен поток движения" << std::endl;
    }
}

void Game::processFight(std::shared_ptr<NPC> attacker, std::shared_ptr<NPC> defender) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dice(1, 6); 
    
    int attack_power = dice(gen); 
    int defense_power = dice(gen);
    
    {
        std::lock_guard lock(cout_mutex);
        std::cout << attacker->getName() << " атакует " << defender->getName() 
                  << " (Атака: " << attack_power << " vs Защита: " << defense_power << ")" << std::endl;
    }
    
    if (attack_power > defense_power) {
        std::unique_lock lock(npcs_mutex);
        
        auto it_att = std::find(npcs.begin(), npcs.end(), attacker);
        auto it_def = std::find(npcs.begin(), npcs.end(), defender);
        
        if (it_att != npcs.end() && it_def != npcs.end()) {
            dead_npcs.push_back(defender);
            npcs.erase(it_def);
            
            {
                std::lock_guard cout_lock(cout_mutex);
                std::cout << attacker->getName() << " убил " << defender->getName() << std::endl;
            }
        }
    } else {
        std::lock_guard lock(cout_mutex);
        std::cout << defender->getName() << " защитился от " << attacker->getName() << std::endl;
    }
}

void Game::fightWorker() {
    {
        std::lock_guard lock(cout_mutex);
        std::cout << "Запущен поток боев" << std::endl;
    }
    
    while (running && !stop_requested) {
        std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>> fight_pair;
        
        {
            std::unique_lock lock(fight_mutex);
            fight_cv.wait(lock, [this]() {
                return !fight_queue.empty() || stop_requested;
            });
            
            if (stop_requested) break;
            
            if (!fight_queue.empty()) {
                fight_pair = fight_queue.front();
                fight_queue.pop();
            }
        }
        
        if (fight_pair.first && fight_pair.second) {
            processFight(fight_pair.first, fight_pair.second);
        }
    }
    
    {
        std::lock_guard lock(cout_mutex);
        std::cout << "Остановлен поток боев" << std::endl;
    }
}

void Game::printMap() {
    std::lock_guard lock(cout_mutex);
    
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    
    std::cout << "ЛАБОРАТОРНАЯ 7" << std::endl;
    std::cout << "Вариант: Эльф (ход=" << ELF_MOVE_DISTANCE << ", бой=" << KILL_DISTANCE << ")" << std::endl;
    
    std::shared_lock lock2(npcs_mutex);
    int alive = npcs.size();
    int dead = dead_npcs.size();
    
    std::cout << "Живые: " << alive << "  Мертвые: " << dead << std::endl;
    std::cout << "Карта: " << MAP_WIDTH << "x" << MAP_HEIGHT << std::endl;
    
    int bears = 0, elves = 0, robbers = 0;
    for (const auto& npc : npcs) {
        if (npc->type() == "Bear") bears++;
        else if (npc->type() == "Elf") elves++;
        else if (npc->type() == "Robber") robbers++;
    }
    
    std::cout << "Медведи: " << bears << " Эльфы: " << elves << " Разбойники: " << robbers << std::endl;
    
    const int grid_size = 10;
    char grid[grid_size][grid_size];
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            grid[i][j] = '.';
        }
    }
    
    for (const auto& npc : npcs) {
        int grid_x = (npc->getX() * grid_size) / MAP_WIDTH;
        int grid_y = (npc->getY() * grid_size) / MAP_HEIGHT;
        
        grid_x = std::clamp(grid_x, 0, grid_size - 1);
        grid_y = std::clamp(grid_y, 0, grid_size - 1);
        
        char symbol = '?';
        if (npc->type() == "Bear") symbol = 'B';
        else if (npc->type() == "Elf") symbol = 'E';
        else if (npc->type() == "Robber") symbol = 'R';
        
        grid[grid_y][grid_x] = symbol;
    }
    
    std::cout << "\nКарта (10x10):" << std::endl;
    std::cout << "  ";
    for (int i = 0; i < grid_size; ++i) std::cout << i << " ";
    std::cout << std::endl;
    
    for (int y = 0; y < grid_size; ++y) {
        std::cout << y << " ";
        for (int x = 0; x < grid_size; ++x) {
            std::cout << grid[y][x] << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nB-Медведь E-Эльф R-Разбойник" << std::endl;
}

void Game::run() {
    running = true;
    
    {
        std::lock_guard lock(cout_mutex);
        std::cout << "\nНачало игры" << std::endl;
        std::cout << "Длительность: " << GAME_DURATION << " секунд" << std::endl;
    }
    
    movement_thread = std::thread(&Game::movementWorker, this);
    fight_thread = std::thread(&Game::fightWorker, this);
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < GAME_DURATION && running; ++i) {
        printMap();
        
        {
            std::lock_guard lock(cout_mutex);
            std::cout << "\nОсталось: " << (GAME_DURATION - i) << " секунд" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    stop();
    
    {
        std::shared_lock lock(npcs_mutex);
        std::lock_guard cout_lock(cout_mutex);
        
        std::cout << "\nИГРА ЗАВЕРШЕНА" << std::endl;
        std::cout << "Выжившие: " << npcs.size() << std::endl;
        std::cout << "Погибшие: " << dead_npcs.size() << std::endl;
        
        for (const auto& npc : npcs) {
            std::cout << npc->getName() << " (" << npc->type() << ") "
                      << "[" << npc->getX() << "," << npc->getY() << "]" << std::endl;
        }
    }
}

void Game::stop() {
    stop_requested = true;
    fight_cv.notify_all();
    
    if (movement_thread.joinable()) movement_thread.join();
    if (fight_thread.joinable()) fight_thread.join();
    
    running = false;
}

std::vector<std::shared_ptr<NPC>> Game::getSurvivors() const {
    std::shared_lock lock(npcs_mutex);
    return npcs;
}

int Game::getAliveCount() const {
    std::shared_lock lock(npcs_mutex);
    return npcs.size();
}

int Game::getDeadCount() const {
    return dead_npcs.size();
}