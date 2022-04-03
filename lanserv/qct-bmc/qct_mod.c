/*
 * qct_mod.c
 *
 * Marvell specific modules for handling BMC and MC functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include <OpenIPMI/ipmi_msgbits.h>
#include <OpenIPMI/ipmi_bits.h>
#include <OpenIPMI/lanserv.h>
#include <OpenIPMI/mcserv.h>

#define PVERSION "0.0.1"

// QCT net function
#define IPMI_OEM_QCT_NETFN		0x36

// QCT netfuntion 
#define IPMI_OEM_QCT_GET_INFO_CMD 0x65



static void
handle_get_platform_id(lmc_data_t    *mc,
		  msg_t         *msg,
		  unsigned char *rdata,
		  unsigned int  *rdata_len,
		  void          *cb_data)
{
    sys_data_t *sys = cb_data;
    int rv;
    memset(rdata, 0, 2);
    rdata[1] = 2; 
    *rdata_len = 2;

    if (rv) {
	sys->log(sys, OS_ERROR, NULL,
		 "MVMOD: Unable to write cold reset reason: %s",
		 strerror(rv));
    }

}


int
ipmi_sim_module_print_version(sys_data_t *sys, char *initstr)
{
    printf("IPMI Simulator Marvell QCT Purley module version %s\n", PVERSION);
    return 0;
}


/**************************************************************************
 * Module initialization
 *************************************************************************/

int
ipmi_sim_module_init(sys_data_t *sys, const char *initstr_i)
{
    unsigned int num;
    int rv;
    const char *c;
    char *next;
    int use_events = 1;
    struct timeval tv;
    int power_up_force = 0;
    char *initstr = strdup(initstr_i);
    int val;


    if (!initstr) {
	sys->log(sys, SETUP_ERROR, NULL, "Error: MV: Out of memory");
	return ENOMEM;
    }

    free(initstr);


    rv = ipmi_emu_register_cmd_handler(IPMI_OEM_QCT_NETFN, IPMI_OEM_QCT_GET_INFO_CMD,
				handle_get_platform_id, sys);
    if (rv) {
	sys->log(sys, OS_ERROR, NULL,
		 "Unable to register get platform id handler: %s", strerror(rv));
    }


    return 0;
}

int
ipmi_sim_module_post_init(sys_data_t *sys)
{
}
