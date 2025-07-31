#include <stdio.h>
#define N 3

void fcfs() {
    int bt[N] = {10, 5, 8}, wt[N] = {0}, tat[N], i;
    for (i = 1; i < N; i++) wt[i] = wt[i-1] + bt[i-1];
    for (i = 0; i < N; i++) tat[i] = wt[i] + bt[i];
    for (i = 0; i < N; i++)
        printf("P%d: W=%d T=%d\n", i+1, wt[i], tat[i]);
}

void sjf() {
    int bt[N] = {10, 5, 8}, idx[N] = {0,1,2}, wt[N] = {0}, tat[N], i, j, t;
    for (i = 0; i < N-1; i++)
        for (j = i+1; j < N; j++)
            if (bt[idx[i]] > bt[idx[j]]) { t = idx[i]; idx[i]=idx[j]; idx[j]=t; }
    for (i = 1; i < N; i++) wt[idx[i]] = wt[idx[i-1]] + bt[idx[i-1]];
    for (i = 0; i < N; i++) tat[idx[i]] = wt[idx[i]] + bt[idx[i]];
    for (i = 0; i < N; i++)
        printf("P%d: W=%d T=%d\n", idx[i]+1, wt[idx[i]], tat[idx[i]]);
}

void srjf() {
    int at[N] = {0,1,2}, bt[N] = {7,4,1}, rt[N], wt[N] = {0}, tat[N], ft[N]={0};
    int i, completed = 0, t = 0, min, idx;
    for (i = 0; i < N; i++) rt[i] = bt[i];
    while (completed < N) {
        min = 9999; idx = -1;
        for (i = 0; i < N; i++)
            if (at[i] <= t && rt[i] > 0 && rt[i] < min) { min = rt[i]; idx = i; }
        if (idx == -1) { t++; continue; }
        rt[idx]--;
        if (rt[idx] == 0) { completed++; ft[idx]=t+1; wt[idx]=ft[idx]-bt[idx]-at[idx]; if(wt[idx]<0)wt[idx]=0; }
        t++;
    }
    for (i = 0; i < N; i++) tat[i]=wt[i]+bt[i];
    for (i = 0; i < N; i++) printf("P%d: W=%d T=%d\n", i+1, wt[i], tat[i]);
}

void priority() {
    int bt[N]={10,5,8}, pr[N]={3,1,2}, idx[N]={0,1,2}, wt[N]={0}, tat[N], i, j, t;
    for(i=0;i<N-1;i++)
        for(j=i+1;j<N;j++)
            if(pr[idx[i]]>pr[idx[j]]){t=idx[i];idx[i]=idx[j];idx[j]=t;}
    for(i=1;i<N;i++)wt[idx[i]]=wt[idx[i-1]]+bt[idx[i-1]];
    for(i=0;i<N;i++)tat[idx[i]]=wt[idx[i]]+bt[idx[i]];
    for(i=0;i<N;i++)
        printf("P%d: W=%d T=%d Pr=%d\n", idx[i]+1, wt[idx[i]], tat[idx[i]], pr[idx[i]]);
}

void rr() {
    int bt[N]={10,5,8}, qt=2, rt[N], wt[N]={0}, tat[N], t=0, i, done;
    for(i=0;i<N;i++)rt[i]=bt[i];
    do {
        done=1;
        for(i=0;i<N;i++) {
            if(rt[i]>0) {
                done=0;
                if(rt[i]>qt){t+=qt;rt[i]-=qt;}
                else{t+=rt[i];wt[i]=t-bt[i];rt[i]=0;}
            }
        }
    } while(!done);
    for(i=0;i<N;i++)tat[i]=wt[i]+bt[i];
    for(i=0;i<N;i++)
        printf("P%d: W=%d T=%d\n", i+1, wt[i], tat[i]);
}

int main() {
    int ch;
    printf("1.FCFS 2.SJF 3.SRJF 4.Priority 5.RoundRobin 0.Exit\n");
    do {
        printf("Enter choice: ");
        scanf("%d",&ch);
        if(ch==1) fcfs();
        else if(ch==2) sjf();
        else if(ch==3) srjf();
        else if(ch==4) priority();
        else if(ch==5) rr();
    } while(ch!=0);
    return 0;
}
