#pragma once

#include <vector>
#include <algorithm>

namespace App {
namespace Core {

    enum Lane {
        TOP = 0,
        BOTTOM = 1,
    };

    struct Note {
        int id;
        Lane lane;
        double timestamp;
    };

    class NodeManager {
    public:
        NodeManager();
        int addNote(int lane, double timestamp);
        void removeNote(int id);
        void moveNote(int id, int newLane, double newTimestamp);
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
