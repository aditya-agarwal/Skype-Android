/*
 * Copyright (C) 2006, Skype Limited
 *
 * All intellectual property rights, including but not limited to copyrights,
 * trademarks and patents, as well as know how and trade secrets contained
 * in, relating to, or arising from the internet telephony software of Skype
 * Limited (including its affiliates, "Skype"), including without limitation
 * this source code, Skype API and related material of such software
 * proprietary to Skype and/or its licensors ("IP Rights") are and shall
 * remain the exclusive property of Skype and/or its licensors. The recipient
 * hereby acknowledges and agrees that any unauthorized use of the IP Rights
 * is a violation of intellectual property laws.
 *
 * Skype reserves all rights and may take legal action against infringers of
 * IP Rights.
 *
 * The recipient agrees not to remove, obscure, make illegible or alter any
 * notices or indications of the IP Rights and/or Skype's rights and ownership
 * thereof.
 */

#ifndef __TCPROTO_HPP__
#define __TCPROTO_HPP__

#define TCPROTO_VERSION 3

#define INDEXFROMSTATUS(s) ((s>>8)&0xffff)
#define AVAILABILITYFROMSTATUS(s) (s&0xff)

struct TCPROTO
{

  enum CALLMETHODS {
    CALLMETHODS_UNDEFINED=-1,
    SWITCHEDCIRCUIT=1
  };

  enum GENDERS {
    GENDERS_UNDEFINED=-1,
    MALE=1,
    FEMALE=2
  };

  // bit positions in bitmask
  enum CONTACT_CAPABILITIES {
    CAPABILITY_UNDEFINED=-1,
    CAPABILITY_VOICEMAIL=1,
    CAPABILITY_SKYPEOUT=2,
    CAPABILITY_SKYPEIN=4,
    CAPABILITY_CAN_BE_SENT_VM=8,
    CAPABILITY_CALL_FORWARD=16,
    CAPABILITY_VIDEO=32,
    CAPABILITY_TEXT=64    
  };

  enum CHATPOLICY {
    CHATPOLICY_UNDEFINED=-1,
    EVERYONE_CAN_ADD=0,
    BUDDIES_CAN_ADD,
    AUTHORIZED_CAN_ADD   
  };

  enum SKYPECALLPOLICY {
    SKYPECALLPOLICY_UNDEFINED=-1,
    EVERYONE_CAN_CALL=0,
    BUDDIES_CAN_CALL,
    AUTHORIZED_CAN_CALL,
    RECIEVE_NO_SKYPE_CALLS   
  };

  enum PSTNCALLPOLICY {
    PSTNCALLPOLICY_UNDEFINED=-1,
    ALL_NUMBERS_CAN_CALL=0,
    DISCLOSED_NUMBERS_CAN_CALL,
    BUDDY_NUMBERS_CAN_CALL,
    RECIEVE_NO_PSTN_CALLS   
  };

  enum AVATARPOLICY {
    AVATARPOLICY_UNDEFINED=-1,
    AUTHORIZED_CAN_SEE=0,
    BUDDIES_CAN_SEE,
    EVERYONE_CAN_SEE  
  };

  enum BUDDYCOUNTPOLICY {
    BUDDYCOUNTPOLICY_UNDEFINED=-1,
    DISCLOSE_TO_AUTHORIZED=0,
    DISCLOSE_TO_NOONE
  };

  enum TIMEZONEPOLICY {
    TIMEZONEPOLICY_UNDEFINED=-1,
    TZ_AUTOMATIC=0,
    TZ_MANUAL,
    TZ_UNDISCLOSED
  };

  enum WEBPRESENCEPOLICY {
    WEBPRESENCEPOLICY_UNDEFINED=-1,
    WEBPRESENCE_DISABLED=0,
    WEBPRESENCE_ENABLED 
  };

  enum AVAILABILITY {
    UNKNOWN=0,
    OFFLINE=1,
    ONLINE=2,
    AWAY=3,
    NOT_AVAILABLE=4,
    DO_NOT_DISTURB=5,
    INVISIBLE=6,
    SKYPE_ME=7,
    PENDINGAUTH=8,
    BLOCKED=9,
    SKYPEOUT=10,
    BLOCKED_SKYPEOUT=11,
    OFFLINE_BUT_VM_ABLE=12,
    OFFLINE_BUT_CF_ABLE=13,
    CONNECTING=14,
    // thinclient specific
    DELETED=99
  };

  enum PROFILESTATUS {
    FOREGROUND=0,
    BACKGROUND
  };

  enum CONTACTTYPE {
    UNRECOGNIZED=0,
    SKYPE,
    PSTN,
    EMERGENCY_PSTN,
    FREE_PSTN,
    UNDISCLOSED_PSTN
  };

  enum CALLTYPE {
    INCOMINGSKYPE=0,
    OUTGOINGSKYPE,
    INCOMINGPSTN,
    OUTGOINGPSTN,
//    OUTGOINGVOICEMAIL,
    UNDEFINEDCALLTYPE
  };

  enum BUDDYLISTSTATUS {
    BUDDYLISTSTATUS_UNDEFINED=-1,
    COMPLETE =0,
    INCOMPLETE
  };

  enum CHATTYPE {
    CHATTYPE_UNDEFINED=-1,
    LEGACY_DIALOG=1,
    DIALOG=2,
    MULTICHAT=4,
    LEGACY_UNSUBSCRIBED=5,
    SHAREDGROUP=6  
  };

  enum CHATSTATUS {
    CHATSTATUS_UNDEFINED=-1,
    CONNECTINGTOCHAT=1,
    WAITING_REMOTE_ACCEPT,
    ACCEPT_REQUIRED,
    PASSWORD_REQUIRED,
    SUBSCRIBED,
    UNSUBSCRIBED
  };

  enum CHATLEAVEREASON {
    CHATLEAVEREASON_UNDEFINED=-1,
    USER_NOT_FOUND=1,
    USER_INCAPABLE,
    ADDER_MUST_BE_FRIEND,
    ADDER_MUST_BE_AUTHORIZED,
    DECLINE_ADD,
    UNSUBSCRIBE
  };

  enum CHATMSGTYPE {
    CHATMSGTYPE_UNDEFINED=-1,
    ADDEDMEMBERS=1,
    CREATEDCHATWITH=2,
    SAID=3,
    LEFT,
    SETTOPIC,
    SAWMEMBERS,
    EMOTED,
    POSTEDCONTACTS,
    GAP_IN_CHAT
  };

  enum CHATMSGSTATUS {
    CHATMSGSTATUS_UNDEFINED=-1,
    SENDING=1,
    SENT,
    RECEIVED,
    READ  
  };

  enum SEARCHSTATUS {
    SEARCHSTATUS_UNDEFINED=-1,
    CONSTRUCTION=1,
    PENDING,
    EXTENDABLE,
    FINISHED,
    FAILED 
  };
  
  enum CONTACT_AUTHLEVEL {
    AUTHLEVEL_UNDEFINED=-1,
    NONE=0, 
    AUTHORIZED_BY_ME,
    BLOCKED_BY_ME  
  };
  
  enum ACCOUNT_LOGOUTREASON {
    LOGOUTREASON_UNKNOWN=0,
    LOGOUT_CALLED=1,
    INVALID_SKYPENAME, 
    INVALID_EMAIL, 
    UNACCEPTABLE_PASSWORD, 
    SKYPENAME_TAKEN, 
    REJECTED_AS_UNDERAGE,
    NO_SUCH_IDENTITY, 
    INCORRECT_PASSWORD, 
    TOO_MANY_LOGIN_ATTEMPTS,
    PASSWORD_HAS_CHANGED, 
    INTERNAL_LIBRARY_ERROR
  };
  
  enum INTERNAL_SERVER_ERROR_CODE {
    UNKNOWN_ERROR_CODE = 0,
    GET_DNS_NAME_FAILED =1,     //Failed to get DNS name 
    CLIENT_REDIRECT_FAILED,     //Failed to get redirect address 
    GET_DIRECTORY_ADDR_FAILED,  //Failed to get directory address 
    UNABLE_ACQUIRE_SL_INSTACE,  //Unable to acquire sl_instance for new client
    UNEXPECTED_SKYLIB_ACCOUNT_STATUS, //Skylib event internal_error CLE_INTERNAL_SKYLIB_ERROR
    GET_PRECHECK_CM_PARAM_FAILED,//Failed to get parameters after CM pre-check
    GET_REGISTR_CM_PARAM_FAILED,//Failed to get parameters after CM registration
    LS_GET_BEST_HOST_FAILED,    //LocalStorage:: failed to get best host 
    LS_CREATE_SESSION_FAILED    //LocalStorage:: failed to create session   
  };
  
  enum CHUNK {
    GETPRESENCEUPDATE=0,
    PRESENCEUPDATE,
    CHANGEONLINESTATUS,
    CONTACTONLINESTATUS,
    GETBALANCE,
    BALANCE,
    GETCALLHISTORY,
    CALLHISTORY,
    CALLRECORD,
    GETMISSEDCALLS,
    MISSEDCALLS,
    CLEARCALLHISTORY,
    PLACECALL, // V2+ <- SETPSTNNUMBER,V1
    SETPSTNNUMBER = PLACECALL, 
    RESOLVEPSTNNUMBER,
    CALLINGBUDDY,
    GETVOICEMAILS,
    VOICEMAILS,
    VOICEMAIL,
    DELETEVOICEMAIL,
    CONNECT,
    CONNECTED,
    MASTERKEY,
    SERVERIDENTITY,
    CLIENTDATA,
    CALLSETUP,
    REDIRECT,
    DISCONNECT,
    ERROR,
    LOGIN,
    REGISTERUSER,
    GETPROFILEDATA,
    SETPROFILEDATA,
    PROFILEDATA,
    CHANGEPASSWORD,
    CHANGEDPASSWORD,
    LOGOUT,
    GETBUDDYLIST,
    BUDDYLIST,
    BUDDY,
    ADDCONTACT,
    SENDAUTHREQUEST,
    INCOMINGAUTHREQUEST,
    DELETECONTACT,
    AUTHORIZECONTACT,
    BLOCKCONTACT,
    GETCONTACTPROPERTIES,
    CONTACTPROPERTIES,
    GETAVATAR,
    AVATAR,
    CONTACTSEARCH,
    CONTACTSEARCHRESULTS,
    SEARCHRESULT,
    EXTENDCONTACTSEARCH,
    ENDCONTACTSEARCH,
    GETRECENTCHATS,
    RECENTCHATS,
    GETBOOKMARKEDCHATS,
    BOOKMARKEDCHATS,
    GETUNREADCHATS,
    UNREADCHATS,
    CHATRECORD,
    GETCHATINFO,
    CHATINFO,
    GETCHATMESSAGES,
    CHATMESSAGES,
    CHATMESSAGE,
    SENDCHATMESSAGE,
    CREATECHATWITHUSER,
    CREATEDCHAT,
    ADDCHATMEMBERS,
    LEAVECHAT,
    SETCHATTOPIC,
    SENDSMS,
    LOGGEDIN,
    LOGGEDOUT,
    USEPSTNNUMBER,
    SAVEDATA,
    // V2+
    USERDATA, // placed here to provide numbering compatibility with V2 protocol version
// V3+
    WARNING,    
    GIVEDISPLAYNAME,
    PRESENCESETTINGS,
    LISTENVOICEMAIL,
    BOOKMARKCHAT,
    MARKCHATSREAD,
    MARKEDCHATSREAD,
            
    EXTENDEDCHUNKSBEGIN=256,  // define your own from here on
            
  };

  // attribute keys used in chunks
  // most often used keys are listed first
  //
  enum ATTRKEY {
    // These attribute key values idendify key components in key containers.
    ATTR_ROOT_PRIVATE_KEY = 0,
    ATTR_ROOT_PUBLIC_KEY,
    ATTR_SERVER_PRIVATE_KEY,
    ATTR_SERVER_PUBLIC_KEY,
    ATTR_SERVER_SIGNED_PUBLIC_KEY,
    // The above attribute values were placed here to avoid frequent regenarations
    // of rootpriv.key, rootpub.key, server.key crypto keys when new changes
    // are applied to this enum.
    
    CHUNKTYPE,
    PACKETTYPE=CHUNKTYPE,
    BUDDYINDEX,
    TIMESTAMP,
    SKYPENAME,
    FULLNAME,
    
    CONTACTAVAILABILITY,
    BOOKMARK,
    ONLINESTATUSES,
    RECORDCOUNT,
    BALANCE_CURRENCY,
    BALANCE_AMOUNT,
    POLL_INITIATOR,
    NOTIFY_NODES,
    PSTNNUMBER,
// https://wiki.skype.com/FatServer/ThinclientProtocol
// changed attribute names, these will be currently aliases (map the same enum value)
// CHUNKTYPE - PACKETTYPE PSTNNUMBER - CALLTARGET 
    CALLTARGET = PSTNNUMBER, //V2
    
    CHAT,
    CHAT_FRIENDLYNAME,
    UNREAD_MSGS,
    CHAT_NAME,
    ACTIVITY_TIMESTAMP,
    TYPE,
    MEMBERS,
    TOPIC,
    CHATMSG_BODY,
    CHATMSG_LEAVEREASON,
    
    CLIENTCHATID,
    AUTHMSG,
    AUTHORIZATIONTYPE,
    PSTNNUMBERS,
    SMS_MESSAGE,
    BIRTHDAY,
    GENDER,
    LANGUAGES,
    COUNTRY,
    PROVINCE,
    
    CITY,
    PHONE_HOME,
    PHONE_OFFICE,
    PHONE_MOBILE,
    EMAILS,
    HOMEPAGE,
    ABOUT,
    MOOD_TEXT,
    TIMEZONE,
    CONTACTCAPABILITIES,
    
    NROF_AUTHED_BUDDIES,
    IPCOUNTRY,
    UNDER_LEGAL_AGE,
    SKYPEIN_NUMBERS,
    OFFLINE_CALLFORWARD,
    CHAT_POLICY,
    SKYPE_CALL_POLICY,
    PSTN_CALL_POLICY,
    AVATAR_POLICY,
    BUDDYCOUNT_POLICY,
    
    TIMEZONE_POLICY,
    WEBPRESENCE_POLICY,
    SESSION_ID,
    MAXENTRIES,
    MAXRECORDS,
    PASSWORD, 
    DEVICE_PSTNNUMBER,
    EMAIL,
    AVATARIMAGE,
    AVATARWIDTH,
    
    AVATARHEIGHT,
    SEARCHSTRING,
    SEARCH_ID,
    VMINDEX,
    SUBJECT,
    DURATION,
    PROTOCOLVERSION,
    CLIENT_RANDOM,
    PLATFORMID,
    SERVER_RANDOM,
    
    SECRETMASTER,
    CAPABILITIES,
    LANGUAGE,
    SERVER_PUBLIC_KEY,
    CRYPTO_SETUP,
    SERVER_CRYPTO_KEY,
    CLIENT_PRIVATE_IP_ADDRESS,
    CLIENT_CRYPTO_KEY,
    CLIENT_PUBLIC_IP_ADDRESS,
    LATEST_VERSION,
    
    UPGRADEURL,
    SERVER_NAME,
    SERVER_IDENTITY,
    CALLMETHOD,
    DIALIN_NUMBER_PREFIX,
    DIALIN_NUMBER_RANGE_SIZE,
    VOICEMAIL_NUMBER_PREFIX,
    VOICEMAIL_NUMBER_RANGE_SIZE,
    NEW_SERVER,
    ERROR_CODE,
    
    ERROR_MESSAGE,
    ALLOWSPAM,
    NEWPASSWORD,
    VMRECORDS,
    BUDDYRECORDS,
    BUDDYLIST_STATUS,
    SEARCHRESULTRECORDS,
    SEARCH_STATUS,
    CHATRECORDS,
    CHAT_RECORD,    
    
    CHATMSG_RECORDS,
    CALLRECORDS,
    CHATMSG_TYPE,
    CHATMSG_STATUS,
    PROFILE_TIMESTAMP,
    CALLPSTNNUMBER,
    AUTOSYNC,    
    UNREAD_COUNT,    
    SAVEPASSWORD,
    SAVEDDATA,
    
    LOCALPRIVS,
    CREDENTIALS,
    // V2+
    NODEID,
    SDFSESSIONID,
    ENCRYPTED_USERDATA,
    LOGOUTREASON,
    SUGGESTED_SKYPENAMES,
    // V3+
    CALLMETHODS,
    DEVICE_MCC,
    DEVICE_MNC,
    
    HOME_MCC,
    HOME_MNC,
    WARNING_CODE,
    WARNING_MESSAGE,
    HELP_URL,
    SI_GATEWAY_NAME,
    SI_GATEWAY_NUMBER,
    SO_ENABLED,
    CALLING_URI_EXTENSIONS_ENABLED,
    PUSHIDENTIFIER,
    
    DISPLAYNAME,
    FG_POLL_INTERVAL,
    BG_POLL_INTERVAL,
    CLIENTSTATE, 
    SETBOOKMARK,
    SERVER_UTC_TIME,
    REQUEST_ID,
    
    //attributes of CONNECTED chunk
    SUBSCRIPTION_ERROR_MESSAGE,
    SUBSCRIPTION_LSK,
    SUBSCRIPTION_URL,

    UPGRADE_MESSAGE,
    UPGRADE_LSK,
    CHATS,
    POLL_TARGET,
    POLL_COOKIE,

    
    EXTENDEDKEYSBEGIN=256,  // define your own from here on
            
  };//ATTRKEY

  enum CL_ERROR {
    CLE_NO_ERROR = -1,  // No error. Internal FS usage
    // Generic errors
    CLE_UNKNOWN_ERROR = 0, // Unknown server error
    CLE_INCORRECT_BUNDLE, // An incorrect bundle received
    CLE_INVALID_KEY_LENGTH, // Invalid RC4 key length
    CLE_INVALID_PUBLIC_KEY_LENGTH, // Invalid RSA public key
    CLE_NO_BALANCE_CURRENCY, // Unable to extract balance currency
    CLE_REQUEST_PLACEMENT, // Unable to place request
    CLE_NONEXISTING_CHAT, // Received an id of a non-existing chat
    CLE_INVALID_CHAT_OR_MEMBERS, // Invalid chat or some skype_name is invalid/blocked
    CLE_NO_CONTACT_FOUND, // Failed to find a contact
    CLE_INVALID_IDENTITY, // Invalid user name
    CLE_CONTACT_EXISTS, // Contact already exists
    CLE_INVALID_BUDDY_INDEX, // Invalid buddy index (buddy doesn't exist)
    CLE_INVALID_SEARCH_ID, // Invalid contact search index
    CLE_WRONG_AVAILABILITY, // Wrong availability status
    CLE_CHAT_OPEN_FAILED,   // Failed to open a chat
    CLE_NEGATIVE_BOOKMARK, // Negative bookmark
    CLE_MAX_BUDDYLIST_SIZE_EXCEEDED, //Max buddylist size exceeded
    CLE_CBL_SYNC_FAILURE, // Failed to synchronize with CBL
    CLE_PROFILEDATA_NOT_READY, // Profile data is not ready
    CLE_BALANCE_NOT_READY, // Balance data is not ready
    CLE_AVATAR_CONVERSION_FAILED, // Failed to convert avatar from jpg to png
    CLE_INVALID_MAX_ENTRIES_VAL, // Invalid max entries value
    CLE_INVALID_VM_INDEX, // Invalid voice mail index
    CLE_CALL_TRANSFER_FAILED, // Call transfer to a client failed
    CLE_INVALID_DEV_PSTN_NUM, // Invalid device PSTN number
    CLE_PROFILE_UPDATE_FAILURE, // Profile update failure
    CLE_FORCED_LOGOUT, // Server forced logout
    CLE_INVALID_AUTH_LEVEL, // Invalid authorization level

    // Password syntax check result
    CLE_PWD_TOO_LONG, // Password is too long
    CLE_PWD_TOO_SHORT, // Password is too short
    CLE_PWD_CONTAINS_INVALID_CHAR, // Password contains invalid char
    CLE_PWD_CONTAINS_SPACE, // Password contains space
    CLE_PWD_SAME_AS_USERNAME, // Password is same to user name

    // Password change errors
    CLE_PWD_INVALID_OLD_PASSWORD, // Invalid old password
    CLE_PWD_SERVER_CONNECT_FAILED, // Server connect failed
    CLE_PWD_OK_BUT_CHANGE_SUGGESTED, // Password change suggested
    CLE_PWD_MUST_DIFFER_FROM_OLD, // Password must differ from old
    CLE_PWD_INVALID_NEW_PWD, // Invalid new password
    CLE_PWD_MUST_LOG_IN_TO_CHANGE, // Must login to change

    // Login/Register user errors
    CLE_HTTPS_PROXY_AUTH_FAILED, // HTTPS proxy authenficitaion failed
    CLE_SOCKS_PROXY_AUTH_FAILED, // SOCKS proxy authenficitaion failed
    CLE_P2P_CONNECT_FAILED, // Failed to connect to P2P network
    CLE_SERVER_CONNECT_FAILED, // Failed to connect to Skype server
    CLE_SERVER_OVERLOADED, // Skype server is overloaded
    CLE_INVALID_SKYPENAME, // Invalid skype name
    CLE_INVALID_EMAIL, // Invalid e-mail
    CLE_UNACCEPTABLE_PASSWORD, // Unacceptable password
    CLE_SKYPENAME_TAKEN, // Skype name taken
    CLE_REJECTED_AS_UNDERAGE, // Rejected as underage
    CLE_NO_SUCH_IDENTITY, // No such identity
    CLE_INCORRECT_PASSWORD, // Incorrect password
    CLE_TOO_MANY_LOGIN_ATTEMPTS, // Too many login attempts
    CLE_DB_FAILURE, // DB failure
    CLE_PASSWORD_HAS_CHANGED, // Password has changed
    CLE_PERIODIC_UIC_UPDATE_FAILED, // Periodic UIC update failed

    // Identity check result
    CLE_IDENTITY_EMPTY, // Identity is empty
    CLE_IDENTITY_TOO_LONG, // Identity is too long
    CLE_IDENTITY_CONTAINS_INVALID_CHAR, // Identity contains invalid character
    CLE_PSTN_NUMBER_TOO_SHORT, // PSTN number is too short
    CLE_PSTN_NUMBER_HAS_INVALID_PREFIX, // PSTN number has invalid prefix
    CLE_SKYPENAME_STARTS_WITH_NONALPHA, // Skype name starts with non-alpha
    CLE_SKYPENAME_SHORTER_THAN_6_CHARS, // Skype name is shorter than 6 chars

    CLE_INVALID_CLIENT_RANDOM_SIZE, //Invalid client random size
    CLE_INVALID_MASTERKEY_SIZE, //Invalid master key size
    
    CLE_INTERNAL_SERVER_ERROR, // Internal Server error
    CLE_SET_TOPIC_FAILED, // Set topic failed, no rights or text too big
    CLE_UNSUBSCRIBED_CHAT, // Chat is already unsubscribed
    CLE_SENDING_AUTH_REQUEST_FAILED, // Sending authorization request failed
    CLE_CM_REGISTR_FAILED_NO_ACCESS,    //CM registration failed because of NO_ACCESS
    CLE_CM_REGISTR_FAILED_BLOCKED_USER,  //CM registration failed because of BLOCKED_USER
    CLE_BOOKMARK_VALUE_UNEXPECTED,   // Unexpected bookmark value

    CLE_UNSUPPORTED_PACKAGE, // server received unsupported chunk from client, maybe client uses newer version of protocol
    CLE_SEND_SMS_FAILED, // Failed to send SMS
    CLE_INVALID_SMS_BODY, // Invalid SMS body
    CLE_INVALID_SMS_PSTNS, // Ivalid SMS PSTN numbers

    CLE_INVALID_SKYPENAME_OR_PSTN, // Invalid SkypeName or PSTN
    CLE_INVALID_NODEID_SIZE, 
    CLE_INCORRECT_IMAGE_PARAMS, //Failed to get avatar: incorrect avatar parameters
    CLE_INVALID_DEVICE_PLATFORM_IDENTIFIER, // Can't find platform identifier
    CLE_UNSUPPORTED_PROTOCOL_VERSION,  // Unsupported client version

    CLE_PRECHECK_CM_REGISTR_ERROR,   //Pre-check CM registration error
    CLE_INVALID_CLIENT_STATE, // Invalid client state 
    CLE_CM_RESOLVE_CALLER_FAILED, //Failed to resolve caller by Call Manager
    CLE_CM_GET_ACCESS_NUMBER_FAILED, //Failed to get access number
    CLE_CM_GET_VM_ACCESS_NUMBER_FAILED, //Failed to get voice mails access number
    CLE_GIVE_DISPLAY_NAME_FAILED, //Failed to set contact display name
    CLE_INTERNAL_SKYLIB_ERROR, //Internal SkyLib error
    CLE_BLACKLIST_NETWORK_ROAMING, // Client is in a blacklisted network    
    
    CLE_MAX_ERRORS
  };
  
  enum CL_WARN {
    CLW_NO_WARN = -1, 
    CLW_EMPTY_CALLMETHODS_LIST_WARN, // No callmethods sent, calls may be disabled
    CLW_GREYLIST_NETWORK_ROAMING, // Client is in a greylisted network  
    
    CLW_MAX_WARNS
  };
  
};
#endif
