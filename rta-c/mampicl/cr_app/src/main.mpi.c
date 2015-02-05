
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "logging.h"
#include "crystal.h"

// Access implementation details
#include "../src/crystal_aux.h"
#include "../src/crystal_router.h"

#define MAX_DEST 256
#define MESS_LEN 512

#define SETUP_MESSAGES 1
#define LIST_MESSAGES  2
#define SEND_MESSAGES  3
#define GET_MESSAGES   4
#define CHANGE_BUFFER  5
#define CLEAR_MESSAGES 6
#define ERROR_MESSAGES 7
#define STOP           8

int nproc, doc, procnum, cp_num;

static int get_task( void );
static void welcome( void );
static void get_param( void );
static void write_mail( void );
static int print_mess( char *buf, int source, int nbytes, int istat );
// static void list_messages( void );
// static void list_buffer( char *start_ptr, int total_bytes, int in_out );
static void read_mail( void );
static void set_mail_buffers( void );
static void clear_messages( void );
static void error_messages( FILE *stdout );

static MPI_Comm cp_cr_comm;
static CrRouter *crr;

static void setup_logging( void )
{
    int myrank;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    log_init(myrank);
}

static void setup_crystal( void )
{
    MPI_Comm cr_comm;

    // Position in MPI_COMM_WORLD
    MPI_Comm_rank(MPI_COMM_WORLD, &procnum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // rank 0 is control processor
    MPI_Comm_split(MPI_COMM_WORLD, 1, (procnum != 0) ? procnum : INT_MAX,
                   &cp_cr_comm);
    MPI_Comm_rank(cp_cr_comm, &procnum);
    cp_num = nproc - 1;
    nproc -= 1;

    // create a communicator for use within the crystal router.
    MPI_Comm_split(cp_cr_comm, (procnum != cp_num) ? 1 : MPI_UNDEFINED, procnum,
                   &cr_comm);

    if ( procnum != cp_num )
    {
        CommContext comm = mcMPICommContext(cr_comm);
        crr = new_crystal_router(comm);
    }
}

int
main( int argc, char *argv[] )
{
    int task;

    MPI_Init(&argc, &argv);
    setup_logging();
    setup_crystal();

    welcome();
    get_param();

    while ( (task = get_task()) != STOP )
    {
        switch ( task )
        {
        case SETUP_MESSAGES:
            write_mail();
            break;

        case LIST_MESSAGES:
            // list_messages();
            break;

        case SEND_MESSAGES:
            if ( crr != NULL )
                crystal_router(crr);
            break;

        case GET_MESSAGES:
            read_mail();
            break;

        case CHANGE_BUFFER:
            set_mail_buffers();
            break;

        case CLEAR_MESSAGES:
            clear_messages();
            break;

        case ERROR_MESSAGES:
            error_messages(stdout);
            break;
        }
    }
    if ( procnum == cp_num )
        printf("\n\nFinished\n\n");

    MPI_Comm_free(&cp_cr_comm);
    del_crystal_router(crr);
    MPI_Finalize();
    log_fini();
    return 0;
}


static int
exch_int(int dest_rank, int src_rank, int *value, int len)
{
    if ( procnum == src_rank )
    {
        if ( dest_rank < 0 )
            MPI_Bcast(value, len, MPI_INT, src_rank, cp_cr_comm);
        else
            MPI_Send(value, len, MPI_INT, dest_rank, 0, cp_cr_comm);
    }
    else
    {
        if ( dest_rank < 0 )
        {
            MPI_Bcast(value, len, MPI_INT, src_rank, cp_cr_comm);
        }
        else if ( procnum == dest_rank )
        {
            MPI_Status stat;

            MPI_Recv(value, len, MPI_INT, src_rank, 0, cp_cr_comm, &stat);
        }
    }
    return *value;
}


static void *
exch_buf(int dest_rank, int src_rank, void *buffer, int len)
{
    if ( procnum == src_rank )
    {
        if ( dest_rank < 0 )
            MPI_Bcast(buffer, len, MPI_BYTE, src_rank, cp_cr_comm);
        else
        {
            SIMPLETRACE("starting send");
            MPI_Send(buffer, len, MPI_BYTE, dest_rank, 0, cp_cr_comm);
        }
    }
    else
    {
        if ( dest_rank < 0 )
            MPI_Bcast(buffer, len, MPI_BYTE, src_rank, cp_cr_comm);
        else if ( procnum == dest_rank )
        {
            MPI_Status stat;

            SIMPLETRACE("startign receive");
            MPI_Recv(buffer, len, MPI_BYTE, src_rank, 0, cp_cr_comm, &stat);

            int new_len;

            MPI_Get_count(&stat, MPI_BYTE, &new_len);
        }
    }
    return buffer;
}


static int
read_int( int err_val, const char *prompt )
{
    int tmp = err_val;
    char buff[81];

    if ( procnum == cp_num )
    {
        if ( prompt != NULL )
            printf("%s", prompt);
        if ( fgets(buff, 81, stdin) != NULL )
            tmp = atoi(buff);
    }
    return tmp;
}


static char *
read_str( char *buff, int len, const char *prompt )
{
    if ( procnum == cp_num )
    {
        if ( prompt != NULL )
            printf("%s", prompt);
        if ( fgets(buff, len, stdin) == NULL )
            *buff = '\0';
    }
    return buff;
}


static int
get_task( void )
{
    int ok, task;

    if ( procnum == cp_num )
    {
        ok = 0;
        while ( !ok )
        {
            printf("\n\nPlease choose one of the following :\n");
            printf("   1...to create some message\n");
            printf("   2...to list one of the message buffers\n");
            printf("   3...to send messages to their destinations\n");
            printf("   4...to read some of the received messages\n");
            printf("   5...to change size of message buffers\n");
            printf("   6...to clear one of the message buffers\n");
            printf("   7...to list error message, if any\n");
            printf("   8...to terminate the program\n\n==> ");

            task = read_int(-1, NULL);
            // printf("input: %d\n", task);

            // task = atoi(input);

            if ( task > 0 && task < 9 )
                ok = 1;
            else
                printf("\n**** Invalid input - try again ****\n");
        }
    }
    exch_int(-1, cp_num, &task, 1);

    return task;
}


static void
welcome( void )
{
    if ( procnum == cp_num )
    {
        printf("\n\n\t*******************************************************\n");
        printf(    "\t*                                                     *\n");
        printf(    "\t*    Welcome to the MPI version of the                *\n");
        printf(    "\t*    crystal router demonstration program.            *\n");
        printf(    "\t*                                                     *\n");
        printf(    "\t*******************************************************\n");
    }
}


static void
write_mail( void )
{
    L4C(__log4c_category_trace(GlobLogCat, "write_mail() -- begin"));

    int ntemp, destination, source, nbytes, dest[MAX_DEST], ndest;
    char message[80];

    for ( ; ; )
    {
        SIMPLETRACE("  write_mail():\tMPI_Barrier() entering");
        MPI_Barrier(cp_cr_comm);
        SIMPLETRACE("  write_mail():\tMPI_Barrier() passed");
        source = read_int(-1, "\n\nPlease give the source processor "
                              "(-ve to end) ==> ");
        exch_int(-1, cp_num, &source, 1);
        if ( source < 0 )
            break;
        if ( procnum == cp_num )
        {
            ndest = 0;
            for ( ; source >= 0; )
            {
                destination = read_int(-1,
                                "\n\nPlease give the destination processor "
                                "(-ve to end) ==> ");
                if ( destination < 0 ) break;
                dest[ndest] = destination;
                ndest++;
            }
            // getchar();
            if ( ndest > 0 )
                read_str(message, 78, "\n\nMessage (< 78 characters) ==> ");
            else
                *message = '\0';
        }
        SIMPLETRACE("  write_mail():\tmessage defined");

        exch_int(source, cp_num, &ndest, 1);
        SIMPLETRACE("  write_mail():\tndest received");
        exch_int(source, cp_num, dest, ndest);
        SIMPLETRACE("  write_mail():\tdest received");
        exch_buf(source, cp_num, message, 80);
        SIMPLETRACE("  write_mail():\tmessage forwarded");

        if ( procnum == source )
        {
            nbytes = strlen(message + 1);
            ntemp = nbytes/4;
            if ( nbytes%4 != 0 )
                nbytes = 4*(ntemp + 1);
            else
                nbytes = 4*ntemp;
            SIMPLETRACE("  write_mail():\tcalling send_message()");
            send_message(&crr->out_buf, message, nbytes, ndest, dest, crr->procnum);
        }
        SIMPLETRACE("  write_mail():\tmessage processed");
    }

    L4C(__log4c_category_trace(GlobLogCat, "write_mail() -- end"));
}

#if 0
static void
list_messages( void )
{
    L4C(__log4c_category_trace(GlobLogCat, "list_messages() -- begin"));

    int buf_id;

    for ( ; ; )
    {
        if ( procnum == cp_num )
        {
            printf("\n\nPlease select the buffer you want to list :\n");
            printf(    "   1...the buffer of outgoing messages\n");
            printf(    "   2...the buffer of received messages\n");
            printf(    "   3...to quit this menu\n==> ");
            buf_id = read_int(-1, NULL);
            printf("\n");
            if ( buf_id < 1 || buf_id > 3 )
                printf("\n\n**** Invalid input - try again ****\n");
        }
        exch_int(-1, cp_num, &buf_id, 1);

        if ( buf_id == 1 )
        {
            list_buffer(crb_data(crb_buffer(crr, OUT)),
                        crb_data_len(crb_buffer(crr, OUT)), OUT);
        }
        else if ( buf_id == 2 )
            list_buffer(crb_data(crb_buffer(crr, IN)),
                        crb_data_len(crb_buffer(crr, IN)), IN);
        else if ( buf_id == 3 )
            break;
    }

    L4C(__log4c_category_trace(GlobLogCat, "list_messages() -- end"));
}
#endif

#if 0
// Needs a better interface to crystal router and the adaptation to it.
static void
list_buffer( char *start_ptr, int total_bytes, int in_out )
{
    L4C(__log4c_category_trace(GlobLogCat, "list_buffer() -- begin"));

    int proc, message_number, bytes, dest, src, message_size, status, nbytes;
    int i, j, ndest;
    char c, status_string[8], *buf_ptr;

    if ( procnum == cp_num )
    {
        printf("\n\n    ******************************************");
        printf(  "\n      ");
        printf(  "\n      %s", (in_out == OUT)
                               ? "Listing of buffer of outgoing messages"
                               : "Listing of buffer of received messages");
        printf("\n\n    ******************************************");
    }
    for ( ; ; )
    {
        MPI_Barrier(cp_cr_comm);
        proc = read_int(-1, "\n\nPlease give the processor (-ve to end) ==> ");
        MPI_Bcast(&proc, 1, MPI_INT, cp_num, cp_cr_comm);
        if ( proc < 0 ) break;

        // fmulti(stdout);
        if ( procnum == proc )
        {
            exch_int(cp_num, proc, &total_bytes, 1);
            if ( total_bytes != 0 )
                exch_buf(cp_num, proc, start_ptr, total_bytes);
        }
        else if ( procnum == cp_num )
        {
            exch_int(cp_num, proc, &total_bytes, 1);
            if ( total_bytes == 0 )
                printf("\n\nThe buffer of processor %d is empty.\n", proc);
            else
            {
                char proc_buffer[total_bytes];

                buf_ptr = exch_buf(cp_num, proc, proc_buffer, total_bytes);
                bytes   = 0;
                message_number = 1;
                while ( bytes < total_bytes )
                {
                    ndest  = *((int *)buf_ptr);
                    src    = *((int *)(buf_ptr + sizeof(int)));
                    nbytes = *((int *)(buf_ptr + 2*sizeof(int)));
                    status = *((int *)(buf_ptr + 3*sizeof(int)));
                    if ( status == CURRENT )
                        strcpy(status_string, "CURRENT");
                    else
                        strcpy(status_string, "OLD");
                    int header_size = (4 + ndest)*sizeof(int);
                    message_size = nbytes + header_size;
                    buf_ptr += 4*sizeof(int);
                    printf("\nMessage number %d from processor %d",
                           message_number, src);
                    printf(", nbytes = %d, status = %s\n",
                           nbytes, status_string);
                    printf("Destination processors:");
                    for (i = 0, j = 4; i < ndest; ++i, j++)
                    {
                        dest = *((int *)buf_ptr);
                        c = ( j%10 == 9 || i == (ndest-1) ) ? '\n' : ' ';
                        printf("%6d%c", dest, c);
                        buf_ptr += sizeof(int);
                    }
                    printf("%s\n", buf_ptr);
                    bytes   += message_size;
                    buf_ptr += nbytes;
                    message_number++;
                }
            }

        }
        // fsingle(stdout);
    }

    L4C(__log4c_category_trace(GlobLogCat, "list_buffer() -- end"));
}
#endif

static void
read_mail( void )
{
    L4C(__log4c_category_trace(GlobLogCat, "read_mail() -- begin"));

    int nbytes, source, mess_proc;
    char buf[MESS_LEN];

    for ( ; ; )
    {
        MPI_Barrier(cp_cr_comm);
        mess_proc = read_int(-1,
                       "\n\nPlease give processor number for reading message"
                       "  \n (-ve to end) ==> ");
        exch_int(-1, cp_num, &mess_proc, 1);
        if (mess_proc < 0) break;
        if ( procnum != cp_num && procnum != mess_proc )
            continue;

        for ( ; ; )
        {
            char prompt[256];

            sprintf(prompt, "\n\nPlease give the source processor"
                            " (%d = any, -ve = end) ==> ", ANY);
            source = read_int(-1, prompt);
            exch_int(mess_proc, cp_num, &source, 1);
            if ( source < 0 ) break;

            if ( source < nproc || source == ANY )
            {
                // fmulti(stdout)
                if ( procnum == mess_proc )
                {
                    int resp;

                    resp = get_message(&crr->in_buf,
                                       crr->procnum, &source, &nbytes, buf);
                    if ( resp >= 0 )
                        print_mess(buf, source, nbytes, resp);
                }
                // fsingle(stdout);
            }
            else
            {
                if ( procnum == cp_num )
                    printf("\n\n**** Invalid input - try again ****\n");
            }
        }
    }
    L4C(__log4c_category_trace(GlobLogCat, "read_mail() -- end"));
}


static int
print_mess( char *buf, int source, int nbytes, int istat )
{
    switch ( istat )
    {
    case 0:
        printf("\n\nMessage from processor %d to processor %d :\n%s\n",
               source, procnum, buf);
        break;
    case 1:
        printf("\n\nThere are no messages for processor %d\n", procnum);
        break;
    case 2:
        printf("\n\nNo message of the requested type has been\n");
        printf(" sent to processor %d\n", procnum);
        break;
    case 3:
        printf("\n\nAll the messages have been read for processor.\n");
        break;
    }
    return 0;
}


static void
set_mail_buffers( void )
{
    int buf_id, buf_size;
    char input[5];

    for ( ; ; )
    {
        if ( procnum == cp_num )
        {
            printf("\n\nPlease choose one of the following :\n");
            printf(    "   1...to change the outgoing message buffer\n");
            printf(    "   2...to change the received message byffer\n");
            printf(    "   3...to quit this menu\n\n==> ");
            scanf(" %s", input);
            buf_id = atoi(input);
            if ( buf_id < 1 || buf_id > 3 )
                printf("\n\n**** Invalid input - try again ****\n");
        }
        MPI_Bcast(&buf_id, 1, MPI_INT, cp_num, cp_cr_comm);

        if ( buf_id == 3 )
            break;
        else {
            if ( procnum == cp_num )
            {
                printf("\n\nPlease give buffer size in bytes ==> ");
                scanf(" %d", &buf_size);
            }
            MPI_Bcast(&buf_size, 1, MPI_INT, cp_num, cp_cr_comm);
            crb_resize(crb_buffer(crr, buf_id == 1 ? OUT : IN), buf_size);
        }
    }
    if ( procnum == cp_num )
        printf("\n");
}


static void
clear_messages( void )
{
    int buf_id;
    char input[5];

    for ( ; ; )
    {
        if ( procnum == cp_num )
        {
            printf("\n\nPlease choose one of the following :\n");
            printf(    "   1...to change the outgoing message buffer\n");
            printf(    "   2...to change the received message byffer\n");
            printf(    "   3...to quit this menu\n\n==> ");
            scanf(" %s", input);
            buf_id = atoi(input);
            if ( buf_id < 1 || buf_id > 3 )
                printf("\n\n**** Invalid input - try again ****\n");
        }
        MPI_Bcast(&buf_id, 1, MPI_INT, cp_num, cp_cr_comm);
        if ( buf_id == 3 )
            break;
        else if ( buf_id == 1 )
            crb_clear(crb_buffer(crr, OUT));
        else if ( buf_id == 2 )
            crb_clear(crb_buffer(crr, IN));
    }
    if ( procnum == cp_num )
        printf("\n");
}

static void
get_param( void )
{
}

static void
error_messages( FILE *stdout )
{
}
