#include "npc.hh"
#include <fstream>
#include <iostream>
#include <sstream>

NPC::NPC(std::shared_ptr<Shape> s, Vec2 vel, std::string npcId, std::string npcName) 
    : shape(s), velocity(vel), id(npcId), name(npcName), 
      currentNodeId(""), conversationCount(0), startNodeId("start") {
    
    // If no ID provided, generate a default one
    std::cout << "WARNING: NO ID PROVIDED";
    if (id.empty()) {
        static int npcCounter = 0;
        id = "npc_" + std::to_string(npcCounter++);
    }
    
    // If no name provided, use ID as name
    if (name.empty()) {
        name = id;
    }
}

void NPC::update(float dt) {
    shape->position = shape->position + velocity * dt;
}
void NPC::loadDialogue(const std::string& filepath) {
    dialogueFile = filepath;
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
                // If we were already in a node, save it first
                if (inNode && !currentNode.id.empty()) {
                    dialogueNodes[currentNode.id] = currentNode;
                }
                
                currentNode.id = value;
                currentNode.text = "";
                currentNode.nextNodeIds.clear();
                inNode = true;
                
                // Track first node if start node not specified
                if (firstNodeId.empty()) {
                    firstNodeId = value;
                }
            } else if (key == "TEXT") {
                currentNode.text = value;
            } else if (key == "NEXT") {
                // Split multiple NEXT options (they might be on separate lines)
                std::istringstream iss(value);
                std::string nextNode;
                while (std::getline(iss, nextNode, ' ')) {
                    if (!nextNode.empty()) {
                        currentNode.nextNodeIds.push_back(nextNode);
                    }
                }
                
                // Also handle case where NEXT is on its own line without values
                // (next values will come on subsequent lines)
            } else if (key == "OPTION") {
                // For future: handle multiple choice options
                // For now, just add to nextNodeIds
                size_t spacePos = value.find(' ');
                if (spacePos != std::string::npos) {
                    std::string nextId = value.substr(0, spacePos);
                    currentNode.nextNodeIds.push_back(nextId);
                } else {
                    currentNode.nextNodeIds.push_back(value);
                }
            }
        } else {
            // Line without colon - check if it's a continuation of NEXT values
            if (!line.empty() && inNode && !currentNode.nextNodeIds.empty()) {
                // This might be additional NEXT values on new lines
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
    
    // Don't forget the last node
    if (inNode && !currentNode.id.empty()) {
        dialogueNodes[currentNode.id] = currentNode;
    }
    
    // If no START was specified, use the first node
    if (startNodeId.empty() || dialogueNodes.find(startNodeId) == dialogueNodes.end()) {
        if (!firstNodeId.empty()) {
            startNodeId = firstNodeId;
            std::cout << "DEBUG: Using first node as start: " << startNodeId << std::endl;
        }
    }
    
    currentNodeId = startNodeId;
    
    std::cout << "Loaded " << dialogueNodes.size() << " dialogue nodes for " << name << std::endl;
    std::cout << "Start node: " << startNodeId << std::endl;
    
    // DEBUG: Print all nodes
    for (const auto& [id, node] : dialogueNodes) {
        std::cout << "  Node '" << id << "': " << node.text << std::endl;
        std::cout << "    Next: ";
        for (const auto& next : node.nextNodeIds) {
            std::cout << next << " ";
        }
        std::cout << std::endl;
    }
}

std::string NPC::getCurrentDialogueText() const {
    if (currentNodeId.empty() || dialogueNodes.find(currentNodeId) == dialogueNodes.end()) {
        return "...";  // Default if no dialogue
    }
    
    return dialogueNodes.at(currentNodeId).text;
}

void NPC::advanceDialogue() {
    if (currentNodeId.empty() || dialogueNodes.find(currentNodeId) == dialogueNodes.end()) {
        return;
    }
    
    const auto& node = dialogueNodes.at(currentNodeId);
    
    // Move to next node if available
    if (!node.nextNodeIds.empty()) {
        currentNodeId = node.nextNodeIds[0];  // Just take first option for now
    } else {
        // End of dialogue, loop back to start
        conversationCount++;
        currentNodeId = startNodeId;
    }
}

void NPC::resetDialogue() {
    currentNodeId = startNodeId;
}

bool NPC::hasDialogue() const {
    return !dialogueNodes.empty();
}