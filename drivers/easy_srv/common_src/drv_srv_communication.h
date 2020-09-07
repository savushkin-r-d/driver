/// @file drv_srv_communication.h
/// @brief Содержит описания перечислений, которые используются для обмена
///  между сервисом и драйвером.
///
///
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
///
/// @par Текущая версия:
/// @$Rev: 2162 $.\n
/// @$Author: id $.\n
/// @$Date:: 2015-02-10 11:07:09#$.

#pragma once

enum SRV_CMD
    {
    GET_TAG_VALUE = 1,
    GET_TAG_VALUE_BY_ID,

    SET_TAG_VALUE,

    GET_ALARMS,
    SET_ALARMS,
    };

/// @brief Типы значения тега.
enum TAG_VAL_TYPE
    {
    T_NUMBER,///< Вещественное (float, 32 бита).
    T_STRING,///< Строка.
    };

/// @brief Результат получения значения тега.
enum GET_TAG_RES
    {
    GT_OK,
    GT_ERR,

    GT_NO_TAG_FOUND,
    GT_NEED_FUL_TAG_INFO,    
    };
