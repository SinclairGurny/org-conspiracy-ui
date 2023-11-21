#include <regex>
#include <string>
#include <vector>
#include <utility>
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>

#include <raylib-cpp.hpp>

#define BG DARKGRAY
#define NOTE BEIGE
#define TEXT BLACK
#define PIN MAROON
#define LINE DARKBLUE

#define SCALE 5

/*
 [board
 :nid <num>
 :loc <num> <num>
 :anc <tl,(tc),tr,ml,mc,mr,bl,bc,br>
 :con <num> ...
 */

struct Note {
  int id;
  // Position and Size
  int pos_x;
  int pos_y;
  int width;
  int height;
  // Content
  std::string title;
  std::string content;
  // Connections
  std::string anchor;
  std::vector<int> conns;
};

void draw_note(const Note& note_data) {
  int corner_x = note_data.pos_x * SCALE;
  int corner_y = note_data.pos_y * SCALE;
  DrawRectangle(corner_x, corner_y, 100, 100, NOTE);
  DrawText(note_data.title.c_str(), corner_x + 10, corner_y + 10, 10, TEXT);
  DrawText(note_data.content.c_str(), corner_x + 10, corner_y + 30, 6, TEXT);
}

void draw_string(const Note& n1, const Note& n2) {
  int anchor_x1 = (n1.pos_x * SCALE) + 50;
  int anchor_y1 = n1.pos_y * SCALE;
  int anchor_x2 = (n2.pos_x * SCALE) + 50;
  int anchor_y2 = (n2.pos_y * SCALE);
  DrawLine(anchor_x1, anchor_y1, anchor_x2, anchor_y2, LINE);
  DrawCircle(anchor_x1, anchor_y1, 5, PIN);
  DrawCircle(anchor_x2, anchor_y2, 5, PIN);
}

bool is_top_level_section(const std::string& line) {
  if (line.size() > 3 && line[0] == '*' && line[1] == ' ') {
    return true;
  } else {
    return false;
  }
}

std::string get_title(const std::string& line) {
  std::string sub = line.substr(2);
  return sub;
}

bool process_note_info(Note& note, const std::string& line) {
  std::stringstream ss(line);
  std::string tmp;
  int tmps;
  try {
    ss >> tmp;
    assert(tmp == "[board");
    while (ss >> tmp) {
      if (tmp ==  ":nid" ) {
        ss >> tmps;
        note.id = tmps;
      } else if (tmp == ":loc" ) {
        ss >> tmps;
        note.pos_x = tmps;
        ss >> tmps;
        note.pos_y = tmps;
      } else if (tmp == ":anc") {
        ss >> tmp;
        note.anchor = tmp;
      } else if (tmp == ":con" ) {
        while (ss >> tmps) {
          note.conns.push_back(tmps);
        }
      }
    }
    return true;
  } catch (...) {
    return false;
  }
}

std::vector<Note*> read_org_file(const std::string& filename) {
  std::vector<Note*> notes;
  // Open file
  std::ifstream orgfile(filename);
  if (!orgfile.is_open()) {
    std::cerr << "Error opening file -" << filename << std::endl;
    return notes;
  }
  // Process file
  std::string line;
  int mode = -1; // -1 nothing 0 top level header
  while (std::getline(orgfile, line)) {
    if (is_top_level_section(line)) {
      Note* new_note = new Note;
      notes.push_back(new_note);
      new_note->title = get_title(line);
      new_note->pos_x = new_note->pos_y = 0;
      mode = 1;
    } else {
      if (mode == 1) {
        Note* curr_note = notes.back();
        process_note_info(*curr_note, line);
        mode = 2;
      } else if (mode == 2) {
        Note* curr_note = notes.back();
        if (curr_note->content.size() > 0) { curr_note->content += "\n"; }
        curr_note->content += line;
      }
    }
  }
  // Cleanup
  orgfile.close();
  return notes;
}

Note* get_note(std::vector<Note*> notes, int my_id) {
  for (Note* n : notes) {
    if (n->id == my_id) {
      return n;
    }
  }
  return nullptr;
}

void get_connections(const Note& note,
                     std::vector<std::pair<int, int> >& connections) {
  for (int next : note.conns) {
    // Check if it already exists
    bool found = false;
    for (auto cn : connections) {
      if ((note.id == cn.first && next == cn.second)) {// ||
        //          (note.id == cn.second && next == cn.first)) {
        //found = true;
        break;
      }
    }
    if (!found) {
      connections.push_back(std::make_pair(note.id, next));
    }
  }
}

void draw_connection(std::vector<Note*> notes, std::pair<int,int> connection) {
  Note* a = get_note(notes, connection.first);
  Note* b = get_note(notes, connection.second);
  if (a == nullptr || b == nullptr) {
    std::cerr << "Invalid connection "
              << connection.first << ":"
              << connection.second << std::endl;
    return;
  }
  draw_string(*a, *b);
}

void display_notes(std::vector<Note*> notes) {
  std::vector<std::pair<int, int> > connections;
  for (Note* n : notes) {
    draw_note(*n);
    get_connections(*n, connections);
  }
  for (auto con : connections) {
    draw_connection(notes, con);
  }
}

int main(int argc, char** argv) {
  // Get filename
  std::string filename = "test.org";
  if (argc >= 2) {
    filename = argv[1];
  }
  // Initialization
  int screenWidth = 800;
  int screenHeight = 450;

  raylib::Window w(screenWidth, screenHeight, "Org Roaming Conspiracy");

  SetTargetFPS(10);

  // Main game loop
  while (!w.ShouldClose()) {
    // Draw
    BeginDrawing();
    ClearBackground(BG);
    std::vector<Note*> notes = read_org_file(filename);
    display_notes(notes);
    EndDrawing();
  }
  return 0;
}
