#include "IPBtypes.h"

/**
 * @brief Centralized error reporting. Translates status codes to text.
 */
const char* IPBstatusToString(IPBstatus status) {
    switch (status) {
        case IPB_OK:                        return "Success";
        
        // generic system errors
        case IPB_ERROR_INVALID_ARGUMENT:    return "Invalid argument (empty id or NULL?)";
        case IPB_ERROR_INVALID_PACKET:      return "Invalid packet structure";
        case IPB_ERROR_INVALID_PACKET_TYPE: return "Unknown packet type";
        case IPB_ERROR_ARGUMENT_TOO_BIG:    return "Argument too large";
        case IPB_ERROR_ARGUMENT_LENGTH_MISMATCH: return "Argument length mismatch";
        case IPB_ERROR_ARGUMENT_BAD_CHARS:  return "Argument contains forbidden characters";
        case IPB_ERROR_BUFFER_TOO_SMALL:    return "Buffer too small";
        case IPB_ERROR_MEMORY_ALLOCATION:   return "Memory allocation failed";
        case IPB_ERROR_IO_FILE:             return "File i/o error";
        case IPB_ERROR_NO_STREAMS:			return "No streams found";
        // logic errors
        case IPB_ERROR_USER_NOT_FOUND:      return "User not found";
        case IPB_ERROR_USER_ALREADY_REGISTERED: return "User already registered";
        case IPB_ERROR_MAX_REACHED:         return "Server full (max clients/streams reached)";
        case IPB_ERROR_PASS_MISMATCH:       return "Incorrect password";
        case IPB_ERROR_ALREADY_FRIENDS:     return "Already friends";
        
        // parser errors
        case IPB_ERROR_MALFORMED_RESPONSE:  return "Server sent malformed data";
        case IPB_ERROR_MALFORMED_MSG:       return "Message malformed";
        
        // network errors
        case IPB_ERROR_CONN_FAILED:         return "Connection failed";
        case IPB_ERROR_SEND_FAILED:         return "Network send failed";
        case IPB_ERROR_RECV_FAILED:         return "Network receive failed";
        case IPB_ERROR_SOCKET_SETUP:        return "Socket initialization failed";
        
        // server response errors
        case IPB_ERROR_SERVER_REJECTED:     return "Operation rejected by server";
        
        case IPB_ERROR_UNKNOWN:             return "Unknown error";
        default:                            return "Unrecognized status code";
    }
}