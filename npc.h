#pragma once
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cctype>

class FightVisitor;

class NPC {
protected:
    std::string name;
    int x, y;
    int attack_power;
    int defense_power;
    
    void validateCoordinates(int xx, int yy) {
        if (xx <= 0 || xx > 500 || yy <= 0 || yy > 500) {
            throw std::invalid_argument("Координаты должны быть в диапазоне (0, 500]");
        }
    }
    
public:
    NPC(std::string n, int xx, int yy, int attack, int defense) 
        : name(n), x(xx), y(yy), attack_power(attack), defense_power(defense) {
        validateCoordinates(xx, yy);
    }
    
    virtual ~NPC() = default;

    const std::string& getName() const { return name; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getAttack() const { return attack_power; }
    int getDefense() const { return defense_power; }
    
    virtual void move(int dx, int dy) {
        validateCoordinates(x + dx, y + dy);
        x += dx;
        y += dy;
    }
    
    void setPosition(int xx, int yy) {
        validateCoordinates(xx, yy);
        x = xx;
        y = yy;
    }
    
    virtual void increasePower() {
        attack_power += 5;
        defense_power += 5;
    }
    
    virtual std::string type() const = 0;
    virtual void accept(FightVisitor& v) = 0;
    
    virtual std::string toString() const {
        std::string typeLower = type();
        std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        return typeLower + " " + name + " " + std::to_string(x) + " " + 
               std::to_string(y) + " " + std::to_string(attack_power) + " " + 
               std::to_string(defense_power);
    }
};

class Bear: public NPC {
public:
    Bear(std::string n, int xx, int yy, int attack = 30, int defense = 40) 
        : NPC(n, xx, yy, attack, defense) {}
    std::string type() const override;
    void accept(FightVisitor& v) override;
};

class Elf: public NPC {
public:
    Elf(std::string n, int xx, int yy, int attack = 25, int defense = 20) 
        : NPC(n, xx, yy, attack, defense) {}
    std::string type() const override;
    void accept(FightVisitor& v) override;
};

class Robber: public NPC {
public:
    Robber(std::string n, int xx, int yy, int attack = 20, int defense = 15) 
        : NPC(n, xx, yy, attack, defense) {}
    std::string type() const override;
    void accept(FightVisitor& v) override;
};