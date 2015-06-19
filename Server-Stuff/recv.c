//
//  recv.c
//  NSG-wireless-MultiCast
//
//  Created by Huzaifa Kamran on 6/19/15.

//
#define SHELL "/bin/sh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


#define MCAST_PORT 1532
#define MCAST_GROUP "224.0.67.67"
#define MSGBUFSIZE 256


typedef unsigned char byte;
typedef enum { false, true } bool;

static double WEIGHT = 0.5;
static double PROBING_RATE = 0.3;
/* -------------------- */

/* CONSTANTS */
static  int E_THROUGHPUT = 0;
 static  int PER = 1;
 static  double rates[] = {1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 52};
/* -------------------- */

 static int current_Rate = 4;
 static int prev_Rate = 0;
 static int leader = 0;

 static int FIXED_RATE = 0;

 static long total_sent = 0;
 static long total_received = 0;
 bool probed = false;

 double rate_table[12][2];

static char pkts_Sent[] = "iptables -t mangle --list -v | awk '{if ($3 == \"TTL\") print $ 1}'";
 static  char reset[] = "iptables -t mangle -F";
 static  char mangle[] = "iptables -t mangle -A OUTPUT -j TTL -d 224.0.67.67 --ttl-set ";
 static  char fpath[] = "/sys/kernel/debug/ieee80211/phy0/ath9k/mcast_rate";

char* execute_shell_command(char* command,char* output)
{
    
    int status=0;
    pid_t pid;
    
    pid = fork ();
    if (pid == 0)
    {
        /* This is the child process.  Execute the shell command. */
        execl (SHELL, "bash", "-c", command, NULL);
        ssize_t bufsize = 0; // have getline allocate a buffer for us
        getline(&output, &bufsize, stdin);
        _exit (EXIT_FAILURE);
    }
    else if (pid < 0) status = -1;
    else
            if (waitpid (pid, &status, 0) != pid)
            status = -1;
    
    if (status != -1) return output;
    else return NULL;

}

int count_sent()
{
    int s = 0;
    char* ret;char* temp;
    s = (int)(execute_shell_command(pkts_Sent,ret));
    
    execute_shell_command("iptables -t mangle -Z",temp);
    return s;
}

void set_Rate(int rate)
{
    if (FIXED_RATE != 0)
    {
        current_Rate = FIXED_RATE;
        rate = FIXED_RATE;
    }
    
    if(prev_Rate - current_Rate != 0)
    {
        char temp[sizeof("echo ") + sizeof(rate) + sizeof(" > ") + sizeof(fpath)+1];
        sprintf(temp, "echo %d > %s", rate,fpath);
        char* t;
        execute_shell_command(temp,t);
        int ttl = (2*rate)+5;
        int write_value = ttl | (leader << 5);
        total_sent += count_sent();
        execute_shell_command(reset,t);
        char mangleUpdated[sizeof(mangle) + sizeof(write_value) + 1];
        sprintf(mangleUpdated,"%s%i",mangle,write_value);
        execute_shell_command((mangleUpdated),t);
        char LastOne[sizeof(prev_Rate) + sizeof(" NEW Rate: ") + sizeof(current_Rate) +1];
        sprintf(LastOne,"%i%s%i",prev_Rate," NEW Rate: ",current_Rate);
        printf("%s\n",LastOne);
    }
}

void find_Rate()
{
    /* Probing
     Don't probe if previous turn was probed
     */
    
    time_t t;
    srand((unsigned)(&t));
    if (rand() < PROBING_RATE)
    {
        /*for (int i = current_Rate + 1; i < 12; i++)
         {
         if (rate_table[i][E_THROUGHPUT] == 0)
         {
         current_Rate = i;
         break;
         }
         }
         */
        if (probed == false)
        {
            double chance = 0.75;
            for (int i = current_Rate - 2; i < current_Rate + 2; i++)
            {
                if (i == current_Rate || i < 1 || i > 11)
                {
                    continue;
                }
                if (rand() > chance)
                {
                    current_Rate = i;
                    probed = true;
                    break;
                }
                chance -= 0.25;
            }
        }
    }
    else
    {
        probed = false;
        for (int i=0; i < 12; i++)
        {
            if (rate_table[i][E_THROUGHPUT] > rate_table[current_Rate][E_THROUGHPUT])
            {
                current_Rate = i;
            }
        }
    }
    
    set_Rate(current_Rate);
}

void updateRateTable(int* mcs_feedback)
{
    int sent = count_sent();
    total_sent += sent;
    
    for (int i=0; i<sent; i++)
    {
        if (FIXED_RATE == 0)
            printf("%i ",current_Rate);
    }
    double per = 0;
    
    double lost = sent - mcs_feedback[current_Rate];
    
    if (sent < mcs_feedback[current_Rate])
    {
        total_sent += mcs_feedback[current_Rate] - sent;
    }
    
    //total_received += mcs_feedback[current_Rate];
    
    for (int i=0; i<12; i++)
    {
        total_received += mcs_feedback[i];
    }
    
    if (lost < sent && lost >= 0)
        per = lost/(double)sent;
    else
        per = 0;
    
    rate_table[current_Rate][PER] = WEIGHT*rate_table[current_Rate][PER]
    + ((double)(1 - WEIGHT))*(per);
    
    // System.out.println(mcs_feedback[current_Rate] + " / " + sent);
    
    rate_table[current_Rate][E_THROUGHPUT] = rates[current_Rate]*(1 - rate_table[current_Rate][PER]);
    // System.out.println(Arrays.deepToString(rate_table));
}

int main(int argc, char *argv[])
{
//    Runtime.getRuntime().addShutdownHook(new Thread() {
//        @Override
//        public void run() {
//            double error_rate = (double)(total_sent - total_received)/(double)(total_sent);
//            
//            System.out.println("Throughput: " + 8*(1-error_rate)*(total_sent*1500)/(60*1024*1024) + " Mbps");
//            System.out.println("Error_Rate: " + 100*error_rate + " %");
//            System.out.println("Exiting...");
//        }
//    });
    if (argc > 0)
    {
        FIXED_RATE = (int)(argv[0]);
        printf("%s%i","Rate Fixed to ",FIXED_RATE);
    }
    
    
    struct sockaddr_in addr;
    int fd,nbytes,addrlen;
    char msgbuf[MSGBUFSIZE];
   // struct ip_mreq mreq;
    char *message;
    
    /* create what looks like an ordinary UDP socket */
    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        exit(1);
    }
    
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(MCAST_GROUP);
    addr.sin_port=htons(MCAST_PORT);
    
    //byte receiveData[48];
        
    set_Rate(current_Rate);
    
    /* bind to receive address */
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

        while(true)
        {
            
            addrlen=sizeof(addr);
            if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                                 (struct sockaddr *) &addr,&addrlen)) < 0) {
                perror("recvfrom");
                exit(1);
        }
            puts(message);
            int myArray[nbytes/4];
            
            if (nbytes%4 == 0) {
                for (int i = 0; i < nbytes; i+=4) {
                    
                    char tempArray[5];
                    tempArray[4]='\0';
                    
                    for (int j = 0; j < 4; j++) {
                        tempArray[j] = message[i+j];
                    }
                    
                    myArray[i/4] = atoi(tempArray);
                    
                }
                
            }else{
            
                printf("%s\n%s%d\n","nbytes is NOT a multiple of 4","Size of NBYTES is: ",nbytes);
            }
            

            
            struct sockaddr_in abc = (struct sockaddr_in) addr;
            struct in_addr last = abc.sin_addr;
            int check =(int) last.s_addr;
            
            int* mcs_feedback = myArray;
            if (leader == 0 || mcs_feedback[0] == 999)
            {
                int val = check;
                leader = (val & 0xFF);
                set_Rate(current_Rate);
            }
            
            updateRateTable(mcs_feedback);
            prev_Rate = current_Rate;
            find_Rate();
        }
}


