/**
 * @file ARNETWORKAL_Error.c
 * @brief informations about libARNetwork errors
 * @date 04/25/2013
 * @author frederic.dhaeyer@parrot.com
 */

#include <libARNetworkAL/ARNETWORKAL_Error.h>

//
// To generate the function using emacs regexp syntax
// - remove all cases (except for default)
// - copy the content (no braces) of eARNETWORKAL_ERROR enum
// - replace rexexp (M-x replace-regexp) with the following parameters
//   - Source: [\ \t]*\([A-Z_]*\)[-0-9\ \t,=]*/\*\*<\ \(.*\)\ \*/
//   - Dest  : case \1:^Jreturn "\2";^J;break;
// - Reindent all lines in file
//
// Note: ^J character is the newline character, C-q C-j in emacs
//
char* ARNETWORKAL_Error_ToString (eARNETWORKAL_ERROR error)
{
    switch (error)
    {
    case ARNETWORKAL_OK:
        return "No error";
        break;
    case ARNETWORKAL_ERROR:
        return "Unknown generic error";
        break;
    case ARNETWORKAL_ERROR_ALLOC:
        return "Memory allocation error";
        break;
    case ARNETWORKAL_ERROR_BAD_PARAMETER:
        return "Bad parameters";
        break;
    case ARNETWORKAL_ERROR_MANAGER:
        return "Unknown ARNETWORK_Manager error";
        break;
    case ARNETWORKAL_ERROR_WIFI:
        return "Unknown wifi error";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION:
        return "Creation socket error";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED:
        return "Permission denied on a socket";
        break;
    case ARNETWORKAL_ERROR_BLE_CONNECTION:
        return "Unknown BLE error";
        break;
    default:
        return "Unknown error";
        break;
    }
    return "Unknown error";
}
