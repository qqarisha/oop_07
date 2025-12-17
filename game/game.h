#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <string>

class NPC;

class Game {
private:
    std::vector<std::shared_ptr<NPC>> npcs;
    std::vector<std::shared_ptr<NPC>> dead_npcs;
    
    std::thread movement_thread;
    std::thread fight_thread;
    
    mutable std::shared_mutex npcs_mutex;
    mutable std::mutex cout_mutex;
    
    std::queue<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> fight_queue;
    std::mutex fight_mutex;
    std::condition_variable fight_cv;
    
    std::atomic<bool> running;
    std::atomic<bool> stop_requested;
    
    static constexpr int MAP_WIDTH = 100;
    static constexpr int MAP_HEIGHT = 100;
    static constexpr int ELF_MOVE_DISTANCE = 10;  
    static constexpr int KILL_DISTANCE = 1;   
    static constexpr int GAME_DURATION = 30;
    
    void initializeNPCs();
    void movementWorker();
    void fightWorker();
    void printMap();
    void processFight(std::shared_ptr<NPC> attacker, std::shared_ptr<NPC> defender);
    
public:
    Game();
    ~Game();
    
    void run();
    void stop();
    std::vector<std::shared_ptr<NPC>> getSurvivors() const;
    int getAliveCount() const;
    int getDeadCount() const;
};