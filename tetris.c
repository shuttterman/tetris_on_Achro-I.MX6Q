#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include "dot_matrix_font.h"

#define MAX_DOT_MATRIX 10
#define MAX_BUTTON 9

unsigned char concrete[MAX_DOT_MATRIX+2];
unsigned char frame[MAX_DOT_MATRIX];
unsigned char sw[MAX_BUTTON];
int dot_matrix, push_switch, led, sseg;
int block_type;
int next_block_type;
int block_pose;
int i;
int x, y;
int level=1;
int clear_line_num=0;
int harddown;
int curr_sseg_status;

void display(void);
void oneround();
void *input_thread_run(void *data);
void *timer_thread_run(void *data);
void for_oneround_init(void);
int falldown(void);
void clear_line(void);
int win(void);
int gameover(void);
void write_led(void);
void write_sseg(int num);
void cleanup(void);
void user_signal1(int sig) { printf("\n---Exit the game---\n"); cleanup(); exit(0);}

int main(int argc, char *argv[]) {
    pthread_t input_thread, timer_thread;
    int thread_err;

    dot_matrix = open("/dev/csemad_dot_matrix", O_RDWR);
    push_switch = open("/dev/csemad_push_switch", O_RDWR);
    led = open("/dev/csemad_led", O_RDWR);
    sseg = open("/dev/csemad_sseg", O_RDWR);

    if(dot_matrix == -1 || push_switch == -1 || led == -1 || sseg == -1) {
        printf("fail to open\n");
        return -1;
    }
    concrete[0] = 0x00;
    memcpy(concrete+1, dot_matrix_blank, MAX_DOT_MATRIX);
    concrete[10] = 0x7f;
    srand(time(NULL));

    if((thread_err = pthread_create(&input_thread, NULL, input_thread_run, NULL))) {
        printf("Thread Err = %d", thread_err);
        return -1;
    }
    if((thread_err = pthread_create(&timer_thread, NULL, timer_thread_run, NULL))) {
        printf("Thread Err = %d", thread_err);
        return -1;
    }

    (void)signal(SIGINT, user_signal1);

    next_block_type = rand()%4;
    write_sseg(clear_line_num);

    system("clear");
    printf("\t\n\n\n\nTETRIS ON BOARD\n");

    while(!(gameover() || win())) {
        for_oneround_init();
        write_led();
        oneround(); // start one round
    }

    cleanup();
    return 0;
}

void display(void) {
    unsigned char tmp[MAX_DOT_MATRIX];
    for(i=0 ; i<MAX_DOT_MATRIX ; i++) tmp[i] = concrete[i] | frame[i];
    write(dot_matrix, tmp, MAX_DOT_MATRIX);
}

void oneround() {
    int tmp; // for over left side ratate correction
    while(1) {
        tmp=0;
        memcpy(frame, dot_matrix_blank, MAX_DOT_MATRIX); // frame init
        for(i=0 ; i<3 ; i++) { //correct location scatter
            frame[i+y]=block[block_type][block_pose%4][i];
            if((frame[i+y]<<x) > 0x00)frame[i+y]<<=x; // right side limit
            if(frame[i+y] >= 0x80)tmp=1; // over left side rotate correction
        }
        while(tmp--)x--; // over left side rotate correction
        if(!falldown()) { // falldown 1:ok 0:not ok
            for(i=0 ; i<MAX_DOT_MATRIX ; i++) concrete[i] |= frame[i]; // falldown end and frame, concrete merge
            display();
            break;
        }
        display();
    }
    clear_line(); // clear line for complete line
}

void *input_thread_run(void *data) { // input thread
    int j;
    int unable; //for left side limit
    while(1) {
        unable = 0;
        read(push_switch, sw, 9); // reading switch state
        if(sw[1] == 1)block_pose++; // rorate
        else if(sw[3] == 1 ) { // to the left
            for(j=0 ; j<MAX_DOT_MATRIX ; j++) {
                if((frame[j]<<1) >= 0x80)unable=1;
            }
            if(!unable)x++;
        } 
        else if(sw[5] == 1) { // to the right
            if(!unable)x--;
        }
        else if(sw[4] == 1) { // soft down
            y++;
        }
        else if(sw[7] == 1) { // hard down
            harddown=1;
            while(harddown) {
                y++;
                usleep(80000);
            }
        }
        else if(sw[6] == 1) { // show clear_line_num
            curr_sseg_status = 0;
            write_sseg(clear_line_num);
        }
        else if(sw[8] == 1) { // show level
            curr_sseg_status = 1;
            write_sseg(level);
        }
        usleep(130000);
    }
}

void *timer_thread_run(void *data) {
    float alpha = 0.3;
    level = 1;
    while(1) {
        usleep((int)(2600000/level*alpha));
        y++;
    }
}

void for_oneround_init(void) { //init
    memcpy(frame, dot_matrix_blank, MAX_DOT_MATRIX);
    block_type = next_block_type; // create new block
    next_block_type = rand()%4;
    block_pose = 0;
    x=2;
    y=0;
    harddown = 0;
}

int falldown(void) { 
    for(i=0 ; i<MAX_DOT_MATRIX ; i++) {
        if((frame[i] & concrete[i+1]))return 0;
    }
    return 1;
}

void clear_line(void) {
    int i, j;
    for(i=MAX_DOT_MATRIX-1 ; i>=0 ; i--) {
        if(concrete[i] == 0x7f) {
            clear_line_num++;
            level = (clear_line_num/7)+1;
            if(curr_sseg_status) write_sseg(level);
            else write_sseg(clear_line_num);
            for(j=i ; j>0 ; j--) concrete[j] = concrete[j-1];
            i++;
            continue;
        }
    }
}

void write_led() {
    unsigned char tmp;
    switch(next_block_type) {
        case 0 :
            tmp = 0xC6;
            break;
        case 1 :
            tmp = 0x47;
            break;
        case 2 :
            tmp = 0x4E;
            break;
        case 3 :
            tmp = 0x0E;
            break;
        default :
            break;
    }
    write(led, &tmp, 1);
}

void write_sseg(int num) {
    unsigned char temp[2];
    int tmp, a;
    tmp = (num % 100 - num % 10) *16/10;
    temp[0] = num%10 + tmp;
    a = num/100;
    tmp = (a%100 - a%10)*16/10;
    temp[1] = tmp+a%10;
    write(sseg, temp, sizeof(temp));
}

void cleanup(void) {
    write(dot_matrix, dot_matrix_blank, MAX_DOT_MATRIX);
    write(led, &dot_matrix_blank[0], 1);
    write(sseg, dot_matrix_blank, 2);
    close(dot_matrix);
    close(push_switch);
    close(led);
    close(sseg);
}

int gameover(void) {
    unsigned char tmp = 0xff;
    if(concrete[0] == 0x00) return 0;
    else {
        write(led, &tmp, 1);
        write(dot_matrix, dot_matrix_full, MAX_DOT_MATRIX);
        system("clear");
        printf("\t\t\n\n\n\n\nGAME OVER\n");
        usleep(1000000);
        return 1;
    }
}

int win(void) {
    unsigned char tmp = 0xff;
    if(clear_line_num <= 100) return 0;
    else {
        write(led, &tmp, 1);
        write(dot_matrix, dot_matrix_full, MAX_DOT_MATRIX);
        system("clear");
        printf("\t\t\n\n\n\n\n\nYOU ARE WIN\n");
        usleep(1000000);
        return 1;
    }
}
