#include "common.h"

/*Flag for debug mode*/
bool debug_mode = FALSE;

char* enum_to_str(msg_cause cause)
{
   switch(cause)
   {
      case ACCEPTED:
                    return "ACCEPTED";
      case REJECTED:
                    return "REJECTED";
      default:
                    return "UNKNOWN";
   }
}

msg_cause str_to_enum(char *str)
{
    if (!strcmp(str,"ACCEPTED"))
        return ACCEPTED;
    else if (!strcmp(str,"REJECTED"))
        return REJECTED;
    else
        return UNKNOWN;

}

/* <doc>
 * char* get_my_ip(const char * device)
 * Gives IP for a particular interface
 *
 * </doc>
 */
void get_my_ip(const char * device, struct sockaddr *addr)
{
    int fd;
    struct ifreq ifr;

    fd = socket (AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET; 
    strncpy (ifr.ifr_name, device, IFNAMSIZ-1);

    ioctl (fd, SIOCGIFADDR, &ifr);

    close (fd);

    (*addr).sa_family = ifr.ifr_addr.sa_family;
    memcpy((*addr).sa_data, ifr.ifr_addr.sa_data,sizeof((*addr).sa_data));
}


/* <doc>
 * int IS_SERVER(int oper)
 * Checks if operational mode type
 * is Server
 * </doc>
 */
int IS_SERVER(int oper)
{
  if (SERVER_MODE == oper)
     return 1;

  return 0;
}

/* <doc>
 * int IS_CLIENT(int oper)
 * Checks if operational mode type
 * is Client
 * </doc>
 */
int IS_CLIENT(int oper)
{
  if (CLIENT_MODE == oper)
     return 1;

  return 0;
}

/* <doc>
 * int create_and_bind(char *machine_addr, char *machine_port, int oper_mode)
 * Create the UDP socket and binds it to specified address and port
 *
 * </doc>
 */
int create_and_bind(char *machine_addr, char *machine_port, int oper_mode)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd = -1;
    int yes=1;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(machine_addr, machine_port, &hints, &result) != 0)
    {
        PRINT("Failure in getaddrinfo");
        return -1;
    }

    for (rp = result; rp != NULL; rp=rp->ai_next)
    {
        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
        {
            continue;
        }

        /*Reuse the socket*/
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        /*Keeping separate bind blocks for server and client for future changes*/
        if (IS_SERVER(oper_mode))
        {
           if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == -1)
           {
              continue;
           }
        }
        else if (IS_CLIENT(oper_mode))
        {
           if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == -1)
           {
              continue;
           }
        }
        break;
    }

    if (!rp)
    {
      perror("\nFailure in binding socket.");
      exit(1);
    }

    freeaddrinfo(result);

    return sfd;
}

/* <doc>
 * int make_socket_non_blocking (int sfd)
 * This function accepts fd as input and
 * makes it non-blocking using fcntl
 * system call.
 *
 * </doc>
 */
int make_socket_non_blocking (int sfd)
{
    int flags;

    flags = fcntl(sfd, F_GETFL,0);

    if (flags == -1) {
        perror("Error while fcntl F_GETFL");
        return -1;
    }

    fcntl(sfd, F_SETOWN, getpid());
    if (fcntl(sfd, F_SETFL,flags|O_NONBLOCK) == -1) {
        perror("Error while fcntl F_SETFL");
        return -1;
    }

//  DEBUG("Socket is non-blocking now.");

  return 1;
}


char* get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (char *) &(((struct sockaddr_in*) sa)->sin_addr);
    } else {
        return (char *) &(((struct sockaddr_in6*) sa)->sin6_addr);
    }
}

void initialize_echo_request(echo_req_t *echo_req)
{
    echo_req->group_name  = NULL;
    echo_req->num_clients = 0;
    echo_req->client_ids  = NULL;
}

/* <doc>
 * int send_echo_request(unsigned int sockfd, struct sockaddr *addr, char *grp_name)
 * Sends echo request on mentioned sockfd to the addr passed.
 * grp_name is filled in echo req pdu.
 *
 * </doc>
 */
int send_echo_request(const int sockfd, struct sockaddr *addr, char *grp_name)
{
    pdu_t pdu;
    char ipaddr[INET6_ADDRSTRLEN];
    int j = 0;

    comm_struct_t *req = &(pdu.msg);
    req->id = echo_req;
    
    echo_req_t *echo_request = &(req->idv.echo_req);
    initialize_echo_request(echo_request);

    /*Create the echo request pdu*/
    echo_request->group_name = MALLOC_STR;
    strcpy(echo_request->group_name, grp_name);

    inet_ntop(AF_INET, get_in_addr(addr), ipaddr, INET6_ADDRSTRLEN);
    //ECHO
    //PRINT("[Echo_Request: GRP - %s] Echo Request sent to %s", echo_request->group_name, ipaddr);
    LOGGING_INFO("[Echo_Request: GRP - %s] Echo Request sent to %s", echo_request->group_name, ipaddr);

    write_record(sockfd, addr, &pdu);

    return 0;
}


/* <doc>
 * inline int calc_key(struct sockaddr *addr)
 * calc_key returns the key based on IP address.
 *
 * </doc>
 */
inline unsigned int
calc_key(struct sockaddr *addr)
{
  if (addr && addr->sa_family == AF_INET)
    return (((struct sockaddr_in*) addr)->sin_addr.s_addr);

  return 0;
}

void display_server_clis()
{
  PRINT("show groups                                          --  Displays list of groups");
  PRINT("show group info <group_name|all>                     --  Displays group - client association");
  PRINT("task <task_type> group <group_name> file <filename> type <structured> --  Assigns a specific task to the specified Group. \
                                                                                   filename is optional. Default Value: task_set/prime_set1.txt \
                                                                                   Default file type is unstructured");
  PRINT("server backup <backup_server_ip>                     --  Configures IP of secondary server");
  PRINT("migrate                                              --  Migrates to secondary server");
  PRINT("enable debug                                         --  Enables the debug mode");
  PRINT("disable debug                                        --  Disables the debug mode");
  PRINT("enable metrics                                       --  Enables the execution metrics");
  PRINT("disable metrics                                      --  Disables the execution metrics");
  PRINT("cls                                                  --  Clears the screen");
}

void start_oneshot_timer(timer_t *timer_id, uint8_t interval, uint32_t sigval)
{
    struct itimerspec its;
    struct sigevent sev;
    
    its.it_value.tv_sec = interval;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = sigval;
    sev.sigev_value.sival_ptr = timer_id;
    
    if(*timer_id == 0)
        if(timer_create(CLOCKID, &sev, timer_id) == -1)
            errExit("timer_create");

    if(timer_settime(*timer_id, 0, &its, NULL) == -1)
         errExit("timer_settime");
}

void start_recurring_timer(timer_t *timer_id, uint8_t interval, uint32_t sigval)
{
    struct itimerspec its;
    struct sigevent sev;
    
    //For now, using simplified timer structure. First timeout and subsequent
    //timeouts are at the granularity of seconds and are same. This can be
    //modified to operate at nanosecond granularity though.
    uint8_t r_interval = interval;
    
    its.it_value.tv_sec = interval;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = sigval;
    sev.sigev_value.sival_ptr = timer_id;
    
    if(*timer_id == 0)
        if(timer_create(CLOCKID, &sev, timer_id) == -1)
            errExit("timer_create");

    if(timer_settime(*timer_id, 0, &its, NULL) == -1)
         errExit("timer_settime");
}

/* <doc>
 * unsigned int generate_random_capability(void)
 * This function returns a random capability between 1-3 for a client
 * </doc>
 */
unsigned int generate_random_capability(void)
{
  unsigned int number = 1;
  
  srand(time(NULL));
  number = (rand() % 3) + 1;

  return number;
}

/* <doc>
 * void enable_logging(char *prog_name)
 * This just enables the logging functionality.
 *
 * </doc>
 */
void enable_logging(char *prog_name)
{
    int dummy_argc = 4;

    /*LOGGING PARAMETERS*/
    mkdir("./execution_logs", S_IRWXU);   /*create dir read/write/search user privilages*/
    char* dummy_argv[] =
    {
        prog_name,                        /*substituted as executing binary name*/
        "--v=1",                          /*log anything that has verbosity level higher than 1*/
        "--log_dir=./execution_logs",     /*specifying directory where we want logs to be generated.*/
        "--logtostderr=0"                 /*mildly sticky/inconvenient. Would dump all allowed logs onto terminal as well.*/
    };
    /*start logging*/
    initialize_logging(dummy_argc, dummy_argv);
}
/* <doc>
 * unsigned int get_task_count(const char* filename,long** task_set)
 * This reads the task file and gets the total count of numbers
 * </doc>
 */
unsigned int get_task_count(const char* filename){

     FILE *cmd_line;
     uint32_t count;
     char cmd[256];

     sprintf(cmd, "%s %s", "wc -l" , filename);
     cmd_line = popen (cmd, "r");
     fscanf(cmd_line, "%i", &count);
     pclose(cmd_line);

     return count;
}


/* <doc>
 * Fetches the file from remote location.
 *
 * </doc>
 */
char * fetch_file(char * src, char *dest){

   char cmd[180];
   sprintf(cmd,"./file_tf %s %s >>/tmp/file_tf.logs", src, dest);
//   sprintf(cmd,"./file_tf root@%s %s >>/tmp/file_tf.logs", src, dest); //for rtp machines

   PRINT("fetching file from %s", src);
   LOGGING_INFO("fetching file from %s", src);

   int status = system(cmd);
   if(status == -1)
     PRINT("some error is occured in that shell command");
   else if (WEXITSTATUS(status) == 127)
     PRINT("That shell command is not found");
   else{
     LOGGING_INFO("system call return succesfull with  %d",WEXITSTATUS(status));
     return dest;
   } 
   //failed to fetch file hence freeing allocated memory
   free(dest);
   return NULL;
}  

void inline create_folder(char * path){
  int len=strlen(path);
  char cmd[len+10];
  sprintf(cmd,"mkdir -p %s",path);
  system(cmd);
}

void inline check_and_create_folder(char * path){
  struct stat st={0};
  if(stat(path, &st)==-1){
   create_folder(path);
  }
}

/* When timer is changing queues/type of signal raised. DO NOT use this API. Use
 * timer_delete instead */
void stop_timer(timer_t *timer_id)
{
    struct itimerspec its;

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(*timer_id, 0, &its, NULL) == -1)
        errExit("stop timer_settime");
}

void mask_signal(uint32_t sigval, bool flag)
{
    sigset_t mask;
    sigset_t orig_mask;
    struct sigaction act;

    sigemptyset (&mask);
    sigaddset (&mask, sigval);

    if (flag)
    {
        if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
            perror ("sigprocmask");
            return;
        }
    }
    else
    {
        if (sigprocmask(SIG_UNBLOCK, &mask, &orig_mask) < 0) {
            perror ("sigprocmask");
            return;
        }
    }
}

/* <doc>
 * float timedifference_msec(struct timeval t0, struct timeval t1)
 * Finds the difference between two timevals in milliseconds.
 *
 * </doc>
 */
float timedifference_msec(struct timeval t0, struct timeval t1)
{
    float result = ((t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000);
    return result;
}

