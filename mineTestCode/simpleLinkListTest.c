#include "stdio.h"
#include "myHeadFile.h"
#include "malloc.h"
#include "stdlib.h"

typedef struct _simpleList{
    int elem;//such as student num, it is unique
    float score;
    struct _simpleList *next;
}*simpleList, simpleListNode;

int createLinkList(
    OUT simpleList* newList)
{
    simpleList p = NULL;
    if (NULL == newList)
        ERROUT("newList is NULL\n");
    p = (simpleList)malloc(sizeof(simpleListNode));
    if (NULL == p)
        ERROUT("createListNode failed\n");
    p->elem = 0;
    p->score = 0;
    p->next = NULL;

    *newList = p;
    return 0;
Err:
    return -1;
}

int createLinkNode(
    IN int elem,
    IN float score,
    OUT simpleListNode** linkNode)
{
    simpleListNode* node = NULL;
    if (NULL == linkNode)
        ERROUT("pointer is NULL \n");
    node = (simpleListNode*)malloc(sizeof(simpleListNode));
    if (NULL == node)
        ERROUT("create linkNode failed\n");
    node->elem = elem;
    node->score = score;
    node->next = NULL;

    *linkNode = node;
    return 0;
Err:
    return -1;
}

int listLenGet(
    IN simpleList list,
    OUT int* listLen)
{
    int len = 0;
    simpleList p = list;
    if ((NULL == list) || (NULL == listLen))
        ERROUT("pointer is NULL \n");
    while (p->next)
        len++;

    *listLen = len; 
    return 0;
Err:
    return -1;
}

int insertNode(
    IN simpleList list, 
    IN simpleListNode* student)
{
    simpleList p = list;
    if ((NULL == list) || (NULL == student))
        ERROUT("pointer is NULL\n");
    if (checkStudentelem(list, student->elem))
        ERROUT("thiere is the same elem existing\n");
    while (p->next)
        p = p->next;
    p->next = student;
    return 0;
Err:
    return -1;
}

int delelemNode(
    IN simpleList list, 
    IN int elem,
    OUT int* deleteCount)
{
    simpleList p = list;
    simpleList q = NULL;
    int count = 0;
    if (NULL == list)
        ERROUT("pointer is NULL\n");
    while (p->next){
        if (p->next->elem != elem)
            p = p->next;
        else {
            q = p->next;
            p->next = q->next;
            free(q);
            count++;
        }
    }
    if (deleteCount)
        *deleteCount = count;
    return 0;
Err:
    return -1;
}

int delScoreNode(
    IN simpleList list, 
    IN float score,
    OUT int* delCount)
{
    simpleList p = list;
    simpleList q = p->next;
    int count = 0;
    if ((NULL == list) || (NULL == delCount))
        ERROUT("pointer is NULL\n");
    
    while (p->next){
        if (q->score== score){
            count++;
            p->next = q->next;
            free(q);
            q = p->next;
            continue;
        } else
            p = p->next;
    }

    *delCount = count;
    return 0;
Err:
    return -1;
}

int changeelemNodeScore(
    IN simpleList list, 
    IN int elem, 
    IN float score)
{
    simpleList p = list;
    simpleList q = p->next;
    if (NULL == list)
        ERROUT("pointer is NULL\n");
    while (p->next){
        if (q->elem == elem){
            q->score = score;
            return 0;
        }
        p = p->next;
        q = p;
    }
Err:
    return -1;//means  not found the elem
}

int checkStudentelem(
    IN simpleList list, 
    IN int elem)
{
    simpleList p = list;
    simpleList q = p->next;
    if (NULL == list)
        ERROUT("pointer is NULL\n");
    while (p->next){
        if (q->elem == elem)
            return -1;
        p = p->next;
        q = p->next;
    }

    return 0;
Err:
    return -1;
}

int elemNodeScoreGet(
    IN simpleList list, 
    IN int elem,
    OUT float* nodeScore)
{
    simpleList p = list;
    simpleList q = p->next;
    if ((NULL == list) || (NULL == nodeScore))
        ERROUT("pointer is NULL\n");
    while (p->next){
        if (q->elem == elem){
            *nodeScore = q->score;
            return 0;
        }
    }
Err:
    return -1;
}

int printList(
    IN simpleList list)
{
    simpleList p = list;
    simpleList q = p->next;
    if (NULL == list)
        ERROUT("pointer is NULL\n");
    system("clear");
    printf("****************************************\n");
    while (p->next){
        printf("student elem = %d, score = %.2f\n", q->elem, q->score);
        p = p->next;
        q = p->next;
    }
    printf("****************************************\n");
Err:
    return -1;
}


int main()
{
    simpleList testHead = NULL;
    simpleListNode* testNode1 = NULL;
    simpleListNode* testNode2 = NULL;
    createLinkList(&testHead);
    createLinkNode(1, 90, &testNode1);
    createLinkNode(1, 90, &testNode2);
    insertNode(testHead, testNode1);
    insertNode(testHead, testNode2);
    changeelemNodeScore(testHead, 1, 80);
    //delelemNode(testHead, testNode1->elem, NULL);
    printList(testHead);
    return 0;
}
