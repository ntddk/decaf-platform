#include "DECAF_types.h"
#include "DECAF_main.h"
#include "DECAF_callback.h"
#include "DECAF_callback_common.h"
#include "vmi_callback.h"
#include "utils/Output.h"
#include "DECAF_target.h"

// basic stub for plugins
static plugin_interface_t geteip_interface;
static DECAF_Handle processbegin_handle = DECAF_NULL_HANDLE;

char targetname[512];

uint32_t target_cr3;

static DECAF_Handle check_eip_handle;


static void geteip_block_begin_callback(DECAF_Callback_Params* params)
{
//	DECAF_read_register(eip_reg, &eip);
	if(params->ib.env->cr[3]==target_cr3)
	{
		DECAF_printf("EIP 0x%08x \n",params->ib.env->eip);
//		
	}
}


//check EIP tainted
//support for tainted eip check. Only eip of indirect branch is included. Both source eip and target eip can be recorded. now ,only support x86 platform.
static void check_eip(DECAF_Callback_Params* params)
{
	if(params->ec.target_eip_taint)
	printf("CHECK_EIP : SOURCE: 0x%08x TARGET: 0x%08x  TAINT_VALUE: 0x%08x \n", params->ec.source_eip, params->ec.target_eip, params->ec.target_eip_taint);
}

// This callback is invoked when a new process starts in the guest OS.
static void geteip_loadmainmodule_callback(VMI_Callback_Params* params)
{
	if(strcmp(params->cp.name,targetname)==0){
		DECAF_printf("Process %s you spcecified starts \n",params->cp.name);
		target_cr3=params->cp.cr3;
		DECAF_register_callback(DECAF_BLOCK_BEGIN_CB,&geteip_block_begin_callback,NULL);
	}
}

// Handler to implement the command monitor_proc.
void do_monitor_proc(Monitor* mon, const QDict* qdict)
{
/*
* Copy the name of the process to be monitored to targetname.
*/
	if ((qdict != NULL) && (qdict_haskey(qdict, "procname"))) {
		strncpy(targetname, qdict_get_str(qdict, "procname"), 512);
	}
	targetname[511] = '\0';
	DECAF_printf("Ready to track \n", targetname);
}

static int geteip_init(void)
{
	DECAF_printf("Hello World\n");

	//register for process create and process remove events
	processbegin_handle = VMI_register_callback(VMI_CREATEPROC_CB, &geteip_loadmainmodule_callback, NULL);
	if (processbegin_handle == DECAF_NULL_HANDLE)
	{
		DECAF_printf("Could not register for the create or remove proc events\n");  
	}
	return (0);
}

/*
* This function is invoked when the plugin is unloaded.
*/
static void geteip_cleanup(void)
{
	DECAF_printf("Bye world\n");
	/*
	* Unregister for the process start and exit callbacks.
	*/
	if (processbegin_handle != DECAF_NULL_HANDLE) {
		VMI_unregister_callback(VMI_CREATEPROC_CB, processbegin_handle);  
		processbegin_handle = DECAF_NULL_HANDLE;
	}
}

static mon_cmd_t geteip_term_cmds[] = {
				{
				.name			= "geteip",
				.args_type		= "procname:s?",
				.mhandler.cmd	= do_monitor_proc,
// Handler registration
				.params			= "[procname]",
				.help			= "tracking EIP of [procname] as block"
				},
	{NULL, NULL, },
};

// Initialization
// The interface is used to register custom commands, let DECAF know which cleanup function to call upon plugin unload, etc,.
plugin_interface_t* init_plugin(void)
{
	geteip_interface.mon_cmds = geteip_term_cmds;
	geteip_interface.plugin_cleanup = &geteip_cleanup;

	check_eip_handle = DECAF_register_callback(DECAF_EIP_CHECK_CB, check_eip, NULL);
	DECAF_printf("register eip check callback\n");
	geteip_init();

//
//		DECAF_register_callback(DECAF_EIP_CHECK_CB,&geteip_eip_check_callback,NULL);
//
	return (&geteip_interface);
}