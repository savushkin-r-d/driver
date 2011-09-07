/*
** Lua binding: PAC_dev
** Generated automatically by tolua++-1.0.92 on 09/07/11 15:44:02.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_PAC_dev_open (lua_State* tolua_S);

#include "errors.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

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
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_PAC_dev (lua_State* tolua_S) {
 return tolua_PAC_dev_open(tolua_S);
};
#endif

