#include "player.hh"
#include <random>
#include <sstream>
#include <algorithm>

Player::Player(const std::string& name)
    : name(name), healingPotions(0), visionPotions(0) {
    
    // Randomly generate HP between 75 and 100
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(75, 100);
    hitPoints = distr(gen);
    maxHitPoints = hitPoints;
}

Player::~Player() {
}

const std::string& Player::getName() const {
    return name;
}

int Player::getHitPoints() const {
    return hitPoints;
}

int Player::getMaxHitPoints() const {
    return maxHitPoints;
}

int Player::getHealingPotions() const {
    return healingPotions;
}

int Player::getVisionPotions() const {
    return visionPotions;
}

const std::vector<std::string>& Player::getPillarsPieces() const {
    return pillarsPieces;
}

void Player::setName(const std::string& newName) {
    name = newName;
}

void Player::takeDamage(int damage) {
    hitPoints -= damage;
    if (hitPoints < 0) {
        hitPoints = 0;
    }
}

void Player::heal(int amount) {
    hitPoints += amount;
    if (hitPoints > maxHitPoints) {
        hitPoints = maxHitPoints;
    }
}

void Player::useHealingPotion() {
    if (healingPotions > 0) {
        healingPotions--;
        heal(25);  // Each potion heals 25 HP
    }
}

void Player::addHealingPotion(int count) {
    healingPotions += count;
}

void Player::addVisionPotion(int count) {
    visionPotions += count;
}

void Player::useVisionPotion() {
    if (visionPotions > 0) {
        visionPotions--;
        // Vision potion effect would be handled elsewhere
    }
}

void Player::addPillarPiece(const std::string& pillarName) {
    // Check if already have this pillar piece
    if (std::find(pillarsPieces.begin(), pillarsPieces.end(), pillarName) 
        == pillarsPieces.end()) {
        pillarsPieces.push_back(pillarName);
    }
}

bool Player::hasPillarPiece(const std::string& pillarName) const {
    return std::find(pillarsPieces.begin(), pillarsPieces.end(), pillarName) 
        != pillarsPieces.end();
}

bool Player::isAlive() const {
    return hitPoints > 0;
}

std::string Player::toString() const {
    std::ostringstream oss;
    
    oss << "=== PLAYER STATS ===\n";
    oss << "Name: " << name << "\n";
    oss << "Hit Points: " << hitPoints << "/" << maxHitPoints << "\n";
    oss << "Healing Potions: " << healingPotions << "\n";
    oss << "Vision Potions: " << visionPotions << "\n";
    oss << "Pillars Found: " << pillarsPieces.size();
    
    if (!pillarsPieces.empty()) {
        oss << "\n  - ";
        for (size_t i = 0; i < pillarsPieces.size(); ++i) {
            oss << pillarsPieces[i];
            if (i < pillarsPieces.size() - 1) {
                oss << "\n  - ";
            }
        }
    } else {
        oss << " (None)";
    }
    oss << "\n";
    
    return oss.str();
}
