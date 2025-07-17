#pragma once

#include <vector>
#include <algorithm>

namespace App {
namespace Core {

    enum Lane {
        TOP = 0,
        BOTTOM = 1,
    };

    enum NoteType {
        TAP = 0,
        HOLD = 1,
    };

    struct Note {
        int id;
        Lane lane;
        NoteType type;
        double timestamp;        // Start time for both TAP and HOLD
        double endTimestamp;     // End time for HOLD notes (same as timestamp for TAP)
    };

    class NodeManager {
    public:
        NodeManager();
        int addNote(int lane, double timestamp);
        int addNoteWithId(int id, int lane, double timestamp);
        int addHoldNote(int lane, double startTimestamp, double endTimestamp);
        int addHoldNoteWithId(int id, int lane, double startTimestamp, double endTimestamp);
        void removeNote(int id);
        void moveNote(int id, int newLane, double newTimestamp);
        void moveHoldNote(int id, int newLane, double newStartTimestamp, double newEndTimestamp);
        Note* getNoteById(int id);
        std::vector<Note>& getNotes();
        void clear();
        int getNextId() const;

    private:
        std::vector<Note> notes;
        int nextId;
    };

} // namespace Core
} // namespace App
