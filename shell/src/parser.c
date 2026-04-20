//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

static char *tokens[1024];
static int token_count = 0;
static int current_token = 0;

void tokenize(char *line) {
    token_count = 0;
    current_token = 0;
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t\n");
    }
}

int parse_atomic();

int cmd_group() {
    if (current_token >= token_count) return 0;
    if (!parse_atomic()) return 0;

    while (current_token < token_count) {
        char *tok = tokens[current_token];
        if (strcmp(tok, "&&") == 0 || strcmp(tok, "||") == 0 || strcmp(tok, ";") == 0) {
            current_token++;
            if (!parse_atomic()) return 0;
        } else if (strcmp(tok, "&") == 0) {
            break;
        } else {
            break;
        }
    }
    return 1;
}

int parse_atomic() {
    if (current_token >= token_count) return 0;
    char *tok = tokens[current_token];

    if (strcmp(tok, "(") == 0) {
        current_token++;
        if (!cmd_group()) return 0;
        if (current_token >= token_count || strcmp(tokens[current_token], ")") != 0) return 0;
        current_token++;
        return 1;
    } else {
        if (strcmp(tok, "&") == 0 || strcmp(tok, "&&") == 0 || strcmp(tok, "||") == 0 ||
            strcmp(tok, ";") == 0 || strcmp(tok, ")") == 0) {
            return 0;
        }
        current_token++;
        while (current_token < token_count) {
            char *arg = tokens[current_token];
            if (strcmp(arg, "&") == 0 || strcmp(arg, "&&") == 0 || strcmp(arg, "||") == 0 ||
                strcmp(arg, ";") == 0 || strcmp(arg, ")") == 0) {
                break;
            }
            current_token++;
        }
        return 1;
    }
}

int valid_command(const char *line) {
    char line_copy[1024];
    strncpy(line_copy, line, sizeof(line_copy));
    line_copy[sizeof(line_copy) - 1] = '\0';

    tokenize(line_copy);
    if (token_count == 0) return 1;

    int res = cmd_group();
    if (res && (current_token == token_count ||
               (current_token == token_count - 1 && strcmp(tokens[current_token], "&") == 0))) {
        return 1;
    }
    return 0;
}
//############## LLM Generated Code Ends ###############