/*
** Lua binding: PAC_dev
** Generated automatically by tolua++-1.0.92 on 10/07/16 09:38:32.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_PAC_dev_open (lua_State* tolua_S);

#include "errors_manager.h"
#pragma warning(push)
#pragma warning(disable:4800)

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"alarm_manager");
}

/* method: set_alarm of class  alarm_manager */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_manager_set_alarm00
static int tolua_PAC_dev_alarm_manager_set_alarm00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"alarm_manager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isstring(tolua_S,5,0,&tolua_err) ||
     !tolua_isstring(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,8,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,10,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,11,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,12,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,13,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,14,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,15,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,16,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  alarm_manager* self = (alarm_manager*)  tolua_tousertype(tolua_S,1,0);
  unsigned char PAC_description_id = ((unsigned char)  tolua_tonumber(tolua_S,2,0));
  int n = ((int)  tolua_tonumber(tolua_S,3,0));
  ALARM_TYPE a_type = ((ALARM_TYPE) (int)  tolua_tonumber(tolua_S,4,0));
  char* a_description = ((char*)  tolua_tostring(tolua_S,5,0));
  char* a_group = ((char*)  tolua_tostring(tolua_S,6,0));
  unsigned char a_enable = ((unsigned char)  tolua_tonumber(tolua_S,7,0));
  bool a_suppress = ((bool)  tolua_toboolean(tolua_S,8,0));
  unsigned char a_inhibit = ((unsigned char)  tolua_tonumber(tolua_S,9,0));
  int a_priority = ((int)  tolua_tonumber(tolua_S,10,0));
  ALARM_STATE a_state = ((ALARM_STATE) (int)  tolua_tonumber(tolua_S,11,0));
  unsigned char a_driver_id = ((unsigned char)  tolua_tonumber(tolua_S,12,0));
  int a_id_object_type = ((int)  tolua_tonumber(tolua_S,13,0));
  int a_id_object_number = ((int)  tolua_tonumber(tolua_S,14,0));
  int a_id_object_alarm_number = ((int)  tolua_tonumber(tolua_S,15,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_alarm'", NULL);
#endif
  {
   int tolua_ret = (int)  self->set_alarm(PAC_description_id,n,a_type,a_description,a_group,a_enable,a_suppress,a_inhibit,a_priority,a_state,a_driver_id,a_id_object_type,a_id_object_number,a_id_object_alarm_number);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_alarm'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: set_alarms_id of class  alarm_manager */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_manager_set_alarms_id00
static int tolua_PAC_dev_alarm_manager_set_alarms_id00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"alarm_manager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  alarm_manager* self = (alarm_manager*)  tolua_tousertype(tolua_S,1,0);
  unsigned char PAC_description_id = ((unsigned char)  tolua_tonumber(tolua_S,2,0));
  int id = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_alarms_id'", NULL);
#endif
  {
   int tolua_ret = (int)  self->set_alarms_id(PAC_description_id,id);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_alarms_id'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: set_alarms_cnt of class  alarm_manager */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_manager_set_alarms_cnt00
static int tolua_PAC_dev_alarm_manager_set_alarms_cnt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"alarm_manager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  alarm_manager* self = (alarm_manager*)  tolua_tousertype(tolua_S,1,0);
  unsigned char PAC_description_id = ((unsigned char)  tolua_tonumber(tolua_S,2,0));
  int cnt = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_alarms_cnt'", NULL);
#endif
  {
   int tolua_ret = (int)  self->set_alarms_cnt(PAC_description_id,cnt);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_alarms_cnt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: G_ALARM_MANAGER */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_G_ALARM_MANAGER00
static int tolua_PAC_dev_G_ALARM_MANAGER00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   alarm_manager* tolua_ret = (alarm_manager*)  G_ALARM_MANAGER();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"alarm_manager");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'G_ALARM_MANAGER'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_PAC_dev_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_constant(tolua_S,"AS_NORMAL",AS_NORMAL);
  tolua_constant(tolua_S,"AS_ALARM",AS_ALARM);
  tolua_constant(tolua_S,"AS_RETURN",AS_RETURN);
  tolua_constant(tolua_S,"AS_ACCEPT",AS_ACCEPT);
  tolua_constant(tolua_S,"AT_DISCRETE",AT_DISCRETE);
  tolua_constant(tolua_S,"AT_VALUE",AT_VALUE);
  tolua_constant(tolua_S,"AT_DEVIATION",AT_DEVIATION);
  tolua_constant(tolua_S,"AT_RATE_OF_CHANGE",AT_RATE_OF_CHANGE);
  tolua_constant(tolua_S,"AT_SPECIAL",AT_SPECIAL);
  tolua_constant(tolua_S,"P_FATAL",P_FATAL);
  tolua_constant(tolua_S,"P_ERR_CONNECTION",P_ERR_CONNECTION);
  tolua_constant(tolua_S,"P_ALARM",P_ALARM);
  tolua_constant(tolua_S,"P_MESSAGE",P_MESSAGE);
  tolua_constant(tolua_S,"P_ANSWER",P_ANSWER);
  tolua_constant(tolua_S,"P_REMIND",P_REMIND);
  tolua_cclass(tolua_S,"alarm_manager","alarm_manager","",NULL);
  tolua_beginmodule(tolua_S,"alarm_manager");
   tolua_function(tolua_S,"set_alarm",tolua_PAC_dev_alarm_manager_set_alarm00);
   tolua_function(tolua_S,"set_alarms_id",tolua_PAC_dev_alarm_manager_set_alarms_id00);
   tolua_function(tolua_S,"set_alarms_cnt",tolua_PAC_dev_alarm_manager_set_alarms_cnt00);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"G_ALARM_MANAGER",tolua_PAC_dev_G_ALARM_MANAGER00);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10, 97,108, 97,114,109,115, 32, 61, 32,123,125, 10,102,117,
    110, 99,116,105,111,110, 32,103,101,116, 95, 97,108, 97,114,
    109,115, 40, 32, 80, 65, 67, 95,105,100, 44, 32,101,114,114,
     95,105,100, 32, 41, 10,105,102, 32, 97,108, 97,114,109,115,
     91, 32, 80, 65, 67, 95,105,100, 32, 93, 32,126, 61, 32, 78,
     85, 76, 76, 32,116,104,101,110, 10,105,102, 32,101,114,114,
     95,105,100, 32,126, 61, 32, 97,108, 97,114,109,115, 91, 32,
     80, 65, 67, 95,105,100, 32, 93, 46,105,100, 32,116,104,101,
    110, 10,102,111,114, 32,105,100,120, 44, 32, 97, 32,105,110,
     32,105,112, 97,105,114,115, 40, 32, 97,108, 97,114,109,115,
     91, 32, 80, 65, 67, 95,105,100, 32, 93, 32, 41, 32,100,111,
     10, 71, 95, 65, 76, 65, 82, 77, 95, 77, 65, 78, 65, 71, 69,
     82, 40, 41, 58,115,101,116, 95, 97,108, 97,114,109, 40, 32,
     80, 65, 67, 95,105,100, 44, 32,105,100,120, 32, 45, 32, 49,
     44, 10, 97, 46,116,121,112,101, 32,111,114, 32, 65, 84, 95,
     83, 80, 69, 67, 73, 65, 76, 44, 10, 97, 46,100,101,115, 99,
    114,105,112,116,105,111,110, 32,111,114, 32, 34,237,229,232,
    231,226,229,241,242,237,224,255, 32,238,248,232,225,234,224,
     34, 44, 10, 97, 46,103,114,111,117,112, 32,111,114, 32, 34,
     63, 34, 44, 10, 97, 46,101,110, 97, 98,108,101, 32,111,114,
     32, 48, 44, 10, 97, 46,115,117,112,112,114,101,115,115, 32,
    111,114, 32,102, 97,108,115,101, 44, 10, 97, 46,105,110,104,
    105, 98,105,116, 32,111,114, 32, 48, 44, 10, 97, 46,112,114,
    105,111,114,105,116,121, 32,111,114, 32, 57, 57, 57, 44, 10,
     97, 46,115,116, 97,116,101, 32,111,114, 32, 65, 83, 95, 65,
     76, 65, 82, 77, 44, 10, 80, 65, 67, 95,105,100, 44, 10, 97,
     46,105,100, 95,116,121,112,101, 32,111,114, 32, 48, 44, 10,
     97, 46,105,100, 95,110, 32,111,114, 32, 48, 44, 10, 97, 46,
    105,100, 95,111, 98,106,101, 99,116, 95, 97,108, 97,114,109,
     95,110,117,109, 98,101,114, 32,111,114, 32, 48, 32, 41, 10,
    101,110,100, 10, 71, 95, 65, 76, 65, 82, 77, 95, 77, 65, 78,
     65, 71, 69, 82, 40, 41, 58,115,101,116, 95, 97,108, 97,114,
    109,115, 95,105,100, 40, 32, 80, 65, 67, 95,105,100, 44, 32,
     97,108, 97,114,109,115, 91, 32, 80, 65, 67, 95,105,100, 32,
     93, 46,105,100, 32, 41, 10, 71, 95, 65, 76, 65, 82, 77, 95,
     77, 65, 78, 65, 71, 69, 82, 40, 41, 58,115,101,116, 95, 97,
    108, 97,114,109,115, 95, 99,110,116, 40, 32, 80, 65, 67, 95,
    105,100, 44, 32, 35, 97,108, 97,114,109,115, 91, 32, 80, 65,
     67, 95,105,100, 32, 93, 32, 41, 10,101,110,100, 10,101,110,
    100, 10,101,110,100, 45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 1");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_PAC_dev (lua_State* tolua_S) {
 return tolua_PAC_dev_open(tolua_S);
};
#endif

