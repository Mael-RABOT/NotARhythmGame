#include "NodeManager.hpp"

namespace App {
namespace Core {

    NodeManager::NodeManager() : nextId(1) {}

    int NodeManager::addNote(int lane, double timestamp) {
        notes.push_back(Note{nextId, static_cast<Lane>(lane), TAP, timestamp, timestamp});
        return nextId++;
    }

    int NodeManager::addNoteWithId(int id, int lane, double timestamp) {
        notes.push_back(Note{id, static_cast<Lane>(lane), TAP, timestamp, timestamp});
        if (id >= nextId) {
            nextId = id + 1;
        }
        return id;
    }

    int NodeManager::addHoldNote(int lane, double startTimestamp, double endTimestamp) {
        notes.push_back(Note{nextId, static_cast<Lane>(lane), HOLD, startTimestamp, endTimestamp});
        return nextId++;
    }

    int NodeManager::addHoldNoteWithId(int id, int lane, double startTimestamp, double endTimestamp) {
        notes.push_back(Note{id, static_cast<Lane>(lane), HOLD, startTimestamp, endTimestamp});
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
                if (n.type == TAP) {
                    n.endTimestamp = newTimestamp;
                }
                break;
            }
        }
    }

    void NodeManager::moveHoldNote(int id, int newLane, double newStartTimestamp, double newEndTimestamp) {
        for (auto& n : notes) {
            if (n.id == id) {
                n.lane = static_cast<Lane>(newLane);
                n.timestamp = newStartTimestamp;
                n.endTimestamp = newEndTimestamp;
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
