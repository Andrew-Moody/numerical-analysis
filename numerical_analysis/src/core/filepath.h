#pragma once

// Returns a string containing the full filepath given a filepath relative to the repository
// prefix can used to add an optional path between the repository root and filename i.e. models/
char* get_full_filepath(const char* filename, const char* prefix);