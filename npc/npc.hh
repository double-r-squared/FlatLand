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
    std::string id;              // Unique identifier from map file
                                 // ALSO used as:
                                 // - display name in UI
                                 // - key for avatar & dialogue files
                                 // Example: "guard1", "caarl", "wizard"

    NPC(std::shared_ptr<Shape> s,
        Vec2 vel = Vec2(),
        std::string npcId = "Unnamed");

    void update(float dt);

    // ────────────────────────────────────────────────
    //          High-level conversation interface
    // ────────────────────────────────────────────────
    bool canTalk() const;                    // Can player start conversation?
    void startConversation();                // Begin conversation
    bool advanceConversation();              // Continue - returns true if still active
    void endConversation();                  // End and reset

    // ────────────────────────────────────────────────
    //                UI / View helpers
    // ────────────────────────────────────────────────
    std::string getCurrentText() const;      // Current dialogue line
    std::string getPrompt() const;           // "E - Talk", "E - Continue", etc.
    bool isInConversation() const;

    // ────────────────────────────────────────────────
    //           Derived asset paths (based on id)
    // ────────────────────────────────────────────────
    std::string getAvatarPath() const;
    std::string getDialoguePath() const;

    bool hasDialogue() const;
    bool hasAvatar() const;                  // Optional: check if file likely exists

    // ────────────────────────────────────────────────
    //               Internal state
    // ────────────────────────────────────────────────
private:
    ConversationState conversationState = IDLE;
    std::string currentNodeId;
    int conversationCount = 0;

    std::map<std::string, DialogueNode> dialogueNodes;
    std::string startNodeId = "start";

    // Dialogue loading & management
    void ensureDialogueLoaded();
    void loadDialogue(const std::string& filepath);
    void resetDialogue();
    bool isAtConversationEnd() const;
};

#endif // NPC_HH