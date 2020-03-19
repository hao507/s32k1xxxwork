#include "nxpQueue.h"
#include "string.h"


//???? queueCapacity-????
status initQueue(queue *PQueue,int queueCapacity)
{
    //?????????
    PQueue->pBase = (ElemType *)malloc(sizeof(ElemType)*queueCapacity);
    if(!PQueue->pBase)
    {
        return ERROR;
    }

    PQueue->front = 0; //??????,?????0
    PQueue->rear = 0; //??????,?????0
    PQueue->maxSize = queueCapacity;

    return OK;
}

//????
void destroyQueue(queue *PQueue)
{
    free(PQueue);  //?????????????
    PQueue = NULL;    //??????????NULL,???????
}

//????
void clearQueue(queue *PQueue)
{
    PQueue->front = 0; //?????0
    PQueue->rear = 0; //?????0
}

//????????
status isEmpityQueue(queue *PQueue)
{
    if( PQueue->front == PQueue->rear )  //??==??,????
        return TRUE;

    return FALSE;
}

/*
 *??????,�??�?�??�??????????,??front==rear,
 *?????,?????�??�??�??�?
 *??????,?3????????:
 *(1)??????????�??�??�??�?(???/???????�??�/�??�)
 *(2)??????,??????????????
 *(3)????????,?????????????????????�??�???,
 *?�??�???:(PQueue->rear+1)%MAX_SIZE == PQueue->front?
 *  ???????3??????
 */
//????????
status isFullQueue(queue *PQueue)
{
    if( (PQueue->rear+1)%PQueue->maxSize == PQueue->front )  //???
        return TRUE;

    return FALSE;
}

//??????
int getQueueLen(queue *PQueue)
{
    //?????,?????????????,??????????????,?%
    return (PQueue->rear - PQueue->front + PQueue->maxSize)%PQueue->maxSize;
}

//????? [??????:????????] element-?????
status enQueue(queue *PQueue,ElemType element)
{
#if 1
    if(isFullQueue(PQueue)==TRUE)
    {
//        printf("????,????????,??????!\n");
//        return FALSE;
        ElemType elemTemp;
        deQueue(PQueue, &elemTemp);
    }

    //?????????
//    PQueue->pBase[PQueue->rear] = element;
    memcpy(&PQueue->pBase[PQueue->rear], &element, sizeof(ElemType));
    PQueue->rear = (PQueue->rear+1) % PQueue->maxSize; //?rear????????
#endif
    return TRUE;
}

//?????,????????? [??????:????????]
status deQueue(queue *PQueue,ElemType *pElement)
{
    //??????,???false
    if(isEmpityQueue(PQueue)==TRUE)
    {
//        printf("????,????!\n");
        return FALSE;
    }

//    *pElement = PQueue->pBase[PQueue->front];       //????
    memcpy(pElement, &PQueue->pBase[PQueue->front],sizeof(ElemType));
    PQueue->front = (PQueue->front+1) % PQueue->maxSize; //??????

    return TRUE;
}

//????
void queueTraverse(queue *PQueue)
{
    int i = PQueue->front;           //??????
    while(i != PQueue->rear)     //??????rear??,???
    {
//        printf("%d  ", PQueue->pBase[i]);
        i = (i+1) % PQueue->maxSize;              //??????
    }
    printf("\n");
}


//---------------------------------------------------------------------
//???? queueCapacity-????
status initQueueCan(queueCan *PQueue,int queueCapacity)
{
    //?????????
    PQueue->pBase = (ElemTypeCan *)malloc(sizeof(ElemTypeCan)*queueCapacity);
    if(!PQueue->pBase)
    {
        return ERROR;
    }

    PQueue->front = 0; //??????,?????0
    PQueue->rear = 0; //??????,?????0
    PQueue->maxSize = queueCapacity;

    return OK;
}

//????
void destroyQueueCan(queueCan *PQueue)
{
    free(PQueue);  //?????????????
    PQueue = NULL;    //??????????NULL,???????
}

//????
void clearQueueCan(queueCan *PQueue)
{
    PQueue->front = 0; //?????0
    PQueue->rear = 0; //?????0
}

//????????
status isEmpityQueueCan(queueCan *PQueue)
{
    if( PQueue->front == PQueue->rear )  //??==??,????
        return TRUE;

    return FALSE;
}


status isFullQueueCan(queueCan *PQueue)
{
    if( (PQueue->rear+1)%PQueue->maxSize == PQueue->front )  //???
        return TRUE;

    return FALSE;
}

//??????
int getQueueLenCan(queueCan *PQueue)
{
    //?????,?????????????,??????????????,?%
    return (PQueue->rear - PQueue->front + PQueue->maxSize)%PQueue->maxSize;
}

//????? [??????:????????] element-?????
status enQueueCan(queueCan *PQueue,ElemTypeCan element)
{
#if 1
    if(isFullQueueCan(PQueue)==TRUE)
    {
//        printf("????,????????,??????!\n");
//        return FALSE;
        ElemTypeCan elemTemp;
        deQueueCan(PQueue, &elemTemp);
    }

    //?????????
//    PQueue->pBase[PQueue->rear] = element;
    memcpy(&PQueue->pBase[PQueue->rear], &element, sizeof(ElemTypeCan));
    PQueue->rear = (PQueue->rear+1) % PQueue->maxSize; //?rear????????
		
#endif
    return TRUE;
}

//?????,????????? [??????:????????]
status deQueueCan(queueCan *PQueue,ElemTypeCan *pElement)
{
    //??????,???false
    if(isEmpityQueueCan(PQueue)==TRUE)
    {
//        printf("????,????!\n");
        return FALSE;
    }

//    *pElement = PQueue->pBase[PQueue->front];       //????
    memcpy(pElement, &PQueue->pBase[PQueue->front],sizeof(ElemTypeCan));
    PQueue->front = (PQueue->front+1) % PQueue->maxSize; //??????

    return TRUE;
}

//????
void queueTraverseCan(queueCan *PQueue)
{
    int i = PQueue->front;           //??????
    while(i != PQueue->rear)     //??????rear??,???
    {
//        printf("%d  ", PQueue->pBase[i]);
        i = (i+1) % PQueue->maxSize;              //??????
    }
    printf("\n");
}