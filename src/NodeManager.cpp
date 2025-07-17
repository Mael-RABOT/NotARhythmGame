#include "NodeManager.hpp"

namespace App {
namespace Core {

    NodeManager::NodeManager() : nextId(1) {}

    int NodeManager::addNote(int lane, double timestamp) {
        notes.push_back(Note{nextId, static_cast<Lane>(lane), timestamp});
        return nextId++;
    }

    int NodeManager::addNoteWithId(int id, int lane, double timestamp) {
        notes.push_back(Note{id, static_cast<Lane>(lane), timestamp});
        if (id >= nextId) {
            nextId = id + 1;
        }
        return id;
    }

    void NodeManager::removeNote(int id) {
        notes.erase(std::remove_if(notes.begin(), notes.end(), [id](const Note& n){ return n.id == id; }), notes.end());
    }

    void NodeManager::moveNote(int id, int newLane, double newTimestamp) {
        for (auto& n : notes) {
            if (n.id == id) {
                n.lane = static_cast<Lane>(newLane);
                n.timestamp = newTimestamp;
                break;
            }
        }
    }

    Note* NodeManager::getNoteById(int id) {
        for (auto& n : notes) if (n.id == id) return &n;
        return nullptr;
    }

    std::vector<Note>& NodeManager::getNotes() { return notes; }
    void NodeManager::clear() { notes.clear(); nextId = 1; }
    int NodeManager::getNextId() const { return nextId; }

} // Core
} // App
