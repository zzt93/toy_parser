//
// Created by zzt on 12/4/16.
//
extern "C" {
#include "compiler.h"
}

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stdlib.h>

using namespace std;

void assign_str(char **dest, const char *src) {
    *dest = (char *) malloc(strlen(src) + 1);
    strcpy(*dest, src);
}

bool operand(int type) {
    return type == ID || type == NUM;
}

#define MAX_DEPTH 128
static StatementNode *stack[MAX_DEPTH];
static int sp = 0;
map<string, ValueNode *> table;

ValueNode *getVar();

ValueNode *getConst();

void addTail(StatementNode **tail, StatementNode *newTail);

/**
 * Implementation Note:
 * <br>implement while by if statement with goto</br>
 * <br>set fake noop true branch at {, set fake noop false branch at }</br>
 * <br>add noop statement to connect fake branch and real branch</br>
 * @return
 */
struct StatementNode *parse_generate_intermediate_representation() {
    StatementNode *head = new StatementNode;
    StatementNode *tail = head;
    ValueNode *last = NULL;

    int type;
    while ((type = getToken()) != EOF && type != ERROR) {
        switch (type) {
            case EQUAL: {
                StatementNode *assign = new StatementNode;
                assign->type = ASSIGN_STMT;
                AssignmentStatement *equal = new AssignmentStatement;
                assert(last != NULL);
                equal->left_hand_side = last;

                int first = getToken();
                assert(operand(first));
                if (first == ID) {
                    equal->operand1 = getVar();
                } else {
                    equal->operand1 = getConst();
                }
                int op;
                switch ((op = getToken())) {
                    case PLUS:
                    case MINUS:
                    case DIV:
                    case MULT: {
                        equal->op = op;
                        int second = getToken();
                        assert(operand(second));
                        if (second == ID) {
                            equal->operand2 = getVar();
                        } else {
                            equal->operand2 = getConst();
                        }
                        break;
                    }
                    case SEMICOLON:
                    case COMMA:
                        break;
                    default:
                        assert(false);
                }
                assign->assign_stmt = equal;
                addTail(&tail, assign);
                break;
            }
            case IF:
            case WHILE: {
                StatementNode *node = new StatementNode;
                if (type == IF) {
                    node->type = IF_STMT;
                } else if (type == WHILE) {
                    node->type = WHILE_STMT;
                } else {
                    assert(false);
                }
                node->if_stmt = new IfStatement;
                stack[sp++] = node;
                addTail(&tail, node);
                break;
            }
            case PRINT: {
                StatementNode *node = new StatementNode;

                node->type = PRINT_STMT;
                int var = getToken();
                assert(var == ID);
                last = getVar();
                node->print_stmt = new PrintStatement;
                node->print_stmt->id = last;
                addTail(&tail, node);
                break;
            }
            case GREATER:
            case NOTEQUAL:
            case LESS: {
                assert(sp > 0);
                StatementNode *node = stack[sp - 1];
                node->if_stmt->condition_operand1 = last;
                node->if_stmt->condition_op = type;
                int sec = getToken();
                assert(operand(sec));
                if (sec == ID) {
                    node->if_stmt->condition_operand2 = getVar();
                } else {
                    node->if_stmt->condition_operand2 = getConst();
                }
                break;
            }
            case LBRACE: {
                if (sp == 0) {
                    continue;
                }
                StatementNode *node = stack[sp - 1];
                StatementNode *empty = new StatementNode;
                empty->type = NOOP_STMT;
                node->if_stmt->true_branch = empty;
                addTail(&tail, empty);
                break;
            }
            case RBRACE: {
                if (sp == 0) {
                    continue;
                }
                StatementNode *node = stack[--sp];
                StatementNode *empty = new StatementNode;
                empty->type = NOOP_STMT;
                node->if_stmt->false_branch = empty;
                if (node->type == IF_STMT) {
                } else if (node->type == WHILE_STMT) {
                    node->type = IF_STMT;
                    StatementNode *back = new StatementNode;
                    back->type = GOTO_STMT;
                    back->goto_stmt = new GotoStatement;
                    back->goto_stmt->target = node;
                    addTail(&tail, back);
                }
                addTail(&tail, empty);
            }
            case COMMA:
            case SEMICOLON:
                break;
            case ID: {
                last = getVar();
                break;
            }
            default:
                break;
        }
    }
    tail->next = NULL;
    return head->next;
}

void addTail(StatementNode **tail, StatementNode *newTail) {
    (*tail)->next = newTail;
    *tail = newTail;
}

ValueNode *getVar() {
    ValueNode *last;
    string name(token);
    if (table.count(name)) {
        last = table[name];
    } else {
        last = new ValueNode;
        table[name] = last;
        assign_str(&last->name, token);
        last->value = 0;
    }
    return last;
}

ValueNode *getConst() {
    ValueNode *node = new ValueNode;
    node->value = atoi(token);
    return node;
}
