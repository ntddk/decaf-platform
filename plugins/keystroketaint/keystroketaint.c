/*
Copyright (C) <2012> <Syracuse System Security (Sycure) Lab>
This is a plugin of DECAF. You can redistribute and modify it
under the terms of BSD license but it is made available
WITHOUT ANY WARRANTY. See the top-level COPYING file for more details.

For more information about DECAF and other softwares, see our
web site at:
http://sycurelab.ecs.syr.edu/

If you have any questions about DECAF,please post it on
http://code.google.com/p/decaf-platform/
*/
/**
 * @author Xunchao Hu
 * @date May 24 2013
 *
 *This plugin taints a keystroke,and print out some basic info of the tainted instructions.
 *To use this pllugin, first compile decaf with "--enalbe-tcg-taint" and "--disable-vmi".
 *After starting decaf, taint a keystroke using command "taint_sendkey".Also, you can use the
 *similar method to register network read/write callback and taint the input/output of network.
 *
 *Other functions you can use to extend this plugin
 *taintcheck_check_virtmem(uint32_t vaddr, uint32_t size, uint8_t * taint)
 *taintcheck_register_check(int regid,int offset,int size,CPUState *env)
 *taint_mem(uint32_t addr,int size,uint8_t *taint)
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

#include <sys/time.h>
#include <math.h>


#include "procmod.h"
#include "DECAF_main.h"
#include "DECAF_target.h"
#include "DECAF_callback.h"
#include "DECAF_callback_common.h"

#include "hookapi.h"
#include "function_map.h"
#include "vmi_include.h"


/* Plugin interface */
static plugin_interface_t keystroketaint_interface;


#ifdef CONFIG_TCG_TAINT
DECAF_Handle keystroke_cb_handle = DECAF_NULL_HANDLE;
DECAF_Handle handle_write_taint_mem = DECAF_NULL_HANDLE;
DECAF_Handle handle_read_taint_mem = DECAF_NULL_HANDLE;
#endif /* CONFIG_TCG_TAINT */



#ifdef CONFIG_TCG_TAINT
void do_read_taint_mem(DECAF_Callback_Params *param)
{
	uint32_t eip = cpu_single_env->eip;
	uint32_t cr3 = cpu_single_env->cr[3];
	char name[128];
	dmodule * dm = NULL;
	dm = locate_module(eip,cr3,name);
	if(!dm)
	return;

	if(param->rt.size <=4)
	DECAF_printf("READ 0x%08x 0x%08x 0x%08x %s\n",param->rt.vaddr,param->rt.paddr,*((uint32_t *)param->rt.taint_info),dm->name);
	else
	DECAF_printf("READ 0x%08x 0x%08x 0x%16x %s\n",param->rt.vaddr,param->rt.paddr,*((uint32_t *)param->rt.taint_info),dm->name);

}


void do_write_taint_mem(DECAF_Callback_Params *param)
{
	uint32_t eip = cpu_single_env->eip;
	uint32_t cr3 = cpu_single_env->cr[3];
	char name[128];
	dmodule * dm = NULL;
	dm = locate_module(eip,cr3,name);
	if(!dm)
		return;

	if(param->rt.size <=4)
	DECAF_printf("WRITE 0x%08x 0x%08x 0x%08x %s\n",param->wt.vaddr,param->wt.paddr,*((uint32_t *)param->rt.taint_info),dm->name);
	else
	DECAF_printf("WRITE 0x%08x 0x%08x 0x%16x %s\n",param->wt.vaddr,param->wt.paddr,*((uint32_t *)param->rt.taint_info),dm->name);



}
static int taint_key_enabled=0;
static void tracing_send_keystroke(DECAF_Callback_Params *params)
{

	int keycode = params->ks.keycode;
	uint32_t *taint_mark = params->ks.taint_mark;
	*taint_mark = taint_key_enabled;
	taint_key_enabled = 0;
	printf("taint keystroke %d \n ",keycode);

}

void do_taint_sendkey(Monitor *mon,const QDict *qdict)
{
	if (qdict_haskey(qdict, "key")) {
		//register keystroke callback
		taint_key_enabled = 1;
		if (!keystroke_cb_handle)
			keystroke_cb_handle = DECAF_register_callback(DECAF_KEYSTROKE_CB,
					tracing_send_keystroke, &taint_key_enabled);
		// Send the key
		do_send_key(qdict_get_str(qdict, "key"));

	} else
		monitor_printf(mon, "taint_sendkey command is malformed\n");

}
#endif /* CONFIG_TCG_TAINT */

static mon_cmd_t keystroketaint_term_cmds[] = {
#include "plugin_cmds.h"
  {NULL, NULL, },
};

void keystroketaint_init()
{
#ifdef CONFIG_TCG_TAINT
	if (!handle_read_taint_mem)
		handle_read_taint_mem = DECAF_register_callback( DECAF_READ_TAINTMEM_CB, do_read_taint_mem, NULL );
	if (!handle_write_taint_mem)
		handle_write_taint_mem = DECAF_register_callback( DECAF_WRITE_TAINTMEM_CB, do_write_taint_mem, NULL );
#endif
}

void keystroketaint_cleanup()
{

#ifdef CONFIG_TCG_TAINT
	if(keystroke_cb_handle!=0)
		DECAF_unregister_callback(DECAF_KEYSTROKE_CB,keystroke_cb_handle);
	if(handle_read_taint_mem)
		DECAF_unregister_callback(DECAF_READ_TAINTMEM_CB,handle_read_taint_mem);
	if(handle_write_taint_mem)
		DECAF_unregister_callback(DECAF_WRITE_TAINTMEM_CB,handle_write_taint_mem);
#endif /* CONFIG_TCG_TAINT */

}

/* Register the plugin and the callbacks */
plugin_interface_t * init_plugin()
{
  keystroketaint_interface.mon_cmds = keystroketaint_term_cmds;
  keystroketaint_interface.plugin_cleanup = keystroketaint_cleanup;


  keystroketaint_init ();
  return &keystroketaint_interface;
}

