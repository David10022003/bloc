#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

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
        return NULL;
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

    fclose(file);
    return head;
}

Node* encontrar_node(Node* head, int y, int x) {
    Node* current = head;
    while (current != NULL) {
        if (current->y == y && current->x == x) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void modificar_x(Node* start_node, int index) {
    Node* current = start_node;
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
                    Node* nuevo = encontrar_node(head, --y, x);
                    curr = nuevo;
                }
                break;
            case KEY_DOWN:
                Node* nuevo = encontrar_node(head, ++y, x);
                curr = nuevo;
                break;
            case KEY_LEFT:
                if (curr->prev != NULL) {
                    curr = curr->prev;
                    y = curr->y;
                    x = curr->x;
                }
                break;
            case KEY_RIGHT:
                if (curr->next != NULL) {
                    curr = curr->next;
                    y = curr->y;
                    x = curr->x;
                }
                break;
            case KEY_BACKSPACE:
                if (curr->prev != NULL) {
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
                    y++;
                    x=0; 
                }
                break;
            default:
                if (ch >= 32 && ch <= 126) {
                    Node *new_node = malloc(sizeof(Node));
                    new_node->c = ch;
                    new_node->y = y;
                    new_node->x = x;
                    new_node->prev = curr->prev;
                    new_node->next = curr;
                    
                    Node *ant;

                    if ((ant = curr->prev) != NULL) {
                        ant->next = new_node;
                    }
                    curr->prev = new_node;
                    
                    modificar_x(curr, 1);
                    x++;   
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

    return 0;
}
