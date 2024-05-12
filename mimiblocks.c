#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int max_y = 0;

typedef struct Node {
    char c;
    int y, x;
    struct Node *prev, *next;
} Node;

void save_text(Node *head, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        Node *curr = head;
        while (curr != NULL) {
            fputc(curr->c, file);
            curr = curr->next;
        }
        fclose(file);
    }
}

Node *read_text(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        file = fopen(filename, "w");
        fclose(file);
        file = fopen(filename, "r");
    }

    Node *head = NULL, *tail = NULL;
    int y = 0, x = 0;
    char c;

    while ((c = fgetc(file)) != EOF) {
        Node *new_node = malloc(sizeof(Node));
        new_node->c = c;
        new_node->y = y;
        new_node->x = x;
        new_node->prev = NULL;
        new_node->next = NULL;

        if (head == NULL) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            tail = new_node;
        }

        if (c == '\n') {
            y++;
            x = 0;
        } else {
            x++;
        }
    }
    max_y = y-1;
    fclose(file);
    return head;
}

Node* encontrar_node(Node* start_node, int y, int x, int opc) {
    Node* current = start_node;
    Node* encontrado;

    if (opc == 1) {
        while (current != NULL) {
            if (current->y == y) {
                if (current->x == x) {
                    return current;
                }
                encontrado = current;
            } else if (current->y == y+1) {
                return encontrado;
            }
            current = current->next;
        }
    } else {
        bool primeraVezY = true;
        while (current != NULL) {
            if (current->y == y) {
                if (current->x == x) {
                    return current;
                } else if (primeraVezY) {
                    encontrado = current;
                    primeraVezY = false;
                }
            } else if (current->y == y-1) {
                return encontrado;
            }
            current = current->prev;
        }
    }
    return NULL;
}

void modificar_x(Node* start_node, int index) {
    Node* current = start_node;

    if (start_node != NULL) {
        int current_y = start_node->y;
        while (current != NULL && current->y == current_y) {
            if (index == 1) {
                current->x++;
            } else if (index == 0) {
                current->x--;
            }
            current = current->next;
        }
    }
}

void modificar_y(Node* start_node, int index) {
    Node* current = start_node;
    int start_y = start_node->y;

    if (index == 1) {
        current->x = 0;
        current->y++;
        int next_x = 1;

        while (current->next != NULL) {
            current = current->next;
            current->y++;

            if (current->y == start_y+1) {
                current->x = next_x++;
            }
        }
    } else if (index == 0) {
        int next_x = start_node->x;
        while (current->next != NULL) {
            current = current->next;
            current->y--;

            if (current->y == start_y) {
                current->x = ++next_x;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo.txt>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    Node *head = read_text(filename);
    Node *curr = head;
    int y = 0, x = 0;

    clear();
    Node *temp = head;
    while (temp != NULL) {
        mvaddch(temp->y, temp->x, temp->c);
        temp = temp->next;
    }
    refresh();
    move(0, 0);

    int ch;
    while ((ch = getch()) != 27) {
        switch (ch) {
            case KEY_UP:
                if (y > 0) {
                    Node* nuevo = encontrar_node(curr, --y, x, 0);
                    curr = nuevo;
                    x = curr->x;
                    y = curr->y;
                }
                break;
            case KEY_DOWN:
                if (y < max_y) {
                    Node* nuevo = encontrar_node(curr, ++y, x, 1);
                    curr = nuevo;
                    x = curr->x;
                    y = curr->y;
                }
                break;
            case KEY_LEFT:
                if (curr != NULL && curr->prev != NULL) {
                    curr = curr->prev;
                    y = curr->y;
                    x = curr->x;
                }
                break;
            case KEY_RIGHT:
                if (curr != NULL && curr->next != NULL) {
                    curr = curr->next;
                    y = curr->y;
                    x = curr->x;
                }
                break;
            case KEY_BACKSPACE:
                if (curr != NULL && curr->prev != NULL) {
                    Node *temp = curr->prev;
                    curr->x = temp->x;
                    curr->y = temp->y;

                    if((curr->prev = temp->prev) != NULL) {
                        Node *ant = temp->prev;
                        ant->next = curr;
                    }

                    if (temp->c == '\n') {
                        modificar_y(curr, 0);
                    } else {
                        modificar_x(curr->next, 0);
                    }
                    
                    free(temp);
                    x = curr->x;
                    y = curr->y;
                }
                break;
            case '\n':
                if (y < LINES - 1) {
                    if (curr != NULL) {
                        Node *new_node = malloc(sizeof(Node));
                        new_node->c = '\n';
                        new_node->y = y;
                        new_node->x = x;
                        new_node->prev = curr->prev;
                        new_node->next = curr;
                        
                        Node *ant;
                        if ((ant = curr->prev) != NULL) {
                            ant->next = new_node;
                        }
                        curr->prev = new_node;
                        
                        modificar_y(curr, 1);                      
                    } else {
                        head = malloc(sizeof(Node));
                        head->c = ch;
                        head->y = y;
                        head->x = x;
                        head->prev = NULL;

                        curr = malloc(sizeof(Node));
                        curr->prev = head;
                        curr->c = ' ';
                        curr->y = y+1;
                        curr->x = x;
                        head->next = curr;
                    }

                    x = 0;
                    y++;
                    if (y > max_y) {
                            max_y = y;
                        }
                }
                break;
            default:
                if (ch >= 32 && ch <= 126) {
                    if (curr != NULL) {
                        Node *new_node = malloc(sizeof(Node));
                        new_node->c = ch;
                        new_node->y = y;
                        new_node->x = x;
                        new_node->prev = curr->prev;
                        new_node->next = curr;
                        
                        Node *ant;

                        if ((ant = curr->prev) != NULL) {
                            ant->next = new_node;
                        } else {
                            head = new_node;
                        }
                        curr->prev = new_node;
                        
                        modificar_x(curr, 1);
                    } else {
                        head = malloc(sizeof(Node));
                        head->c = ch;
                        head->y = head->x = 0;
                        head->prev = NULL;

                        curr = malloc(sizeof(Node));
                        curr->prev = head;
                        curr->c = ' ';
                        curr->y = y;
                        curr->x = x+1;
                        head->next = curr;
                    }
                    
                    x++;   
                    if (y > max_y) {
                        max_y = y;
                    }
                }
                break;
        }

        if (x < 0) {
            x = 0;
        } else if (x >= COLS) {
            x = COLS - 1;
        }

        if (y < 0) {
            y = 0;
        } else if (y >= LINES) {
            y = LINES - 1;
        }

        clear();
        move(0, 0);
        Node *current_node = head;
        while (current_node != NULL) {
            mvaddch(current_node->y, current_node->x, current_node->c);
            current_node = current_node->next;
        }
        move(y, x);
        refresh();
    }

    save_text(head, filename);
    endwin();

    temp = head;
    while (temp != NULL) {
        Node *next = temp->next;
        free(temp);
        temp = next;
    }

    return 0;
}