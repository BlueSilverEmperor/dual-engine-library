#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "flow_exporter.h"

// Helper to escape quotes for valid JSON strings
static void sanitizeJsonString(char *dest, const char *src, size_t maxLen) {
    if (!src || !dest || maxLen == 0) return;
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d < maxLen - 2; s++) {
        if (src[s] == '"' || src[s] == '\\') {
            dest[d++] = '\\';
        }
        if (src[s] == '\n' || src[s] == '\r') {
            dest[d++] = ' ';
        } else {
            dest[d++] = src[s];
        }
    }
    dest[d] = '\0';
}

static void writeJsonFile(const char* filepath, const char* action, const char* details, const char* activeNode, int activeStep, int totalBooks) {
    FILE *f = fopen(filepath, "w");
    if (!f) return;

    char cleanAction[256];
    char cleanDetails[512];
    char cleanNode[128];

    sanitizeJsonString(cleanAction, action ? action : "IDLE", sizeof(cleanAction));
    sanitizeJsonString(cleanDetails, details ? details : "System initialized and waiting...", sizeof(cleanDetails));
    sanitizeJsonString(cleanNode, activeNode ? activeNode : "gui", sizeof(cleanNode));

    time_t rawtime;
    struct tm *timeinfo;
    char timeBuffer[64];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo) {
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    } else {
        strcpy(timeBuffer, "2026-10-03 14:38:00");
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"lastAction\": \"%s\",\n", cleanAction);
    fprintf(f, "  \"details\": \"%s\",\n", cleanDetails);
    fprintf(f, "  \"activeNode\": \"%s\",\n", cleanNode);
    fprintf(f, "  \"activeStep\": %d,\n", activeStep);
    fprintf(f, "  \"timestamp\": \"%s\",\n", timeBuffer);
    fprintf(f, "  \"stats\": {\n");
    fprintf(f, "    \"totalBooks\": %d\n", totalBooks);
    fprintf(f, "  }\n");
    fprintf(f, "}\n");

    fclose(f);
}

void exportFlowSnapshot(const char* action, const char* details, const char* activeNode, int activeStep, int totalBooks) {
    writeJsonFile("flow_visualizer/flow_state.json", action, details, activeNode, activeStep, totalBooks);
    writeJsonFile("flow_state.json", action, details, activeNode, activeStep, totalBooks);
}
