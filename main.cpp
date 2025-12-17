#include <iostream>
#include "game/game.h"

int main() {
    try {
        std::cout << "ЛАБОРАТОРНАЯ РАБОТА №7" << std::endl;
        std::cout << "Вариант: Эльф (10/50)" << std::endl;
        
        Game game;
        
        game.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}