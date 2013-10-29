/*-------------------------------------------------------------------------------
// dynamic.cc
// Defines the main() entry point for the application.
// Contains functions for reading inputs and generating output.
// Taylor Sprinkle
//--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>


#define EMPTY -1

class ROB {                     //ROB class instance
private:

public:
    int IF;
    int ID;
    int IS;
    int EX;
    int WB;

    void mROB();                

    int stg;
    int tg;
    int opTp;
    int opSt;
    int cmpltd;

    int dest;
    int src1;
    int src2;

};

class DQ {                      //Dispatch Queue instance
private:
public:
    int rBTg;
    int nID;
    void mDQ(int N);            //constructor
};


class EQ {                      //EQueue instance
private:
public:
    int tg;
    int cycl;
    void mEQ(int N);            //constructor

};

class SchedQ{                   //Scheduling Queue instance
private:
public:
    int rBTg;
    int rdy1;
    int tag1;
    int rdy2;
    int tag2;
    SchedQ *nxt;
    SchedQ *prv;
};

class RF{                       //Register File instance
private:
public:
    int nRF;
    int tg;
    void mRF();                 //constructor
};

unsigned int numI = 0;
unsigned int numC = 0;
unsigned int rH = 0;
unsigned int rT = 0;
unsigned int dH = 0;
unsigned int dT = 0;
unsigned int dSz = 0;
unsigned int sSz = 0;
unsigned int countr = 0;
ROB** rB = 0;
DQ** DQueue = 0;
EQ** EQueue = 0;
RF** RFile = 0;
SchedQ* schdH, *schdT;

void ROB::mROB(){                           //ROB constructor

    rB = new ROB*[1024];
    int i = 0;
    while(i < 1024){
        rB[i] = new ROB;
        i++;
    }
}

void DQ::mDQ(int N) {                       //Dispatch Queue constructor

    DQueue = new DQ*[N*2];
    int i = 0;
    while(i < 2*N){
        DQueue[i] = new DQ;
        DQueue[i]->rBTg = EMPTY;
        DQueue[i]->nID = EMPTY;
        i++;
    }
}

void EQ::mEQ(int N) {                       //EQueue constructor

    EQueue = new EQ*[N*5];
    int i = 0;
    while(i < N*5){
        EQueue[i] = new EQ;
        EQueue[i]->tg = EMPTY;
        i++;
    }
}

void RF::mRF(){                             //Register File constructor

    RFile = new RF*[128];
    int i = 0;
    while(i < 128){
        RFile[i] = new RF;
        RFile[i]->nRF = 1;
        i++;
    }
}

SchedQ* mkN(SchedQ* nxt, SchedQ* prv){      //Scheduling Queue constructor
    SchedQ* sWn;
    sWn = new SchedQ;

    sWn->nxt = nxt;
    sWn->prv = prv;

    if(sWn->nxt == NULL) schdT = sWn;
    else nxt->prv = sWn;
    if(sWn->prv == NULL) schdH = sWn;
    else prv->nxt = sWn;

    sSz++;
    return sWn;
}

void upStage(int newStage, int rBTg){       //upstage procedure
    rB[rBTg]->stg = newStage;

    switch (newStage) {
        case 1: rB[rBTg]->ID = numC;
        case 2: rB[rBTg]->IS = numC;
        case 3: rB[rBTg]->EX = numC;
        case 4: rB[rBTg]->WB = numC;
    }
    return;
}

void dlN(SchedQ* sWn){                      //delete node from Schedule
    SchedQ* prv = sWn->prv;
    SchedQ* nxt = sWn->nxt;

    if(prv == NULL) schdH = nxt;
    else prv->nxt = nxt;
    --sSz;
    if(nxt == NULL) schdT = prv;
    else nxt->prv = prv;

    delete(sWn);

    return;
}

int aRb(int stg, int opTp, int opSt, int cmpltd, int dest, int src1, int src2){     //add info to ROB
    int temp;

    rB[rT]->stg = stg;
    rB[rT]->opTp = opTp;
    rB[rT]->cmpltd = cmpltd;
    rB[rT]->opSt = opSt;
    rB[rT]->dest = dest;
    rB[rT]->src1 = src1;
    rB[rT]->src2 = src2;
    rB[rT]->tg = countr;
    rB[rT]->IF = numC;
    temp = rT;
    rT++;
    if(rT == 1024) rT = 0;
    return temp;
}

void issue(int S, int N){
    int i = 0;

    int k = 0;
    SchedQ** tmpSQueue;                                                         //*sWnHld pointer
    SchedQ* sWnHld = schdH;

    tmpSQueue = new SchedQ*[N];

    while(sWnHld != NULL){
        if((sWnHld->rBTg >= 0) && (sWnHld->rdy1 == 1) && (sWnHld->rdy2 == 1 && i < N)){
            tmpSQueue[i] = sWnHld;
            i++;

        }
        sWnHld = sWnHld->nxt;
    }
    for(int j = 0; j < N*5; j++){
        if((EQueue[j]->tg < 0) && (k < i)){                                     //Search empty slot for EQueue and TempList if not empty
            upStage(3, tmpSQueue[k]->rBTg);

            EQueue[j]->tg = tmpSQueue[k]->rBTg;                                 //set tg to empty an EQueue slot

            EQueue[j]->cycl = rB[tmpSQueue[k]->rBTg]->opTp + 1;
            if (EQueue[j]->cycl == 3) EQueue[j]->cycl = EQueue[j]->cycl + 2;

            dlN(tmpSQueue[k]);                                                  //invalidate instruction in Window
            k++;
        }
    }
    delete(tmpSQueue);
    return;
}

void fakeRetire(){                          //remove ROB

    while(rB[rH]->stg == 4 && rH != rT){
        int tg = rB[rH]->tg;
        int opTp = rB[rH]->opTp;
        int dest = rB[rH]->dest;
        int src1 = rB[rH]->src1;
        int src2 = rB[rH]->src2;
        int IF = rB[rH]->IF;
        int ID = rB[rH]->ID;
        int IS = rB[rH]->IS;
        int EX = rB[rH]->EX;
        int WB = rB[rH]->WB;
        int ECycle = opTp + 1;
        if (ECycle == 3) ECycle = ECycle + 2;
    //Print scope formated output
        printf("%d fu{%d} src{%d,%d} dst{%d} IF{%d,%d} ID{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,1}\n",tg, opTp, src1, src2, dest, IF, (ID - IF), ID, (IS - ID), IS, (EX - IS), EX, ECycle, WB);

        rH++;
        while (rH == 1024) {
            rH = 0;
            break;
        }
    }
    return;
}

void dispatch(unsigned int S, unsigned int N){
    SchedQ* sWnTmp;
    int* tmpDQueue;
    unsigned int  i = 0;
    unsigned int dIndex, listCount;
    int src1, src2;

    dIndex = dH;
    listCount = 0;
    tmpDQueue = new int[N];
    while(i < 2*N){
        if(dIndex == 2*N) dIndex = 0;
        if(DQueue[dIndex]->nID == 1 && (sSz + listCount < S)){
            DQueue[dIndex]->nID = EMPTY;
            tmpDQueue[listCount] = DQueue[dIndex]->rBTg;
            DQueue[dIndex]->rBTg = EMPTY;
            listCount = listCount + 1;
        }
        dIndex = dIndex + 1;
        if(listCount == N) break;
        i++;
    }
    i = 0;
    for(; (sSz < S) && (listCount > 0); ){
        sWnTmp = mkN(NULL, schdT);
        src1 = rB[tmpDQueue[i]]->src1;
        upStage(2, tmpDQueue[i]);
        sWnTmp->rBTg = tmpDQueue[i];
        src2 = rB[tmpDQueue[i]]->src2;
                                                                                //src1 or src2 does not exist if -1

        if((src1 == EMPTY) || (RFile[src1]->nRF) || (!RFile[src1]->nRF && rB[RFile[src1]->tg]->cmpltd)){
            sWnTmp->rdy1 = 1;
        }
        else{
            sWnTmp->rdy1 = 0;
            sWnTmp->tag1 = RFile[src1]->tg;
        }

        if((src2 == EMPTY) || (RFile[src2]->nRF) || ((!RFile[src2]->nRF) && (rB[RFile[src2]->tg]->cmpltd))) {
            sWnTmp->rdy2 = 1;
        }
        else{
            sWnTmp->rdy2 = 0;
            sWnTmp->tag2 = RFile[src2]->tg;
        }

        if(rB[tmpDQueue[i]]->dest >= 0){
            RFile[rB[tmpDQueue[i]]->dest]->nRF = 0;
            RFile[rB[tmpDQueue[i]]->dest]->tg = tmpDQueue[i];
        }
        dH = dH + 1;
        if(dH == (2*N)) {
            dH = 0;
        }
        listCount = listCount - 1;
        dSz = dSz - 1;

        i = i+ 1;
    }
    return;
}

void execute(unsigned int S, unsigned int N){
    unsigned int i = 0;
    int rTg;
    SchedQ* hold;

    while(i< N*5){
        hold = schdH;
        --EQueue[i]->cycl;                                                      //decrement cycle count by 1
        rTg = EQueue[i]->tg;                                                    //rTg <0 does not exist
        if((rTg >= 0) && (EQueue[i]->cycl == 0)){                               //check if execute done
            rB[rTg]->cmpltd = 1;
            numI++;
            EQueue[i]->tg = EMPTY;                                              //delete instruc from list
            upStage(4, rTg);

            while(hold != NULL){
                if(!hold->rdy1 && hold->tag1 == rTg) hold->rdy1 = 1;
                if(!hold->rdy2 && hold->tag2 == rTg) hold->rdy2 = 1;
                hold = hold->nxt;
            }
            if((rB[rTg]->dest >= 0) && (rTg == RFile[rB[rTg]->dest]->tg) && (!RFile[rB[rTg]->dest]->nRF)){
                    RFile[rB[rTg]->dest]->nRF = 1;
            }
        }
        i++;
    }
    return;
}

void Fetch(unsigned int S, unsigned int N, char* filename){
    unsigned int i = 0;
    unsigned int j;
    int opTp, dest, src1, src2, tg;
    unsigned int PC;
    char memreg[256];
    unsigned int oldPC = 0;
    FILE* file;

    if((file = fopen(filename, "r")) == NULL){
        printf("Could not open the file.");
        exit(1);
    }
    do{
        if((i == N) || (dSz == 2*N)){                                         //if DQueue or bandwidth full
            numC++;
            fakeRetire();
            execute(S, N);
            issue(S, N);
            dispatch(S, N);
            j = 0;
            while(j <2*N){
                if(DQueue[j]->nID == 0){
                    DQueue[j]->nID = 1;
                    upStage(1, DQueue[j]->rBTg);
                    i = i - 1;
                }
                j = j+ 1;
            }
        }
        if(dSz < 2*N){                                          //Read file
            if(i < N){
                fscanf(file,"%x", &PC);
                fgetc(file);
                fscanf(file, "%d", &opTp);
                fgetc(file);
                fscanf(file, "%d", &dest);
                fgetc(file);
                fscanf(file, "%d", &src1);
                fgetc(file);
                fscanf(file, "%d", &src2);
                fgetc(file);
            	fscanf(file, "%s", memreg);
            	fgetc(file);
                if(PC == oldPC) break;
                tg = aRb(0, opTp, 0, 0, dest, src1, src2);
                countr = countr + 1;
                DQueue[dT]->rBTg = tg;
                DQueue[dT]->nID = 0;
                dT = dT + 1;
                if(dT==2*N)dT = 0;
                dSz = dSz + 1;
                i = i +1;
                oldPC = PC;
            }
        }
    } while(!feof(file));

    while(rH != rT){
        numC++;
        fakeRetire();
        execute(S, N);
        issue(S, N);
        dispatch(S, N);
        j = 0;
        while(j < 2*N){
            if(DQueue[j]->nID == 0){
                DQueue[j]->nID = 1;
                upStage(1, DQueue[j]->rBTg);
            }
            j = j + 1;
        }
    }
    return;
}

int main (int argc, char *argv[]){              //main that runs simulator
    int S, N, bs, L1_size, L1_assoc,L2_size,L2_assoc;
    char* file;
    ROB r;
    DQ d;
    EQ e;
    RF f;

    S = (unsigned int)atoi(argv[1]);
    N = (unsigned int)atoi(argv[2]);
    file = argv[8];
    r.mROB();
    d.mDQ(N);
    e.mEQ(N);
    f.mRF();
    Fetch(S,N,file);
    //Print Input
        printf("CONFIGURATION\n");
        printf(" superscalar bandwidth (N) = %d\n", N);
        printf(" dispatch queue size (2*N) = %d\n", 2*N);
        printf(" schedule queue size (S)   = %d\n", S);
    //Print Data
        printf("RESULTS\n");
        printf(" number of instructions\t= %d\n", numI);
        printf(" number of cycles\t= %d\n", numC);
        printf(" IPC\t\t\t= %.2f\n", ((float)numI/(float)numC));
    return(0);
}
