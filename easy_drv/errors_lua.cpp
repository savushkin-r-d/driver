/*
** Lua binding: PAC_dev
** Generated automatically by tolua++-1.0.92 on 10/10/11 14:55:15.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_PAC_dev_open (lua_State* tolua_S);

#include "errors.h"
#include "exchange_data.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_alarm (lua_State* tolua_S)
{
 alarm* self = (alarm*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_alarm_id (lua_State* tolua_S)
{
 alarm_id* self = (alarm_id*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"alarm");
 tolua_usertype(tolua_S,"alarm_params");
 tolua_usertype(tolua_S,"alarm_id");
}

/* get function: object_type of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_get_alarm_id_object_type
static int tolua_get_alarm_id_object_type(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_type'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->object_type);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: object_type of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_set_alarm_id_object_type
static int tolua_set_alarm_id_object_type(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_type'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->object_type = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: object_number of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_get_alarm_id_object_number
static int tolua_get_alarm_id_object_number(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_number'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->object_number);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: object_number of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_set_alarm_id_object_number
static int tolua_set_alarm_id_object_number(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_number'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->object_number = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: object_alarm_number of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_get_alarm_id_object_alarm_number
static int tolua_get_alarm_id_object_alarm_number(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_alarm_number'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->object_alarm_number);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: object_alarm_number of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_set_alarm_id_object_alarm_number
static int tolua_set_alarm_id_object_alarm_number(lua_State* tolua_S)
{
  alarm_id* self = (alarm_id*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_alarm_number'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->object_alarm_number = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_id_new00
static int tolua_PAC_dev_alarm_id_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"alarm_id",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   alarm_id* tolua_ret = (alarm_id*)  Mtolua_new((alarm_id)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"alarm_id");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  alarm_id */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_id_new00_local
static int tolua_PAC_dev_alarm_id_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"alarm_id",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   alarm_id* tolua_ret = (alarm_id*)  Mtolua_new((alarm_id)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"alarm_id");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param1 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param1
static int tolua_get_alarm_params_param1(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param1'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param1);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param1 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param1
static int tolua_set_alarm_params_param1(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param1'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param1 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param2 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param2
static int tolua_get_alarm_params_param2(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param2'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param2);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param2 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param2
static int tolua_set_alarm_params_param2(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param2'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param2 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param3 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param3
static int tolua_get_alarm_params_param3(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param3'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param3);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param3 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param3
static int tolua_set_alarm_params_param3(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param3'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param3 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param4 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param4
static int tolua_get_alarm_params_param4(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param4'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param4);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param4 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param4
static int tolua_set_alarm_params_param4(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param4'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param4 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param5 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param5
static int tolua_get_alarm_params_param5(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param5'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param5);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param5 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param5
static int tolua_set_alarm_params_param5(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param5'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param5 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param6 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param6
static int tolua_get_alarm_params_param6(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param6'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param6);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param6 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param6
static int tolua_set_alarm_params_param6(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param6'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param6 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param7 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param7
static int tolua_get_alarm_params_param7(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param7'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param7);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param7 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param7
static int tolua_set_alarm_params_param7(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param7'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param7 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param8 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param8
static int tolua_get_alarm_params_param8(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param8'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param8);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param8 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param8
static int tolua_set_alarm_params_param8(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param8'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param8 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param9 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param9
static int tolua_get_alarm_params_param9(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param9'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param9);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param9 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param9
static int tolua_set_alarm_params_param9(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param9'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param9 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: param10 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params_param10
static int tolua_get_alarm_params_param10(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param10'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->param10);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: param10 of class  alarm_params */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params_param10
static int tolua_set_alarm_params_param10(lua_State* tolua_S)
{
  alarm_params* self = (alarm_params*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'param10'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->param10 = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: params of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_params
static int tolua_get_alarm_params(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'params'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->params,"alarm_params");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: params of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_params
static int tolua_set_alarm_params(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'params'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"alarm_params",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->params = *((alarm_params*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: type of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_type
static int tolua_get_alarm_type(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'type'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->type);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: type of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_type
static int tolua_set_alarm_type(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'type'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->type = ((ALARM_TYPE) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: description of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_description
static int tolua_get_alarm_description(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'description'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->description);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: description of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_description
static int tolua_set_alarm_description(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'description'",NULL);
  if (!tolua_isstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->description = ((char*)  tolua_tostring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: enable of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_unsigned_enable
static int tolua_get_alarm_unsigned_enable(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'enable'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->enable);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: enable of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_unsigned_enable
static int tolua_set_alarm_unsigned_enable(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'enable'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->enable = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: group of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_group
static int tolua_get_alarm_group(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'group'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->group);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: group of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_group
static int tolua_set_alarm_group(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'group'",NULL);
  if (!tolua_isstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->group = ((char*)  tolua_tostring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: inhibit of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_unsigned_inhibit
static int tolua_get_alarm_unsigned_inhibit(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'inhibit'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->inhibit);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: inhibit of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_unsigned_inhibit
static int tolua_set_alarm_unsigned_inhibit(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'inhibit'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->inhibit = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: priority of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_priority
static int tolua_get_alarm_priority(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'priority'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->priority);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: priority of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_priority
static int tolua_set_alarm_priority(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'priority'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->priority = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: state of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_state
static int tolua_get_alarm_state(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'state'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->state);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: state of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_state
static int tolua_set_alarm_state(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'state'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->state = ((ALARM_STATE) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: suppress of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_unsigned_suppress
static int tolua_get_alarm_unsigned_suppress(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'suppress'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->suppress);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: suppress of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_unsigned_suppress
static int tolua_set_alarm_unsigned_suppress(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'suppress'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->suppress = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: id of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_id
static int tolua_get_alarm_id(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'id'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->id,"alarm_id");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: id of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_id
static int tolua_set_alarm_id(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'id'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"alarm_id",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->id = *((alarm_id*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: driver_id of class  alarm */
#ifndef TOLUA_DISABLE_tolua_get_alarm_unsigned_driver_id
static int tolua_get_alarm_unsigned_driver_id(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'driver_id'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->driver_id);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: driver_id of class  alarm */
#ifndef TOLUA_DISABLE_tolua_set_alarm_unsigned_driver_id
static int tolua_set_alarm_unsigned_driver_id(lua_State* tolua_S)
{
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'driver_id'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->driver_id = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  alarm */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_new00
static int tolua_PAC_dev_alarm_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"alarm",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   alarm* tolua_ret = (alarm*)  Mtolua_new((alarm)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"alarm");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  alarm */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_new00_local
static int tolua_PAC_dev_alarm_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"alarm",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   alarm* tolua_ret = (alarm*)  Mtolua_new((alarm)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"alarm");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  alarm */
#ifndef TOLUA_DISABLE_tolua_PAC_dev_alarm_delete00
static int tolua_PAC_dev_alarm_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"alarm",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  alarm* self = (alarm*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
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
  tolua_constant(tolua_S,"PT_SYSTEM",PT_SYSTEM);
  tolua_constant(tolua_S,"PT_CRITICAL",PT_CRITICAL);
  tolua_constant(tolua_S,"PT_IMPOTENT",PT_IMPOTENT);
  tolua_constant(tolua_S,"PT_UNIMPOTENT",PT_UNIMPOTENT);
  tolua_constant(tolua_S,"P_INFORMATIONAL",P_INFORMATIONAL);
  tolua_constant(tolua_S,"AT_DISCRETE",AT_DISCRETE);
  tolua_constant(tolua_S,"AT_VALUE",AT_VALUE);
  tolua_constant(tolua_S,"AT_DEVIATION",AT_DEVIATION);
  tolua_constant(tolua_S,"AT_RATE_OF_CHANGE",AT_RATE_OF_CHANGE);
  tolua_constant(tolua_S,"AT_SPECIAL",AT_SPECIAL);
  tolua_constant(tolua_S,"OT_UNKNOWN",OT_UNKNOWN);
  tolua_constant(tolua_S,"OT_PAC",OT_PAC);
  tolua_constant(tolua_S,"AC_UNKNOWN",AC_UNKNOWN);
  tolua_constant(tolua_S,"AC_NO_CONNECTION",AC_NO_CONNECTION);
  tolua_constant(tolua_S,"AC_COM_DRIVER",AC_COM_DRIVER);
  tolua_constant(tolua_S,"AC_RUNTIME_ERROR",AC_RUNTIME_ERROR);
  tolua_constant(tolua_S,"AS_WAGO",AS_WAGO);
  tolua_constant(tolua_S,"AS_PANEL",AS_PANEL);
  tolua_constant(tolua_S,"AS_MODBUS_DEVICE",AS_MODBUS_DEVICE);
  tolua_constant(tolua_S,"AS_EASYSERVER",AS_EASYSERVER);
  tolua_constant(tolua_S,"AS_EMERGENCY_BUTTON",AS_EMERGENCY_BUTTON);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"alarm_id","alarm_id","",tolua_collect_alarm_id);
  #else
  tolua_cclass(tolua_S,"alarm_id","alarm_id","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"alarm_id");
   tolua_variable(tolua_S,"object_type",tolua_get_alarm_id_object_type,tolua_set_alarm_id_object_type);
   tolua_variable(tolua_S,"object_number",tolua_get_alarm_id_object_number,tolua_set_alarm_id_object_number);
   tolua_variable(tolua_S,"object_alarm_number",tolua_get_alarm_id_object_alarm_number,tolua_set_alarm_id_object_alarm_number);
   tolua_function(tolua_S,"new",tolua_PAC_dev_alarm_id_new00);
   tolua_function(tolua_S,"new_local",tolua_PAC_dev_alarm_id_new00_local);
   tolua_function(tolua_S,".call",tolua_PAC_dev_alarm_id_new00_local);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"alarm_params","alarm_params","",NULL);
  tolua_beginmodule(tolua_S,"alarm_params");
   tolua_variable(tolua_S,"param1",tolua_get_alarm_params_param1,tolua_set_alarm_params_param1);
   tolua_variable(tolua_S,"param2",tolua_get_alarm_params_param2,tolua_set_alarm_params_param2);
   tolua_variable(tolua_S,"param3",tolua_get_alarm_params_param3,tolua_set_alarm_params_param3);
   tolua_variable(tolua_S,"param4",tolua_get_alarm_params_param4,tolua_set_alarm_params_param4);
   tolua_variable(tolua_S,"param5",tolua_get_alarm_params_param5,tolua_set_alarm_params_param5);
   tolua_variable(tolua_S,"param6",tolua_get_alarm_params_param6,tolua_set_alarm_params_param6);
   tolua_variable(tolua_S,"param7",tolua_get_alarm_params_param7,tolua_set_alarm_params_param7);
   tolua_variable(tolua_S,"param8",tolua_get_alarm_params_param8,tolua_set_alarm_params_param8);
   tolua_variable(tolua_S,"param9",tolua_get_alarm_params_param9,tolua_set_alarm_params_param9);
   tolua_variable(tolua_S,"param10",tolua_get_alarm_params_param10,tolua_set_alarm_params_param10);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"alarm","alarm","",tolua_collect_alarm);
  #else
  tolua_cclass(tolua_S,"alarm","alarm","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"alarm");
   tolua_variable(tolua_S,"params",tolua_get_alarm_params,tolua_set_alarm_params);
   tolua_variable(tolua_S,"type",tolua_get_alarm_type,tolua_set_alarm_type);
   tolua_variable(tolua_S,"description",tolua_get_alarm_description,tolua_set_alarm_description);
   tolua_variable(tolua_S,"enable",tolua_get_alarm_unsigned_enable,tolua_set_alarm_unsigned_enable);
   tolua_variable(tolua_S,"group",tolua_get_alarm_group,tolua_set_alarm_group);
   tolua_variable(tolua_S,"inhibit",tolua_get_alarm_unsigned_inhibit,tolua_set_alarm_unsigned_inhibit);
   tolua_variable(tolua_S,"priority",tolua_get_alarm_priority,tolua_set_alarm_priority);
   tolua_variable(tolua_S,"state",tolua_get_alarm_state,tolua_set_alarm_state);
   tolua_variable(tolua_S,"suppress",tolua_get_alarm_unsigned_suppress,tolua_set_alarm_unsigned_suppress);
   tolua_variable(tolua_S,"id",tolua_get_alarm_id,tolua_set_alarm_id);
   tolua_variable(tolua_S,"driver_id",tolua_get_alarm_unsigned_driver_id,tolua_set_alarm_unsigned_driver_id);
   tolua_function(tolua_S,"new",tolua_PAC_dev_alarm_new00);
   tolua_function(tolua_S,"new_local",tolua_PAC_dev_alarm_new00_local);
   tolua_function(tolua_S,".call",tolua_PAC_dev_alarm_new00_local);
   tolua_function(tolua_S,"delete",tolua_PAC_dev_alarm_delete00);
  tolua_endmodule(tolua_S);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10, 97,108, 97,114,109,115, 32, 61, 32,123,125, 10,102,117,
    110, 99,116,105,111,110, 32,103,101,116, 95, 97,108, 97,114,
    109,115, 95, 99,110,116, 40, 32,112,114,111,106,101, 99,116,
     95,100,101,115, 99,114,105,112,116,105,111,110, 95,105,100,
     32, 41, 10,105,102, 32, 97,108, 97,114,109,115, 91, 32,112,
    114,111,106,101, 99,116, 95,100,101,115, 99,114,105,112,116,
    105,111,110, 95,105,100, 32, 93, 32,126, 61, 32, 78, 85, 76,
     76, 32,116,104,101,110, 10,114,101,116,117,114,110, 32, 35,
     97,108, 97,114,109,115, 91, 32,112,114,111,106,101, 99,116,
     95,100,101,115, 99,114,105,112,116,105,111,110, 95,105,100,
     32, 93, 10,101,110,100, 10,114,101,116,117,114,110, 32, 48,
     10,101,110,100, 10,102,117,110, 99,116,105,111,110, 32,103,
    101,116, 95, 97,108, 97,114,109,115, 95,105,100, 40, 32,112,
    114,111,106,101, 99,116, 95,100,101,115, 99,114,105,112,116,
    105,111,110, 95,105,100, 32, 41, 10,105,102, 32, 97,108, 97,
    114,109,115, 91, 32,112,114,111,106,101, 99,116, 95,100,101,
    115, 99,114,105,112,116,105,111,110, 95,105,100, 32, 93, 32,
    126, 61, 32, 78, 85, 76, 76, 32,116,104,101,110, 10,114,101,
    116,117,114,110, 32, 97,108, 97,114,109,115, 91, 32,112,114,
    111,106,101, 99,116, 95,100,101,115, 99,114,105,112,116,105,
    111,110, 95,105,100, 32, 93, 46,105,100, 10,101,110,100, 10,
    114,101,116,117,114,110, 32, 48, 10,101,110,100, 10,102,117,
    110, 99,116,105,111,110, 32,103,101,116, 95, 97,108, 97,114,
    109, 40, 32,112,114,111,106,101, 99,116, 95,100,101,115, 99,
    114,105,112,116,105,111,110, 95,105,100, 44, 32,110, 32, 41,
     10, 97, 32, 61, 32, 78, 85, 76, 76, 10,105,102, 32, 97,108,
     97,114,109,115, 91, 32,112,114,111,106,101, 99,116, 95,100,
    101,115, 99,114,105,112,116,105,111,110, 95,105,100, 32, 93,
     32,126, 61, 32, 78, 85, 76, 76, 32,116,104,101,110, 10, 76,
    117, 97, 95, 97, 32, 61, 32, 97,108, 97,114,109,115, 91, 32,
    112,114,111,106,101, 99,116, 95,100,101,115, 99,114,105,112,
    116,105,111,110, 95,105,100, 32, 93, 91, 32,110, 32, 93, 10,
    105,102, 32, 76,117, 97, 95, 97, 32,126, 61, 32, 78, 85, 76,
     76, 32,116,104,101,110, 10, 97, 32, 61, 32, 97,108, 97,114,
    109, 58,110,101,119, 40, 41, 10, 97, 46,116,121,112,101, 32,
     61, 32, 76,117, 97, 95, 97, 46,116,121,112,101, 32,111,114,
     32, 65, 84, 95, 83, 80, 69, 67, 73, 65, 76, 10, 97, 46,100,
    101,115, 99,114,105,112,116,105,111,110, 32, 61, 32, 76,117,
     97, 95, 97, 46,100,101,115, 99,114,105,112,116,105,111,110,
     32,111,114, 32, 34,237,229,232,231,226,229,241,242,237,224,
    255, 32,238,248,232,225,234,224, 34, 10, 97, 46,101,110, 97,
     98,108,101, 32, 61, 32, 76,117, 97, 95, 97, 46,101,110, 97,
     98,108,101, 32,111,114, 32, 48, 10, 97, 46,103,114,111,117,
    112, 32, 61, 32, 76,117, 97, 95, 97, 46,103,114,111,117,112,
     32,111,114, 32, 34,237,229,232,231,226,229,241,242,237,224,
    255, 32,227,240,243,239,239,224, 34, 10, 97, 46,105,110,104,
    105, 98,105,116, 32, 61, 32, 76,117, 97, 95, 97, 46,105,110,
    104,105, 98,105,116, 32,111,114, 32, 48, 10, 97, 46,112,114,
    105,111,114,105,116,121, 32, 61, 32, 76,117, 97, 95, 97, 46,
    112,114,105,111,114,105,116,121, 32,111,114, 32, 57, 57, 57,
     10, 97, 46,115,116, 97,116,101, 32, 61, 32, 76,117, 97, 95,
     97, 46,115,116, 97,116,101, 32,111,114, 32, 65, 83, 95, 65,
     76, 65, 82, 77, 10, 97, 46,105,100, 32, 61, 32, 76,117, 97,
     95, 97, 46,105,100, 32,111,114, 32, 97,108, 97,114,109, 95,
    105,100, 58,110,101,119, 40, 41, 10, 97, 46,100,114,105,118,
    101,114, 95,105,100, 32, 61, 32,112,114,111,106,101, 99,116,
     95,100,101,115, 99,114,105,112,116,105,111,110, 95,105,100,
     10,101,110,100, 10,101,110,100, 10,114,101,116,117,114,110,
     32, 97, 10,101,110,100, 45, 45, 45, 45,32
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

