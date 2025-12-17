#include "visitor.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

FightVisitor::FightVisitor(std::vector<std::shared_ptr<NPC>>& n, double d)
    : npcs(n), dist(d), stop_requested(false)
{}

FightVisitor::~FightVisitor() {
    stop();
}

void FightVisitor::addObserver(IObserver* o) {
    observers.push_back(o);
}

void FightVisitor::logMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "[FIGHT] " << message << std::endl;
}

double FightVisitor::distance(NPC& a, NPC& b) {
    return hypot(a.getX() - b.getX(), a.getY() - b.getY());
}

std::pair<bool,bool> FightVisitor::fight(NPC& A, NPC& B) {
    std::string t1 = A.type();
    std::string t2 = B.type();
    
    if (t1 == "Elf" && t2 == "Robber") return {true, false};   
    if (t1 == "Robber" && t2 == "Elf") return {false, true};  
    
    if (t1 == "Robber" && t2 == "Robber") return {false, false}; 
    
    if (t1 == "Bear" && t2 == "Elf") return {true, false};  
    if (t1 == "Elf" && t2 == "Bear") return {false, true};  
    
    if (A.getAttack() > B.getDefense() && B.getAttack() > A.getDefense()) {
        return {true, true};  
    } else if (A.getAttack() > B.getDefense()) {
        return {true, false};  
    } else if (B.getAttack() > A.getDefense()) {
        return {false, true};  
    } else {
        return {false, false}; 
    }
}

void FightVisitor::processSingleFight(std::shared_ptr<NPC> a, std::shared_ptr<NPC> b) {
    auto [aWin, bWin] = fight(*a, *b);
    std::string Aname = a->getName();
    std::string Bname = b->getName();
    
    logMessage("Бой между " + Aname + " [" + a->type() + "] и " + Bname + " [" + b->type() + "]");
    
    if (aWin && !bWin) {
        a->increasePower();
        for(auto o: observers) o->onKill(Aname, Bname);
        
        std::unique_lock lock(npcs_mutex);
        auto it = std::find(npcs.begin(), npcs.end(), b);
        if (it != npcs.end()) {
            npcs.erase(it);
            logMessage(Aname + " убил " + Bname + " и стал сильнее!");
        }
    }
    else if (!aWin && bWin) {
        b->increasePower();
        for(auto o: observers) o->onKill(Bname, Aname);
        
        std::unique_lock lock(npcs_mutex);
        auto it = std::find(npcs.begin(), npcs.end(), a);
        if (it != npcs.end()) {
            npcs.erase(it);
            logMessage(Bname + " убил " + Aname + " и стал сильнее!");
        }
    }
    else if (!aWin && !bWin) {
        for(auto o: observers) o->onDoubleDeath(Aname, Bname);
        
        std::unique_lock lock(npcs_mutex);
        auto it_a = std::find(npcs.begin(), npcs.end(), a);
        auto it_b = std::find(npcs.begin(), npcs.end(), b);
        
        if (it_a != npcs.end() && it_b != npcs.end()) {
            if (std::distance(npcs.begin(), it_a) > std::distance(npcs.begin(), it_b)) {
                npcs.erase(it_a);
                npcs.erase(it_b);
            } else {
                npcs.erase(it_b);
                npcs.erase(it_a);
            }
            logMessage(Aname + " и " + Bname + " погибли вместе!");
        }
    }
    else {
        logMessage(Aname + " и " + Bname + " не смогли убить друг друга");
    }
}

void FightVisitor::detectFights() {
    logMessage("Запущен поток обнаружения боев");
    
    while (!stop_requested) {
        std::vector<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> local_fights;
        
        {
            std::shared_lock lock(npcs_mutex);
            
            for (size_t i = 0; i < npcs.size(); ++i) {
                for (size_t j = i + 1; j < npcs.size(); ++j) {
                    if (distance(*npcs[i], *npcs[j]) <= dist) {
                        local_fights.emplace_back(npcs[i], npcs[j]);
                    }
                }
            }
        }
        
        if (!local_fights.empty()) {
            std::lock_guard lock(queue_mutex);
            for (auto& fight : local_fights) {
                fight_queue.push(fight);
            }
            queue_cv.notify_one();
            logMessage("Обнаружено " + std::to_string(local_fights.size()) + " потенциальных боев");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    logMessage("Поток обнаружения боев остановлен");
}

void FightVisitor::processFights() {
    logMessage("Запущен поток обработки боев");
    
    while (!stop_requested) {
        std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>> fight_pair;
        
        {
            std::unique_lock lock(queue_mutex);
            queue_cv.wait(lock, [this]() {
                return !fight_queue.empty() || stop_requested;
            });
            
            if (stop_requested) break;
            
            if (!fight_queue.empty()) {
                fight_pair = fight_queue.front();
                fight_queue.pop();
            }
        }
        
        if (fight_pair.first && fight_pair.second) {
            processSingleFight(fight_pair.first, fight_pair.second);
        }
    }
    
    logMessage("Поток обработки боев остановлен");
}

void FightVisitor::run() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        std::shared_lock lock(npcs_mutex);
        std::vector<std::shared_ptr<NPC>> npcs_copy = npcs;
        lock.unlock();
        
        for (size_t i = 0; i < npcs_copy.size(); ++i) {
            for (size_t j = i + 1; j < npcs_copy.size(); ++j) {
                if (distance(*npcs_copy[i], *npcs_copy[j]) <= dist) {
                    processSingleFight(npcs_copy[i], npcs_copy[j]);
                    changed = true;
                    goto nextRound;
                }
            }
        }
        nextRound:;
    }
}

void FightVisitor::runAsync() {
    std::thread detector([this]() { this->detectFights(); });
    std::thread processor([this]() { this->processFights(); });
    
    detector.detach();
    processor.detach();
}

void FightVisitor::stop() {
    stop_requested = true;
    queue_cv.notify_all();
}

void FightVisitor::visit(Bear& b) {}
void FightVisitor::visit(Elf& e) {}
void FightVisitor::visit(Robber& r) {}