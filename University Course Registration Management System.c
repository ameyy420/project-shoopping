#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 32
#define MAX_NAME_LEN 128
#define MAX_COMPLETED 200
#define MAX_ENROLLED 50
#define FILENAME "registration_data.txt"

typedef struct TimeSlot {
    int day;
    int start;
    int end;
    struct TimeSlot *next;
} TimeSlot;

typedef struct Course {
    char code[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int credit;
    int capacity;
    int currentEnrollment;
    TimeSlot *slots;
    struct Course *left;
    struct Course *right;
} Course;

typedef struct Prereq {
    char course[MAX_ID_LEN];
    char prereq[MAX_ID_LEN];
    struct Prereq *next;
} Prereq;

typedef struct Student {
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    float cgpa;
    char completed[MAX_COMPLETED][MAX_ID_LEN];
    int completedCount;
    char enrolled[MAX_ENROLLED][MAX_ID_LEN];
    int enrolledCount;
    struct Student *next;
    struct Student *prev;
} Student;

typedef struct WaitQueueNode {
    char studentId[MAX_ID_LEN];
    char courseCode[MAX_ID_LEN];
    float priority;
    struct WaitQueueNode *next;
} WaitQueueNode;

typedef struct EnrollAction {
    char studentId[MAX_ID_LEN];
    char courseCode[MAX_ID_LEN];
    int actionType;
    struct EnrollAction *next;
} EnrollAction;

Course *courseRoot = NULL;
Student *studentHead = NULL;
Prereq *prHead = NULL;
WaitQueueNode *waitQueueHead = NULL;
EnrollAction *actionStackHead = NULL;

void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n == 0) return;
    if (s[n-1] == '\n') s[n-1] = '\0';
}

Course* createCourseNode(const char *code, const char *name, int credit, int capacity) {
    Course *c = (Course*)malloc(sizeof(Course));
    if (!c) return NULL;
    strncpy(c->code, code, MAX_ID_LEN-1); c->code[MAX_ID_LEN-1] = '\0';
    strncpy(c->name, name, MAX_NAME_LEN-1); c->name[MAX_NAME_LEN-1] = '\0';
    c->credit = credit;
    c->capacity = capacity;
    c->currentEnrollment = 0;
    c->slots = NULL;
    c->left = c->right = NULL;
    return c;
}

Student* createStudentNode(const char *id, const char *name, float cgpa) {
    Student *s = (Student*)malloc(sizeof(Student));
    if (!s) return NULL;
    strncpy(s->id, id, MAX_ID_LEN-1); s->id[MAX_ID_LEN-1] = '\0';
    strncpy(s->name, name, MAX_NAME_LEN-1); s->name[MAX_NAME_LEN-1] = '\0';
    s->cgpa = cgpa;
    s->completedCount = 0;
    s->enrolledCount = 0;
    s->next = s->prev = NULL;
    return s;
}

TimeSlot* createTimeSlotNode(int day, int start, int end) {
    TimeSlot *t = (TimeSlot*)malloc(sizeof(TimeSlot));
    if (!t) return NULL;
    t->day = day;
    t->start = start;
    t->end = end;
    t->next = NULL;
    return t;
}

Prereq* createPrereqNode(const char *course, const char *pr) {
    Prereq *p = (Prereq*)malloc(sizeof(Prereq));
    if (!p) return NULL;
    strncpy(p->course, course, MAX_ID_LEN-1); p->course[MAX_ID_LEN-1] = '\0';
    strncpy(p->prereq, pr, MAX_ID_LEN-1); p->prereq[MAX_ID_LEN-1] = '\0';
    p->next = NULL;
    return p;
}

WaitQueueNode* createWaitNode(const char *sid, const char *cc, float prio) {
    WaitQueueNode *w = (WaitQueueNode*)malloc(sizeof(WaitQueueNode));
    if (!w) return NULL;
    strncpy(w->studentId, sid, MAX_ID_LEN-1); w->studentId[MAX_ID_LEN-1] = '\0';
    strncpy(w->courseCode, cc, MAX_ID_LEN-1); w->courseCode[MAX_ID_LEN-1] = '\0';
    w->priority = prio;
    w->next = NULL;
    return w;
}

EnrollAction* createActionNode(const char *sid, const char *cc, int type) {
    EnrollAction *a = (EnrollAction*)malloc(sizeof(EnrollAction));
    if (!a) return NULL;
    strncpy(a->studentId, sid, MAX_ID_LEN-1); a->studentId[MAX_ID_LEN-1] = '\0';
    strncpy(a->courseCode, cc, MAX_ID_LEN-1); a->courseCode[MAX_ID_LEN-1] = '\0';
    a->actionType = type;
    a->next = NULL;
    return a;
}

Course* insertCourseBST(Course* root, Course* newCourse) {
    if (root == NULL) {
        return newCourse;
    }
    int compareResult = strcmp(newCourse->code, root->code);

    if (compareResult < 0) {
        root->left = insertCourseBST(root->left, newCourse);
    } else if (compareResult > 0) {
        root->right = insertCourseBST(root->right, newCourse);
    }

    return root;
}

Course* findCourse(const char *code) {
    Course *current = courseRoot;

    while (current != NULL) {
        int compareResult = strcmp(code, current->code);

        if (compareResult == 0) {
            return current;
        } else if (compareResult < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return NULL;
}

Student* findStudent(const char *id) {
    Student *s = studentHead;
    while (s) {
        if (strcmp(s->id, id) == 0) return s;
        s = s->next;
    }
    return NULL;
}

int studentHasCompleted(Student *s, const char *code) {
    if (!s) return 0;
    for (int i = 0; i < s->completedCount; ++i) {
        if (strcmp(s->completed[i], code) == 0) return 1;
    }
    return 0;
}

int checkPrereqs(Student *s, Course *c) {
    if (!s || !c) return 0;
    Prereq *p = prHead;
    while (p) {
        if (strcmp(p->course, c->code) == 0) {
            if (!studentHasCompleted(s, p->prereq)) return 0;
        }
        p = p->next;
    }
    return 1;
}

int timeslotOverlap(TimeSlot *a, TimeSlot *b) {
    if (!a || !b) return 0;
    if (a->day != b->day) return 0;
    if (a->end <= b->start) return 0;
    if (b->end <= a->start) return 0;
    return 1;
}

int studentScheduleConflict(Student *s, Course *c) {
    if (!s || !c) return 0;
    for (int i = 0; i < s->enrolledCount; ++i) {
        Course *ec = findCourse(s->enrolled[i]);
        if (!ec) continue;
        TimeSlot *ts1 = ec->slots;
        while (ts1) {
            TimeSlot *ts2 = c->slots;
            while (ts2) {
                if (timeslotOverlap(ts1, ts2)) return 1;
                ts2 = ts2->next;
            }
            ts1 = ts1->next;
        }
    }
    return 0;
}

void addCourse(const char *code, const char *name, int credit, int capacity) {
    if (findCourse(code)) {
        printf("Course %s already exists.\n", code);
        return;
    }
    Course *c = createCourseNode(code, name, credit, capacity);
    if (!c) { printf("Memory error adding course.\n"); return; }

    courseRoot = insertCourseBST(courseRoot, c);

    printf("Course %s added.\n", code);
}

int removeTimeSlots(TimeSlot *t) {
    TimeSlot *cur = t;
    while (cur) {
        TimeSlot *nx = cur->next;
        free(cur);
        cur = nx;
    }
    return 1;
}

Course* findMin(Course* node) {
    Course* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

Course* deleteCourseBST_Advanced(Course* root, const char* code, Course** removedNode) {
    if (root == NULL) return NULL;

    int compareResult = strcmp(code, root->code);

    if (compareResult < 0) {
        root->left = deleteCourseBST_Advanced(root->left, code, removedNode);
    } else if (compareResult > 0) {
        root->right = deleteCourseBST_Advanced(root->right, code, removedNode);
    } else {
        *removedNode = root;

        if (root->left == NULL) {
            Course *temp = root->right;
            root->right = NULL;
            return temp;
        } else if (root->right == NULL) {
            Course *temp = root->left;
            root->left = NULL;
            return temp;
        }

        Course* temp = findMin(root->right);

        *removedNode = temp;

        strncpy(root->code, temp->code, MAX_ID_LEN-1);
        strncpy(root->name, temp->name, MAX_NAME_LEN-1);
        root->credit = temp->credit;
        root->capacity = temp->capacity;
        root->currentEnrollment = temp->currentEnrollment;

        TimeSlot *temp_slots = root->slots;
        root->slots = temp->slots;
        temp->slots = temp_slots;

        root->right = deleteCourseBST_Advanced(root->right, temp->code, removedNode);
    }
    return root;
}

void deleteCourse(const char *code) {
    Course *c = findCourse(code);
    if (!c) { printf("Course %s not found.\n", code); return; }

    if (c->currentEnrollment > 0) {
        Student *s = studentHead;
        while (s) {
            int changed = 0;
            for (int i = 0; i < s->enrolledCount; ++i) {
                if (strcmp(s->enrolled[i], code) == 0) {
                    for (int j = i; j < s->enrolledCount - 1; ++j)
                        strncpy(s->enrolled[j], s->enrolled[j+1], MAX_ID_LEN-1);
                    s->enrolledCount--;
                    changed = 1;
                    i--;
                }
            }
            if (changed) printf("Student %s auto-dropped from %s due to course deletion.\n", s->id, code);
            s = s->next;
        }
    }

    WaitQueueNode *prev = NULL;
    WaitQueueNode *cur = waitQueueHead;
    while (cur) {
        if (strcmp(cur->courseCode, code) == 0) {
            WaitQueueNode *rem = cur;
            if (prev) prev->next = cur->next;
            else waitQueueHead = cur->next;
            cur = cur->next;
            free(rem);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    Prereq *pp = prHead, *pprev = NULL;
    while (pp) {
        if (strcmp(pp->course, code) == 0 || strcmp(pp->prereq, code) == 0) {
            Prereq *rem = pp;
            if (pprev) pprev->next = pp->next;
            else prHead = pp->next;
            pp = pp->next;
            free(rem);
        } else {
            pprev = pp;
            pp = pp->next;
        }
    }

    Course *nodeToFree = NULL;
    courseRoot = deleteCourseBST_Advanced(courseRoot, code, &nodeToFree);

    if (nodeToFree) {
        removeTimeSlots(nodeToFree->slots);
        free(nodeToFree);
        printf("Course %s deleted.\n", code);
    } else {
        printf("Error: Course %s found but could not be removed from tree.\n", code);
    }
}

void addStudent(const char *id, const char *name, float cgpa) {
    if (findStudent(id)) {
        printf("Student %s already exists.\n", id);
        return;
    }
    Student *s = createStudentNode(id, name, cgpa);
    if (!s) { printf("Memory error adding student.\n"); return; }
    s->next = studentHead;
    if (studentHead) studentHead->prev = s;
    studentHead = s;
    printf("Student %s added.\n", id);
}

void deleteStudent(const char *id) {
    Student *s = findStudent(id);
    if (!s) { printf("Student %s not found.\n", id); return; }

    WaitQueueNode *prev = NULL;
    WaitQueueNode *cur = waitQueueHead;
    while (cur) {
        if (strcmp(cur->studentId, id) == 0) {
            WaitQueueNode *rem = cur;
            if (prev) prev->next = cur->next;
            else waitQueueHead = cur->next;
            cur = cur->next;
            free(rem);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    for (int i = 0; i < s->enrolledCount; ++i) {
        Course *c = findCourse(s->enrolled[i]);
        if (c) {
            if (c->currentEnrollment > 0) c->currentEnrollment--;
            printf("Student %s removed from course %s due to student deletion.\n", id, c->code);

        }
    }

    if (s->prev) s->prev->next = s->next;
    if (s->next) s->next->prev = s->prev;
    if (studentHead == s) studentHead = s->next;
    free(s);
    printf("Student %s deleted.\n", id);
}


void addTimeSlotToCourse(const char *code, int day, int start, int end) {
    Course *c = findCourse(code);
    if (!c) { printf("Course %s not found.\n", code); return; }
    TimeSlot *t = createTimeSlotNode(day, start, end);
    if (!t) { printf("Memory error adding timeslot.\n"); return; }
    t->next = c->slots;
    c->slots = t;
    printf("TimeSlot added to %s: Day %d, %02d-%02d\n", code, day, start, end);
}

void removeTimeSlotFromCourse(const char *code, int day, int start, int end) {
    Course *c = findCourse(code);
    if (!c) { printf("Course %s not found.\n", code); return; }
    TimeSlot *prev = NULL;
    TimeSlot *cur = c->slots;
    int removed = 0;
    while (cur) {
        if (cur->day == day && cur->start == start && cur->end == end) {
            TimeSlot *rem = cur;
            if (prev) prev->next = cur->next;
            else c->slots = cur->next;
            cur = cur->next;
            free(rem);
            removed = 1;

        } else {
            prev = cur;
            cur = cur->next;
        }
    }
    if (removed) printf("Removed timeslot from %s.\n", code);
    else printf("Timeslot not found in %s.\n", code);
}


void addPrereq(const char *course, const char *pr) {
    Course *c = findCourse(course);
    Course *p = findCourse(pr);
    if (!c) { printf("Course %s not found (target).\n", course); return; }
    if (!p) { printf("Prereq course %s not found.\n", pr); return; }

    Prereq *iter = prHead;
    while (iter) {
        if (strcmp(iter->course, course) == 0 && strcmp(iter->prereq, pr) == 0) {
            printf("Prereq already exists.\n"); return;
        }
        iter = iter->next;
    }
    Prereq *node = createPrereqNode(course, pr);
    node->next = prHead;
    prHead = node;
    printf("Prereq added: %s -> %s\n", course, pr);
}

void deletePrereq(const char *course, const char *pr) {
    Prereq *cur = prHead, *prev = NULL;
    while (cur) {
        if (strcmp(cur->course, course) == 0 && strcmp(cur->prereq, pr) == 0) {
            Prereq *rem = cur;
            if (prev) prev->next = cur->next;
            else prHead = cur->next;
            free(rem);
            printf("Prereq removed: %s -> %s\n", course, pr);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("Prereq not found.\n");
}


void enqueueWait(const char *sid, const char *cc, float prio) {
    WaitQueueNode *n = createWaitNode(sid, cc, prio);
    if (!n) { printf("Memory error enqueue wait.\n"); return; }
    if (!waitQueueHead || prio > waitQueueHead->priority) {
        n->next = waitQueueHead;
        waitQueueHead = n;
        return;
    }
    WaitQueueNode *cur = waitQueueHead;
    while (cur->next && cur->next->priority >= prio) cur = cur->next;
    n->next = cur->next;
    cur->next = n;
}


void removeWaitNode(WaitQueueNode *prev, WaitQueueNode *cur) {
    if (!cur) return;
    if (prev) prev->next = cur->next;
    else waitQueueHead = cur->next;
    free(cur);
}


void pushAction(const char *sid, const char *cc, int type) {
    EnrollAction *a = createActionNode(sid, cc, type);
    if (!a) return;
    a->next = actionStackHead;
    actionStackHead = a;
}

EnrollAction* popAction() {
    if (!actionStackHead) return NULL;
    EnrollAction *top = actionStackHead;
    actionStackHead = actionStackHead->next;
    top->next = NULL;
    return top;
}


void attemptEnroll(Student *s, Course *c) {
    if (!s || !c) return;
    if (s->enrolledCount >= MAX_ENROLLED) {
        printf("Student %s reached max enrolled.\n", s->id);
        return;
    }
    if (!checkPrereqs(s, c)) {
        enqueueWait(s->id, c->code, s->cgpa);
        printf("WAITLISTED: Prereq missing for %s (student %s).\n", c->code, s->id);
        return;
    }
    if (studentScheduleConflict(s, c)) {
        enqueueWait(s->id, c->code, s->cgpa);
        printf("WAITLISTED: Schedule conflict for %s (student %s).\n", c->code, s->id);
        return;
    }
    if (c->currentEnrollment >= c->capacity) {
        enqueueWait(s->id, c->code, s->cgpa);
        printf("WAITLISTED: Course %s is full (student %s).\n", c->code, s->id);
        return;
    }


    strncpy(s->enrolled[s->enrolledCount++], c->code, MAX_ID_LEN-1);
    c->currentEnrollment++;
    printf("SUCCESS: Student %s enrolled in %s.\n", s->id, c->code);
    pushAction(s->id, c->code, 1);
}

int removeEnrolledCourse(Student *s, const char *code) {
    if (!s) return 0;
    for (int i = 0; i < s->enrolledCount; ++i) {
        if (strcmp(s->enrolled[i], code) == 0) {
            for (int j = i; j < s->enrolledCount - 1; ++j)
                strncpy(s->enrolled[j], s->enrolled[j+1], MAX_ID_LEN-1);
            s->enrolledCount--;
            return 1;
        }
    }
    return 0;
}

void dropCourse(Student *s, Course *c) {
    if (!s || !c) return;
    if (removeEnrolledCourse(s, c->code)) {
        if (c->currentEnrollment > 0) c->currentEnrollment--;
        printf("DROPPED: Student %s dropped %s.\n", s->id, c->code);
        pushAction(s->id, c->code, 2);

        WaitQueueNode *prev = NULL, *cur = waitQueueHead;
        while (cur) {
            if (strcmp(cur->courseCode, c->code) == 0) {
                Student *ws = findStudent(cur->studentId);
                if (!ws) { prev = cur; cur = cur->next; continue; }
                if (!checkPrereqs(ws, c) || studentScheduleConflict(ws, c) || ws->enrolledCount >= MAX_ENROLLED) {
                    prev = cur; cur = cur->next;
                    continue;
                }

                strncpy(ws->enrolled[ws->enrolledCount++], c->code, MAX_ID_LEN-1);
                c->currentEnrollment++;
                printf("[WAITLIST PROMOTE] %s enrolled in %s from waitlist.\n", ws->id, c->code);
                WaitQueueNode *rem = cur;
                cur = cur->next;
                if (prev) prev->next = cur;
                else waitQueueHead = cur;
                free(rem);
                pushAction(ws->id, c->code, 1);

                if (c->currentEnrollment >= c->capacity) break;
            } else {
                prev = cur; cur = cur->next;
            }
        }
    } else {
        printf("Student %s is not enrolled in %s.\n", s->id, c->code);
    }
}

void processWaitlistInOrder(Course *c) {
    if (c == NULL) return;

    processWaitlistInOrder(c->left);

    if (c->currentEnrollment < c->capacity) {
        WaitQueueNode *prev = NULL, *cur = waitQueueHead;
        while (cur && c->currentEnrollment < c->capacity) {
            if (strcmp(cur->courseCode, c->code) == 0) {
                Student *s = findStudent(cur->studentId);
                if (!s || !checkPrereqs(s, c) || studentScheduleConflict(s, c) || s->enrolledCount >= MAX_ENROLLED) {
                    prev = cur; cur = cur->next;
                    continue;
                }

                strncpy(s->enrolled[s->enrolledCount++], c->code, MAX_ID_LEN-1);
                c->currentEnrollment++;
                printf("[WAITLIST] Student %s enrolled in %s (P: %.2f).\n", s->id, c->code, s->cgpa);
                WaitQueueNode *rem = cur;
                cur = cur->next;
                if (prev) prev->next = cur; else waitQueueHead = cur;
                free(rem);
                pushAction(s->id, c->code, 1);
            } else {
                prev = cur; cur = cur->next;
            }
        }
    }

    processWaitlistInOrder(c->right);
}

void processAllWaitlists() {
    printf("\n--- Processing all waitlists ---\n");
    processWaitlistInOrder(courseRoot);
    printf("Processing complete.\n");
}


void undoLastAction() {
    EnrollAction *a = popAction();
    if (!a) {
        printf("No actions to undo.\n");
        return;
    }
    Student *s = findStudent(a->studentId);
    Course *c = findCourse(a->courseCode);
    if (!s || !c) {
        printf("Undo failed: entities not found.\n");
        free(a);
        return;
    }
    if (a->actionType == 1) {
        if (removeEnrolledCourse(s, c->code)) {
            if (c->currentEnrollment > 0) c->currentEnrollment--;
            printf("UNDO: Enrollment of %s in %s undone.\n", s->id, c->code);

            WaitQueueNode *prev = NULL, *cur = waitQueueHead;
            while (cur) {
                if (strcmp(cur->courseCode, c->code) == 0) {
                    Student *ws = findStudent(cur->studentId);
                    if (!ws || !checkPrereqs(ws, c) || studentScheduleConflict(ws, c) || ws->enrolledCount >= MAX_ENROLLED) {
                        prev = cur; cur = cur->next;
                        continue;
                    }

                    strncpy(ws->enrolled[ws->enrolledCount++], c->code, MAX_ID_LEN-1);
                    c->currentEnrollment++;
                    printf("[WAITLIST PROMOTE] %s enrolled in %s.\n", ws->id, c->code);
                    WaitQueueNode *rem = cur;
                    cur = cur->next;
                    if (prev) prev->next = cur; else waitQueueHead = cur;
                    free(rem);
                    pushAction(ws->id, c->code, 1);
                    break;
                } else {
                    prev = cur; cur = cur->next;
                }
            }
        } else printf("UNDO failed: student not enrolled.\n");
    } else if (a->actionType == 2) {
        if (s->enrolledCount >= MAX_ENROLLED) {
            printf("UNDO failed: student %s cannot enroll more.\n", s->id);
            free(a);
            return;
        }
        if (!checkPrereqs(s, c) || studentScheduleConflict(s, c) || c->currentEnrollment >= c->capacity) {
            printf("UNDO failed: constraints prevent re-enroll.\n");
            free(a);
            return;
        }
        strncpy(s->enrolled[s->enrolledCount++], c->code, MAX_ID_LEN-1);
        c->currentEnrollment++;
        printf("UNDO: Drop undone. %s re-enrolled in %s.\n", s->id, c->code);
    }
    free(a);
}

void printCourseDetailed(Course *c) {
    if (!c) return;
    printf("\n%s | %s | Credit: %d | Cap: %d | Enrolled: %d\n",
           c->code, c->name, c->credit, c->capacity, c->currentEnrollment);
    TimeSlot *t = c->slots;
    printf("  TimeSlots:");
    if (!t) printf(" [none]");
    while (t) {
        printf(" Day%d %02d-%02d;", t->day, t->start, t->end);
        t = t->next;
    }
    printf("\n  Prereqs:");
    Prereq *p = prHead;
    int first = 1;
    while (p) {
        if (strcmp(p->course, c->code) == 0) {
            if (!first) printf(",");
            printf(" %s", p->prereq);
            first = 0;
        }
        p = p->next;
    }
    if (first) printf(" [none]");
    printf("\n");
}

void printCourseInOrder(Course *root) {
    if (root != NULL) {
        printCourseInOrder(root->left);
        printCourseDetailed(root);
        printCourseInOrder(root->right);
    }
}

void showAllCourses() {
    printf("\n=== COURSE CATALOG ===\n");
    if (!courseRoot) { printf("[No courses]\n"); return; }
    printCourseInOrder(courseRoot);
}

void showAllStudents() {
    Student *s = studentHead;
    printf("\n=== STUDENT RECORDS ===\n");
    if (!s) { printf("[No students]\n"); return; }
    while (s) {
        printf("\nID: %s | Name: %s | CGPA: %.2f\n", s->id, s->name, s->cgpa);
        if (s->completedCount) {
            printf("  Completed:");
            for (int i = 0; i < s->completedCount; ++i) printf(" %s", s->completed[i]);
            printf("\n");
        } else printf("  Completed: [none]\n");
        if (s->enrolledCount) {
            printf("  Enrolled:");
            for (int i = 0; i < s->enrolledCount; ++i) printf(" %s", s->enrolled[i]);
            printf("\n");
        } else printf("  Enrolled: [none]\n");
        s = s->next;
    }
}

void showWaitlist() {
    WaitQueueNode *w = waitQueueHead;
    printf("\n=== WAITLIST (High -> Low Priority) ===\n");
    if (!w) { printf("[Empty waitlist]\n"); return; }
    while (w) {
        printf("  Student %s | Course %s | P: %.2f\n", w->studentId, w->courseCode, w->priority);
        w = w->next;
    }
}

void showActionStack() {
    EnrollAction *a = actionStackHead;
    printf("\n=== ACTION STACK (Top -> Bottom) ===\n");
    if (!a) { printf("[No actions]\n"); return; }
    while (a) {
        printf("  %s %s %s\n", a->studentId, (a->actionType==1?"ENROLLED":"DROPPED"), a->courseCode);
        a = a->next;
    }
}

void saveCourseBST(FILE *f, Course *root) {
    if (root == NULL) return;

    fprintf(f, "COURSE|%s|%s|%d|%d|%d|", root->code, root->name, root->credit, root->capacity, root->currentEnrollment);
    TimeSlot *t = root->slots;
    while (t) {
        fprintf(f, "%d,%d,%d;", t->day, t->start, t->end);
        t = t->next;
    }
    fprintf(f, "\n");

    saveCourseBST(f, root->left);
    saveCourseBST(f, root->right);
}

void saveData() {
    FILE *f = fopen(FILENAME, "w");
    if (!f) { printf("Failed to open file for saving.\n"); return; }
    Student *s = studentHead;
    while (s) {
        fprintf(f, "STUDENT|%s|%s|%.2f|%d|", s->id, s->name, s->cgpa, s->completedCount);
        for (int i = 0; i < s->completedCount; ++i) fprintf(f, "%s;", s->completed[i]);
        fprintf(f, "|%d|", s->enrolledCount);
        for (int i = 0; i < s->enrolledCount; ++i) fprintf(f, "%s;", s->enrolled[i]);
        fprintf(f, "\n");
        s = s->next;
    }
    saveCourseBST(f, courseRoot);

    Prereq *p = prHead;
    while (p) {
        fprintf(f, "PR|%s|%s\n", p->course, p->prereq);
        p = p->next;
    }
    WaitQueueNode *w = waitQueueHead;
    while (w) {
        fprintf(f, "WL|%s|%s|%.2f\n", w->studentId, w->courseCode, w->priority);
        w = w->next;
    }
    fclose(f);
    printf("Data saved to %s\n", FILENAME);
}

void clearCourseBST(Course *root) {
    if (root == NULL) return;
    clearCourseBST(root->left);
    clearCourseBST(root->right);
    removeTimeSlots(root->slots);
    free(root);
}

void clearAllMemory() {
    WaitQueueNode *w = waitQueueHead;
    while (w) { WaitQueueNode *nx = w->next; free(w); w = nx; }
    waitQueueHead = NULL;
    Prereq *p = prHead;
    while (p) { Prereq *nx = p->next; free(p); p = nx; }
    prHead = NULL;

    clearCourseBST(courseRoot);
    courseRoot = NULL;

    Student *s = studentHead;
    while (s) {
        Student *sn = s->next;
        free(s);
        s = sn;
    }
    studentHead = NULL;
    EnrollAction *a = actionStackHead;
    while (a) { EnrollAction *nx = a->next; free(a); a = nx; }
    actionStackHead = NULL;
}

void loadData() {
    FILE *f = fopen(FILENAME, "r");
    if (!f) { printf("No save file found (%s). Skipping load.\n", FILENAME); return; }
    clearAllMemory();
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strncmp(line, "STUDENT|", 8) == 0) {
            char *p = line + 8;
            char id[MAX_ID_LEN], name[MAX_NAME_LEN];
            float cgpa;
            int compCount = 0, enrollCount = 0;
            char *tok = strtok(p, "|");
            if (!tok) continue;
            strncpy(id, tok, MAX_ID_LEN-1); id[MAX_ID_LEN-1] = '\0';
            tok = strtok(NULL, "|"); if (!tok) continue;
            strncpy(name, tok, MAX_NAME_LEN-1); name[MAX_NAME_LEN-1] = '\0';
            tok = strtok(NULL, "|"); if (!tok) continue;
            cgpa = atof(tok);
            tok = strtok(NULL, "|"); if (!tok) continue;
            compCount = atoi(tok);
            Student *s = createStudentNode(id, name, cgpa);
            if (!s) continue;
            tok = strtok(NULL, "|");
            if (tok && compCount > 0) {
                char *cTok = strtok(tok, ";");
                int idx = 0;
                while (cTok && idx < compCount) {
                    strncpy(s->completed[idx++], cTok, MAX_ID_LEN-1);
                    cTok = strtok(NULL, ";");
                }
                s->completedCount = idx;
            }
            tok = strtok(NULL, "|"); if (!tok) tok = "";
            enrollCount = atoi(tok);
            tok = strtok(NULL, "|");
            if (tok && enrollCount > 0) {
                char *eTok = strtok(tok, ";");
                int idx = 0;
                while (eTok && idx < enrollCount) {
                    strncpy(s->enrolled[idx++], eTok, MAX_ID_LEN-1);
                    eTok = strtok(NULL, ";");
                }
                s->enrolledCount = idx;
            }
            s->next = studentHead; if (studentHead) studentHead->prev = s; studentHead = s;
        } else if (strncmp(line, "COURSE|", 7) == 0) {
            char *p = line + 7;
            char *tok = strtok(p, "|");
            if (!tok) continue;
            char code[MAX_ID_LEN]; strncpy(code, tok, MAX_ID_LEN-1);
            tok = strtok(NULL, "|"); if (!tok) continue;
            char name[MAX_NAME_LEN]; strncpy(name, tok, MAX_NAME_LEN-1);
            tok = strtok(NULL, "|"); if (!tok) continue;
            int credit = atoi(tok);
            tok = strtok(NULL, "|"); if (!tok) continue;
            int cap = atoi(tok);
            tok = strtok(NULL, "|"); if (!tok) continue;
            int curEnroll = atoi(tok);
            tok = strtok(NULL, "|");
            Course *c = createCourseNode(code, name, credit, cap);
            c->currentEnrollment = curEnroll;
            if (tok && strlen(tok) > 0) {
                char *tTok = strtok(tok, ";");
                while (tTok) {
                    int d, st, en;
                    if (sscanf(tTok, "%d,%d,%d", &d, &st, &en) == 3) {
                        TimeSlot *ts = createTimeSlotNode(d, st, en);
                        ts->next = c->slots; c->slots = ts;
                    }
                    tTok = strtok(NULL, ";");
                }
            }
            c->left = c->right = NULL;
            courseRoot = insertCourseBST(courseRoot, c);
        } else if (strncmp(line, "PR|", 3) == 0) {
            char *p = line + 3;
            char *tok = strtok(p, "|");
            if (!tok) continue;
            char course[MAX_ID_LEN]; strncpy(course, tok, MAX_ID_LEN-1);
            tok = strtok(NULL, "|");
            if (!tok) continue;
            char prereq[MAX_ID_LEN]; strncpy(prereq, tok, MAX_ID_LEN-1);
            Prereq *prn = createPrereqNode(course, prereq);
            prn->next = prHead; prHead = prn;
        } else if (strncmp(line, "WL|", 3) == 0) {
            char *p = line + 3;
            char *tok = strtok(p, "|");
            if (!tok) continue;
            char sid[MAX_ID_LEN]; strncpy(sid, tok, MAX_ID_LEN-1);
            tok = strtok(NULL, "|"); if (!tok) continue;
            char cc[MAX_ID_LEN]; strncpy(cc, tok, MAX_ID_LEN-1);
            tok = strtok(NULL, "|"); if (!tok) continue;
            float prio = atof(tok);
            enqueueWait(sid, cc, prio);
        }
    }
    fclose(f);
    printf("Data loaded from %s\n", FILENAME);
}

void waitForEnter() {
    printf("\nPress ENTER to continue...");
    while (getchar() != '\n');
}

void getInput(const char *prompt, char *buf, int bufsz) {
    printf("%s", prompt);
    if (fgets(buf, bufsz, stdin) == NULL) { buf[0] = '\0'; return; }
    trim_newline(buf);
}

void menuAddCourse() {
    char code[MAX_ID_LEN], name[MAX_NAME_LEN];
    char scredit[16], scap[16];
    getInput("Course Code: ", code, sizeof(code));
    if (strlen(code) == 0) { printf("Cancelled.\n"); return; }
    getInput("Course Name: ", name, sizeof(name));
    getInput("Credit (int): ", scredit, sizeof(scredit));
    getInput("Capacity (int): ", scap, sizeof(scap));
    int credit = atoi(scredit), cap = atoi(scap);
    addCourse(code, name, credit, cap);
}

void menuDeleteCourse() {
    char code[MAX_ID_LEN];
    getInput("Course Code to delete: ", code, sizeof(code));
    if (strlen(code) == 0) { printf("Cancelled.\n"); return; }
    deleteCourse(code);
}

void menuAddStudent() {
    char id[MAX_ID_LEN], name[MAX_NAME_LEN], scgpa[16];
    getInput("Student ID: ", id, sizeof(id));
    if (strlen(id) == 0) { printf("Cancelled.\n"); return; }
    getInput("Student Name: ", name, sizeof(name));
    getInput("CGPA (e.g., 3.50): ", scgpa, sizeof(scgpa));
    float cgpa = atof(scgpa);
    addStudent(id, name, cgpa);
}

void menuDeleteStudent() {
    char id[MAX_ID_LEN];
    getInput("Student ID to delete: ", id, sizeof(id));
    if (strlen(id) == 0) { printf("Cancelled.\n"); return; }
    deleteStudent(id);
}

void menuAddTimeslot() {
    char code[MAX_ID_LEN];
    char sday[8], sst[8], sen[8];
    getInput("Course Code: ", code, sizeof(code));
    if (strlen(code) == 0) return;
    getInput("Day (1-7): ", sday, sizeof(sday));
    getInput("Start hour (int): ", sst, sizeof(sst));
    getInput("End hour (int): ", sen, sizeof(sen));
    int day = atoi(sday), st = atoi(sst), en = atoi(sen);
    if (day < 1 || day > 7 || st >= en) { printf("Invalid timeslot.\n"); return; }
    addTimeSlotToCourse(code, day, st, en);
}

void menuRemoveTimeslot() {
    char code[MAX_ID_LEN];
    char sday[8], sst[8], sen[8];
    getInput("Course Code: ", code, sizeof(code));
    if (strlen(code) == 0) return;
    getInput("Day: ", sday, sizeof(sday));
    getInput("Start: ", sst, sizeof(sst));
    getInput("End: ", sen, sizeof(sen));
    int day = atoi(sday), st = atoi(sst), en = atoi(sen);
    removeTimeSlotFromCourse(code, day, st, en);
}

void menuAddPrereq() {
    char course[MAX_ID_LEN], pr[MAX_ID_LEN];
    getInput("Course Code (target): ", course, sizeof(course));
    getInput("Prereq Course Code: ", pr, sizeof(pr));
    addPrereq(course, pr);
}

void menuDeletePrereq() {
    char course[MAX_ID_LEN], pr[MAX_ID_LEN];
    getInput("Course Code (target): ", course, sizeof(course));
    getInput("Prereq Course Code to remove: ", pr, sizeof(pr));
    deletePrereq(course, pr);
}

void menuEnroll() {
    char sid[MAX_ID_LEN], code[MAX_ID_LEN];
    getInput("Student ID: ", sid, sizeof(sid));
    getInput("Course Code: ", code, sizeof(code));
    Student *s = findStudent(sid);
    Course *c = findCourse(code);
    if (!s) { printf("Student not found.\n"); return; }
    if (!c) { printf("Course not found.\n"); return; }
    attemptEnroll(s, c);
}

void menuDrop() {
    char sid[MAX_ID_LEN], code[MAX_ID_LEN];
    getInput("Student ID: ", sid, sizeof(sid));
    getInput("Course Code to drop: ", code, sizeof(code));
    Student *s = findStudent(sid);
    Course *c = findCourse(code);
    if (!s) { printf("Student not found.\n"); return; }
    if (!c) { printf("Course not found.\n"); return; }
    dropCourse(s, c);
}

void sampleData() {
    addCourse("CSE101", "Intro to Programming", 3, 50);
    addTimeSlotToCourse("CSE101", 1, 9, 11);
    addCourse("CSE102", "Data Structures", 3, 40);
    addTimeSlotToCourse("CSE102", 2, 9, 11);
    addCourse("CSE201", "Algorithms", 3, 30);
    addTimeSlotToCourse("CSE201", 1, 11, 13);
    addCourse("PHY101", "Physics I", 3, 20);
    addTimeSlotToCourse("PHY101", 1, 9, 11);

    addPrereq("CSE102", "CSE101");
    addPrereq("CSE201", "CSE102");

    addStudent("251-15-367", "Amey", 3.90f);
    addStudent("251-15-266", "Arup", 3.20f);
    addStudent("251-15-029", "Miraz", 2.80f);
    addStudent("251-15-340", "Sajjad", 3.50f);
    addStudent("251-15-087", "Siam", 3.50f);

    Student *s = findStudent("251-15-367");
    if (s) { strncpy(s->completed[s->completedCount++], "CSE101", MAX_ID_LEN-1); }

    Student *s2 = findStudent("251-15-266");
    if (s2) { strncpy(s2->completed[s2->completedCount++], "CSE101", MAX_ID_LEN-1);
              strncpy(s2->completed[s2->completedCount++], "CSE102", MAX_ID_LEN-1); }
}

void printMenu() {
    printf("University Registration Portal\n");
    printf("1. Load sample data\n");
    printf("2. Load saved data from file\n");
    printf("3. Save data to file\n");
    printf("4. Show all courses\n");
    printf("5. Show all students\n");
    printf("6. Show waitlist\n");
    printf("7. Show action stack (undo)\n");
    printf("8. Add course\n");
    printf("9. Delete course\n");
    printf("10. Add student\n");
    printf("11. Delete student\n");
    printf("12. Add timeslot to course\n");
    printf("13. Remove timeslot from course\n");
    printf("14. Add prerequisite\n");
    printf("15. Delete prerequisite\n");
    printf("16. Enroll student in course\n");
    printf("17. Drop student from course\n");
    printf("18. Process all waitlists\n");
    printf("19. Undo last action\n");
    printf("20. Clear all data (memory)\n");
    printf("0. Exit\n");
    printf("Choice: ");
}

int main() {
    char choiceStr[16];
    loadData();

    while (1) {
        printMenu();
        if (fgets(choiceStr, sizeof(choiceStr), stdin) == NULL) break;

        int choice = atoi(choiceStr);

        if (choice == 1) {
            sampleData();
        }
        else if (choice == 2) {
            loadData();
        }
        else if (choice == 3) {
            saveData();
        }
        else if (choice == 4) {
            showAllCourses();
        }
        else if (choice == 5) {
            showAllStudents();
        }
        else if (choice == 6) {
            showWaitlist();
        }
        else if (choice == 7) {
            showActionStack();
        }
        else if (choice == 8) {
            menuAddCourse();
        }
        else if (choice == 9) {
            menuDeleteCourse();
        }
        else if (choice == 10) {
            menuAddStudent();
        }
        else if (choice == 11) {
            menuDeleteStudent();
        }
        else if (choice == 12) {
            menuAddTimeslot();
        }
        else if (choice == 13) {
            menuRemoveTimeslot();
        }
        else if (choice == 14) {
            menuAddPrereq();
        }
        else if (choice == 15) {
            menuDeletePrereq();
        }
        else if (choice == 16) {
            menuEnroll();
        }
        else if (choice == 17) {
            menuDrop();
        }
        else if (choice == 18) {
            processAllWaitlists();
        }
        else if (choice == 19) {
            undoLastAction();
        }
        else if (choice == 20) {
            clearAllMemory();
            printf("All in-memory data cleared.\n");
        }
        else if (choice == 0) {
            clearAllMemory();
            printf("Exiting.\n");
            return 0;
        }
        else {
            printf("Invalid choice.\n");
        }

        printf("\n");
    }

    clearAllMemory();
    return 0;
}
