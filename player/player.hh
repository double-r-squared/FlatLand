#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>
#include <vector>
#include "../Vec2.hh"

class Player {
private:
    std::string name;
    int hitPoints;
    int maxHitPoints;
    int healingPotions;
    int visionPotions;
    std::vector<std::string> pillarsPieces;  // e.g., "Pillar 1", "Pillar 2", etc.
    
public:
    Player(const std::string& name);
    ~Player();
    
    // Getters
    const std::string& getName() const;
    int getHitPoints() const;
    int getMaxHitPoints() const;
    int getHealingPotions() const;
    int getVisionPotions() const;
    const std::vector<std::string>& getPillarsPieces() const;
    
    // Setters
    void setName(const std::string& name);
    
    // HP management
    void takeDamage(int damage);
    void heal(int amount);
    void useHealingPotion();
    
    // Potions
    void addHealingPotion(int count = 1);
    void addVisionPotion(int count = 1);
    void useVisionPotion();
    
    // Pillars
    void addPillarPiece(const std::string& pillarName);
    bool hasPillarPiece(const std::string& pillarName) const;
    
    // String representation
    std::string toString() const;
    
    // Check if player is alive
    bool isAlive() const;
};

#endif // PLAYER_HH
