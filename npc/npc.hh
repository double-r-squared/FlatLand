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
    enum ConversationState {
        IDLE,      // Not in conversation
        ACTIVE,    // Currently talking
        ENDING     // Conversation is ending (returned to start after completing cycle)
    };
    
    std::shared_ptr<Shape> shape;
    Vec2 velocity;
    
    // Identity
    std::string id;              // Unique identifier (e.g., "guard_001")
    std::string name;            // Display name (e.g., "Town Guard")
    std::string dialogueFile;    // Path to dialogue file
    std::string avatarPath;      // Path to avatar image folder
    
    NPC(std::shared_ptr<Shape> s, Vec2 vel = Vec2(), 
        std::string npcId = "", std::string npcName = "", std::string avatar = "");
    
    void update(float dt);
    
    // High-level conversation interface
    bool canTalk() const;                    // Can player start conversation?
    void startConversation();                // Begin conversation
    bool advanceConversation();              // Continue conversation - returns true if still active
    void endConversation();                  // End conversation and reset
    
    // UI helpers
    std::string getCurrentText() const;      // Get current dialogue text
    std::string getPrompt() const;           // Get interaction prompt ("E - Talk", "E - Continue", etc)
    bool isInConversation() const;           // Check if currently talking
    
    // Dialogue loading (auto-loads if needed)
    void ensureDialogueLoaded();
    bool hasDialogue() const;

private:
    // Internal dialogue state
    ConversationState conversationState;
    std::string currentNodeId;
    int conversationCount;
    
    // Dialogue tree
    std::map<std::string, DialogueNode> dialogueNodes;
    std::string startNodeId;
    
    // Internal dialogue methods
    void loadDialogue(const std::string& filepath);
    void resetDialogue();
    bool isAtConversationEnd() const;
};

#endif // NPC_HH