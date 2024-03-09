#pragma once

struct Frame;

// Load a frame from a ".frame" file
int frame_import(const char* path, struct Frame* frame);

// Constructs a frame with hardcoded values for testing
// generally best to import from a file instead
void frame_create_sample(struct Frame* frame);