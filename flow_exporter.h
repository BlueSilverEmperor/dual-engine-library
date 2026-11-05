#ifndef FLOW_EXPORTER_H
#define FLOW_EXPORTER_H

#include <stdbool.h>

/**
 * Exports real-time execution flow state and statistics into flow_state.json
 * for the Flask visualizer to read and animate dynamically.
 */
void exportFlowSnapshot(const char* action, const char* details, const char* activeNode, int activeStep, int totalBooks);

#endif // FLOW_EXPORTER_H
