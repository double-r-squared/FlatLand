#include "npc.hh"
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>  // for stat() in hasAvatar()

NPC::NPC(std::shared_ptr<Shape> s, Vec2 vel, std::string npcId)
    : shape(s), velocity(vel), id(std::move(npcId)),
      conversationState(IDLE), currentNodeId(""), conversationCount(0), startNodeId("start")
{
    // If no ID provided, generate a fallback
    if (id.empty()) {
        std::cout << "WARNING: NO ID PROVIDED FOR NPC" << std::endl;
        static int npcCounter = 0;
        id = "npc_" + std::to_string(npcCounter++);
    }
}

void NPC::update(float dt) {
    shape->position = shape->position + velocity * dt;
}

// ============================================================================
// HIGH-LEVEL CONVERSATION INTERFACE
// ============================================================================

bool NPC::canTalk() const {
    return conversationState == IDLE;
}

void NPC::startConversation() {
    ensureDialogueLoaded();
    conversationState = ACTIVE;
    resetDialogue();
    
    std::cout << "Started conversation with " << id << std::endl;
}

bool NPC::advanceConversation() {
    if (conversationState != ACTIVE) {
        return false;
    }
    
    if (currentNodeId.empty() || dialogueNodes.find(currentNodeId) == dialogueNodes.end()) {
        conversationState = ENDING;
        return false;
    }
    
    const auto& node = dialogueNodes.at(currentNodeId);
    
    // Move to next node if available
    if (!node.nextNodeIds.empty()) {
        currentNodeId = node.nextNodeIds[0];  // Take first option for now
    } else {
        // End of dialogue tree, loop back to start
        conversationCount++;
        currentNodeId = startNodeId;
        
        // Check if we should end conversation (completed at least one cycle)
        if (isAtConversationEnd()) {
            conversationState = ENDING;
            return false;
        }
    }
    
    return true;  // Conversation continues
}

void NPC::endConversation() {
    conversationState = IDLE;
    resetDialogue();
    std::cout << "Ended conversation with " << id << std::endl;
}

// ============================================================================
// UI HELPERS
// ============================================================================

std::string NPC::getCurrentText() const {
    if (currentNodeId.empty() || dialogueNodes.find(currentNodeId) == dialogueNodes.end()) {
        if (hasDialogue()) {
            return "...";  // Dialogue exists but not loaded properly
        } else {
            return "Hello, I'm " + id + ".";  // Default greeting using id
        }
    }
    
    return dialogueNodes.at(currentNodeId).text;
}

std::string NPC::getPrompt() const {
    if (conversationState == IDLE) {
        return "E - Talk";
    } else if (conversationState == ACTIVE) {
        return "E - Continue";
    }
    return "";
}

bool NPC::isInConversation() const {
    return conversationState == ACTIVE;
}

// ============================================================================
// DERIVED PATHS
// ============================================================================

std::string NPC::getAvatarPath() const {
    if (id.empty()) {
        return "assets/npcs/default.png";
    }
    return "assets/npcs/" + id + ".png";
}

std::string NPC::getDialoguePath() const {
    if (id.empty()) {
        return "";
    }
    return "dialogues/" + id + ".txt";  // change to .dialogue if that's your extension
}

bool NPC::hasAvatar() const {
    struct stat buffer;
    return stat(getAvatarPath().c_str(), &buffer) == 0;
}

// ============================================================================
// DIALOGUE LOADING AND MANAGEMENT
// ============================================================================

void NPC::ensureDialogueLoaded() {
    // Only load if we haven't loaded yet
    if (dialogueNodes.empty()) {
        std::string path = getDialoguePath();
        if (!path.empty()) {
            loadDialogue(path);
        }
    }
}

bool NPC::hasDialogue() const {
    // Can be called before loading — triggers lazy load check
    if (dialogueNodes.empty()) {
        const_cast<NPC*>(this)->ensureDialogueLoaded();
    }
    return !dialogueNodes.empty();
}

void NPC::loadDialogue(const std::string& filepath) {
    dialogueNodes.clear();
    startNodeId = "start"; // Default
    
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "Warning: Could not load dialogue file: " << filepath << std::endl;
        return;
    }
    
    std::string line;
    DialogueNode currentNode;
    bool inNode = false;
    std::string firstNodeId; // Track the first node we encounter
    
    while (std::getline(file, line)) {
        // Remove carriage return for Windows files
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Skip empty lines (they separate nodes)
        if (line.empty()) {
            if (inNode && !currentNode.id.empty()) {
                dialogueNodes[currentNode.id] = currentNode;
                currentNode = DialogueNode();
                inNode = false;
            }
            continue;
        }
        
        // Parse line
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            if (key == "START") {
                startNodeId = value;
                std::cout << "DEBUG: Found START node: " << startNodeId << std::endl;
            } else if (key == "NODE") {
                // Save previous node if any
                if (inNode && !currentNode.id.empty()) {
                    dialogueNodes[currentNode.id] = currentNode;
                }
                
                currentNode.id = value;
                currentNode.text = "";
                currentNode.nextNodeIds.clear();
                inNode = true;
                
                if (firstNodeId.empty()) {
                    firstNodeId = value;
                }
            } else if (key == "TEXT") {
                currentNode.text = value;
            } else if (key == "NEXT") {
                std::istringstream iss(value);
                std::string nextNode;
                while (std::getline(iss, nextNode, ' ')) {
                    if (!nextNode.empty()) {
                        currentNode.nextNodeIds.push_back(nextNode);
                    }
                }
            } else if (key == "OPTION") {
                // For future: handle multiple choice
                size_t spacePos = value.find(' ');
                if (spacePos != std::string::npos) {
                    std::string nextId = value.substr(0, spacePos);
                    currentNode.nextNodeIds.push_back(nextId);
                } else {
                    currentNode.nextNodeIds.push_back(value);
                }
            }
        } else {
            // Continuation line (e.g. more NEXT values)
            if (!line.empty() && inNode && !currentNode.nextNodeIds.empty()) {
                std::istringstream iss(line);
                std::string nextNode;
                while (iss >> nextNode) {
                    if (!nextNode.empty()) {
                        currentNode.nextNodeIds.push_back(nextNode);
                    }
                }
            }
        }
    }
    
    // Save last node
    if (inNode && !currentNode.id.empty()) {
        dialogueNodes[currentNode.id] = currentNode;
    }
    
    // Fallback start node
    if (startNodeId.empty() || dialogueNodes.find(startNodeId) == dialogueNodes.end()) {
        if (!firstNodeId.empty()) {
            startNodeId = firstNodeId;
            std::cout << "DEBUG: Using first node as start: " << startNodeId << std::endl;
        }
    }
    
    currentNodeId = startNodeId;
    
    std::cout << "Loaded " << dialogueNodes.size() << " dialogue nodes for " << id << std::endl;
    std::cout << "Start node: " << startNodeId << std::endl;
    
    // DEBUG print
    for (const auto& [nodeId, node] : dialogueNodes) {
        std::cout << "  Node '" << nodeId << "': " << node.text << std::endl;
        std::cout << "    Next: ";
        for (const auto& next : node.nextNodeIds) {
            std::cout << next << " ";
        }
        std::cout << std::endl;
    }
}

void NPC::resetDialogue() {
    currentNodeId = startNodeId;
}

bool NPC::isAtConversationEnd() const {
    return (conversationCount > 0 && currentNodeId == startNodeId);
}