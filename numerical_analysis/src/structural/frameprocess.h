#pragma once

struct Frame;
struct EquationSet;

// Assign nodes to independent groups
void frame_assign_multicolor(struct Frame* frame);

// Rearrange equations by color group
void eqset_reorder(struct Frame* frame, struct EquationSet* eqset, int** order);