#ifndef NPC_HH
#define NPC_HH

#include "Shape.hh"
#include "../Vec2.hh"
#include <memory>
#include <string>
#include <vector>
#include <map>

// Simple dialogue node structure
struct DialogueNode {
    std::string id;
    std::string text;
    std::vector<std::string> nextNodeIds;  // Simple linear progression for now
};

class NPC {
public:
    std::shared_ptr<Shape> shape;
    Vec2 velocity;
    
    // Identity
    std::string id;              // Unique identifier (e.g., "guard_001")
    std::string name;            // Display name (e.g., "Town Guard")
    std::string dialogueFile;    // Path to dialogue JSON (e.g., "dialogues/guard.json")
    
    // Dialogue state
    std::string currentNodeId;   // Which dialogue node we're currently at
    int conversationCount;       // How many times player has talked to this NPC
    
    // Dialogue tree (loaded from file)
    std::map<std::string, DialogueNode> dialogueNodes;
    std::string startNodeId;     // Where conversations begin
    
    NPC(std::shared_ptr<Shape> s, Vec2 vel = Vec2(), 
        std::string npcId = "", std::string npcName = "");
    
    void update(float dt);
    
    // Dialogue methods
    void loadDialogue(const std::string& filepath);
    std::string getCurrentDialogueText() const;
    void advanceDialogue();  // Move to next dialogue node
    void resetDialogue();    // Reset to start
    bool hasDialogue() const;
};

#endif // NPC_HH