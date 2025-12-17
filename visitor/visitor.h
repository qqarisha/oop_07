#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "../npc.h"
#include "../observer/observer.h"

class FightVisitor {
private:
    std::vector<std::shared_ptr<NPC>>& npcs;
    std::vector<IObserver*> observers;
    double dist;
    
    std::queue<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> fight_queue;
    mutable std::mutex queue_mutex;
    mutable std::shared_mutex npcs_mutex;
    mutable std::mutex cout_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> stop_requested;
    
public:
    FightVisitor(std::vector<std::shared_ptr<NPC>>& n, double d);
    ~FightVisitor();
    
    void addObserver(IObserver* o);
    
    void detectFights();
    void processFights();
    
    void visit(Bear& b);
    void visit(Elf& e);
    void visit(Robber& r);
    
    void run();  
    void runAsync();  
    void stop();
    
private:
    std::pair<bool, bool> fight(NPC& a, NPC& b);
    static double distance(NPC& a, NPC& b);
    void processSingleFight(std::shared_ptr<NPC> a, std::shared_ptr<NPC> b);
    void logMessage(const std::string& message);
};