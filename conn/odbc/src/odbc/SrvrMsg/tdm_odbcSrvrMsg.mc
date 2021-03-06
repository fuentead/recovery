;/**********************************************************************
;// @@@ START COPYRIGHT @@@
;//
;//        HP CONFIDENTIAL: NEED TO KNOW ONLY
;//
;//        Copyright 2003
;//        Hewlett-Packard Development Company, L.P.
;//        Protected as an unpublished work.
;//
;//  The computer program listings, specifications and documentation 
;//  herein are the property of Hewlett-Packard Development Company,
;//  L.P., or a third party supplier and shall not be reproduced, 
;//  copied, disclosed, or used in whole or in part for any reason 
;//  without the prior express written permission of Hewlett-Packard 
;//  Development Company, L.P.
;//
;// @@@ END COPYRIGHT @@@ 
;********************************************************************/
;// @@@ START COPYRIGHT @@@
;//
;//  Copyright 1998
;//  Compaq Computer Corporation
;//     Protected as an unpublished work.
;//        All rights reserved.
;//
;//  The computer program listings, specifications, and documentation
;//  herein are the property of Compaq Computer Corporation or a
;//  third party supplier and shall not be reproduced, copied,
;//  disclosed, or used in whole or in part for any reason without
;//  the prior express written permission of Compaq Computer
;//  Corporation.
;//
;// @@@ END COPYRIGHT @@@ 
;//****************************************************************************
;//*
;//*  Header of the Messages file
;//*
;//****************************************************************************

;//* MessageIdTypeDef = A symbolic name that is output as the typedef name 
;//* for each numeric MessageId value
MessageIdTypedef=DWORD	

;//* SeverityNames = Two most significant bits in the MessageId which define 
;//* the type of message
SeverityNames=	(Success=0x0 
				Informational=0x1 
				Warning=0x2 
				Error=0x3)

;//* FacilityNames = Defines the set of names that are allowed as the value 
;//* of the Facility keyword
FacilityNames=	(System=0x0FF 
;//			Application=0xFFF
			AssociationServer=0x001
			ConfigurationServer=0x002
			ODBCDriver=0x003
			ODBCServer=0x004
			ClusterAdministrator=0x005
			ConfigurationClient=0x006) 

;//* LanguageNames = Defines the set of names that are allowed as the value 
;//* of the language keyword.  It maps to the message file for that language
;//LanguageNames=(USEnglish=9:tdm_odbcSrvrMsg_009)

;//**** end of header section; what follows are the message definitions *****
;//*
;//*  All error message definitions start with the keyword "MessageId"
;//*  if no value is specified, the number will be the last number used for the
;//*  facility plus one.  Instead of providing a number, we can provide +NUMBER
;//*  where number is the offset to be added to the last number used in 
;//*  the facility
;//*  MessageId numbers are limited to 16 bit values
;//*  
;//*  Severity and Facility if not specified will use the last option selected
;//*  the names used must match the names defined in the header
;//*  
;//*  SymbolicName is a symbols used to associate a C symbolic ocnstant name
;//*  with the final 32-bit message code that is the result of ORing together
;//*  the MessageId | Severity | Facility bits.  The constant definition is
;//*  output in the generated .h file
;//*
;//*  After the message definition keywords comes one or more message text
;//*  definitions.  Each message text definition starts with the "Language"
;//*  keyword that identifies which binary output file the message text is 
;//*  to be output to.  The message text is terminated by a line containing
;//*  a single period at the beginning of the line, immediately followed by
;//*  a new line.  No spaces allowed around keyword.  Within the message text,
;//*  blank lines and white space are preserved as part of the message.
;//*
;//*  Escape sequences supported in the message text:
;//*    %0 - terminates a message text line without a trailing new line 
;//*        (for prompts)
;//*    %n!printf format string! - Identifies an insert (parameter); 
;//*         n can be between 1 and 99; defaults to !s!
;//*  Inserts refer to parameters used with FormatMessage API call, 
;//*    if not passed, an error is generated
;//*    %% - a single percent sign
;//*    %n - a hard line break
;//*    %r - output a hard carriage return
;//*    %b - space in the formatted message text
;//*    %t - output a tab
;//*    %. - a single period
;//*    %! - a single exclamation point
;//*
;//*
;//*  Values are 32 bit values layed out as follows:
;//*
;//*   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//*   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//*  +---+-+-+-----------------------+-------------------------------+
;//*  |Sev|C|R|     Facility          |               Code            |
;//*  +---+-+-+-----------------------+-------------------------------+
;//*
;//*  where
;//*
;//*      Sev - is the severity code
;//*
;//*          00 - Success
;//*          01 - Informational
;//*          10 - Warning
;//*          11 - Error
;//*
;//*      C - is the Customer code flag
;//*
;//*      R - is a reserved bit
;//*
;//*      Facility - is the facility code
;//*
;//*      Code - is the facility's status code
;//*
;//*************************************************************************
;//*
;//*      Actual Messages follow this line and have the following 
;//*      structure:
;//*          Characters 1 to 5  of the Msg Text will contain SQLState
;//*          Characters 7 to 10 of the Msg Text will contain a Help ID number
;//*          Characters from 11 to the end of the Msg Text will contain the text
;//*
;//*************************************************************************


;//************************************************************************
;//*
;//*           Server Component messages from the string table
;//*
;//************************************************************************

;//************************************************************************
;//*
;//*	MessageId:	CFG_CANNOT_DROP_DEFAULT_DSN(1)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 1
SymbolicName = CFG_CANNOT_DROP_DEFAULT_DSN
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz Unable to delete Default DataSource.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_NO_DATA_FOUND(2)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 2
SymbolicName = CFG_NO_DATA_FOUND
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz No data found for this type of record.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_NO_DATASOURCE(3)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 3
SymbolicName = CFG_NO_DATASOURCE
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz No DataSource is defined in Configuration Database.%0
.

;//************************************************************************
;//*
;//*	MessageId:	IDS_CFG_006(6)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 6
SymbolicName = IDS_CFG_006
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz Please make sure that Initial value is not greater than Maximum value.%0
.

;//************************************************************************
;//*
;//*	MessageId:	IDS_CFG_007(7)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 7
SymbolicName = IDS_CFG_007
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz Please make sure that Available value is not greater than Maximum value.%0
.

;//************************************************************************
;//*
;//*	MessageId:	IDS_CFG_008(8)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 8
SymbolicName = IDS_CFG_008
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz Please make sure that Start Ahead value is not greater than Maximum value.%0
.

;//************************************************************************
;//*
;//*	MessageId:	IDS_CFG_028(28)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 28
SymbolicName = IDS_CFG_028
Severity=Error
Facility=ConfigurationServer
Language=English
zzzzz zzzzz The MX Connectivity Service cannot be started.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_NETWORK_ERROR(50)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 50
SymbolicName = CFG_DSN_LIST_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name List due to a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_SQL_WARNING(51)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 51
SymbolicName = CFG_DSN_LIST_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL Warning has been detected while retrieving the Data Source Name List: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_PARAM_ERROR(52)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 52
SymbolicName = CFG_DSN_LIST_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving the Data Source Name List: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_SQL_ERROR(53)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 53
SymbolicName = CFG_DSN_LIST_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving the Data Source Name List: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_NO_CFG_SRVR(54)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 54
SymbolicName = CFG_DSN_LIST_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name List because there is no Configuration Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_LIST_CFG_SRVR_GONE(55)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 55
SymbolicName = CFG_DSN_LIST_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name List because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_NETWORK_ERROR(60)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 60
SymbolicName = CFG_DSN_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve data associated with the Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_SQL_WARNING(61)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 61
SymbolicName = CFG_DSN_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieve data associated with the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_PARAM_ERROR(62)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 62
SymbolicName = CFG_DSN_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving data associated with the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_SQL_ERROR(63)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 63
SymbolicName = CFG_DSN_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving data associated with the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_NO_CFG_SRVR(64)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 64
SymbolicName = CFG_DSN_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve data associated with the Data Source Name [%1!s!] because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DSN_CFG_SRVR_GONE(65)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 65
SymbolicName = CFG_DSN_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve data associated with the Data Source Name [%1!s!] because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_NETWORK_ERROR(70)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 70
SymbolicName = CFG_SET_DSN_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source Name [%1!s!] changes because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_SQL_WARNING(71)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 71
SymbolicName = CFG_SET_DSN_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while applying Data Source Name [%1!s!] changes: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_PARAM_ERROR(72)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 72
SymbolicName = CFG_SET_DSN_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while applying Data Source Name [%1!s!] changes: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_SQL_ERROR(73)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 73
SymbolicName = CFG_SET_DSN_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while applying Data Source Name [%1!s!] changes: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_NO_CFG_SRVR(74)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 74
SymbolicName = CFG_SET_DSN_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source Name [%1!s!] changes because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CFG_SRVR_GONE(75)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 75
SymbolicName = CFG_SET_DSN_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source Name [%1!s!] changes because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_NETWORK_ERROR(80)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 80
SymbolicName = CFG_ADD_DSN_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to add Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_SQL_WARNING(81)
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 81
SymbolicName = CFG_ADD_DSN_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while adding the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_PARAM_ERROR(82)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 82
SymbolicName = CFG_ADD_DSN_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while adding the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_SQL_ERROR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 83
SymbolicName = CFG_ADD_DSN_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while adding the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_NO_CFG_SRVR(84)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 84
SymbolicName = CFG_ADD_DSN_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to add Data Source Name [%1!s!] because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_ADD_DSN_CFG_SRVR_GONE(85)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 85
SymbolicName = CFG_ADD_DSN_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to add Data Source Name [%1!s!] because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_NETWORK_ERROR(90)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 90
SymbolicName = CFG_CHECK_DSN_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_SQL_WARNING(91)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 91
SymbolicName = CFG_CHECK_DSN_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_PARAM_ERROR(92)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 92
SymbolicName = CFG_CHECK_DSN_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_SQL_ERROR(93)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 93
SymbolicName = CFG_CHECK_DSN_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_NO_CFG_SRVR(94)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 94
SymbolicName = CFG_CHECK_DSN_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name [%1!s!] because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_CHECK_DSN_CFG_SRVR_GONE(95)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 95
SymbolicName = CFG_CHECK_DSN_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source Name [%1!s!] because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_NETWORK_ERROR(100)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 100
SymbolicName = CFG_DROP_DSN_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to drop the Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_SQL_WARNING
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 101
SymbolicName = CFG_DROP_DSN_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while dropping the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_PARAM_ERROR(102)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 102
SymbolicName = CFG_DROP_DSN_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while dropping the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_SQL_ERROR(103)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 103
SymbolicName = CFG_DROP_DSN_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while dropping the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_NO_CFG_SRVR(104)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 104
SymbolicName = CFG_DROP_DSN_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to drop the Data Source Name [%1!s!] because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DROP_DSN_CFG_SRVR_GONE(105)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 105
SymbolicName = CFG_DROP_DSN_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to drop the Data Source Name [%1!s!] because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_NETWORK_ERROR(110)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 110
SymbolicName = CFG_GET_ENV_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Environment Variables because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_SQL_WARNING(111)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 111
SymbolicName = CFG_GET_ENV_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving Environment Variables: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_PARAM_ERROR(112)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 112
SymbolicName = CFG_GET_ENV_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving Environment Variables: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_SQL_ERROR(113)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 113
SymbolicName = CFG_GET_ENV_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving Environment Variables: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_NO_CFG_SRVR(114)
;//*	Severity	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 114
SymbolicName = CFG_GET_ENV_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Environment Variables because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_ENV_CFG_SRVR_GONE(115)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 115
SymbolicName = CFG_GET_ENV_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Environment Variables because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_NETWORK_ERROR(120)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 120
SymbolicName = CFG_SET_ENV_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Environment Variables changes because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_SQL_WARNING(121)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 121
SymbolicName = CFG_SET_ENV_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while applying Environment Variables changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_PARAM_ERROR(122)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 122
SymbolicName = CFG_SET_ENV_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while applying Environment Variables changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_SQL_ERROR(123)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 123
SymbolicName = CFG_SET_ENV_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while applying Environment Variables changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_NO_CFG_SRVR(124)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 124
SymbolicName = CFG_SET_ENV_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Environment Variables changes because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_ENV_CFG_SRVR_GONE(125)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 125
SymbolicName = CFG_SET_ENV_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Environment Variables changes because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_NETWORK_ERROR(130)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 130
SymbolicName = CFG_GET_RESOURCE_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Resource Management data because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_SQL_WARNING(131)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 131
SymbolicName = CFG_GET_RESOURCE_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving Resource Management Policy: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_PARAM_ERROR(132)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 132
SymbolicName = CFG_GET_RESOURCE_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving Resource Management Policy: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_SQL_ERROR(133)
;//*	Severity: 	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 133
SymbolicName = CFG_GET_RESOURCE_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving Resource Management Policy: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_NO_CFG_SRVR(134)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 134
SymbolicName = CFG_GET_RESOURCE_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Resource Management Policy because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_RESOURCE_CFG_SRVR_GONE(135)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 135
SymbolicName = CFG_GET_RESOURCE_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Resource Management Policy because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_NETWORK_ERROR(140)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 140
SymbolicName = CFG_SET_RESOURCE_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Resource Management Policy changes because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_SQL_WARNING(141)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 141
SymbolicName = CFG_SET_RESOURCE_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while applying Resource Management Policy changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_PARAM_ERROR(142)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 142
SymbolicName = CFG_SET_RESOURCE_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while applying Resource Management Policy changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_SQL_ERROR(143)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 143
SymbolicName = CFG_SET_RESOURCE_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while applying Resource Management Policy changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_NO_CFG_SRVR(144)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 144
SymbolicName = CFG_SET_RESOURCE_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Resource Management Policy changes because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_RESOURCE_CFG_SRVR_GONE(145)
;//*	Severity: 	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 145
SymbolicName = CFG_SET_RESOURCE_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Resource Management Policy changes because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_NETWORK_ERROR(150)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 150
SymbolicName = CFG_GET_DSN_CTRL_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source control data because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_SQL_WARNING
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 151
SymbolicName = CFG_GET_DSN_CTRL_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving Data Source control data: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_PARAM_ERROR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 152
SymbolicName = CFG_GET_DSN_CTRL_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving Data Source control data: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_SQL_ERROR(153)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 153
SymbolicName = CFG_GET_DSN_CTRL_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving Data Source control data: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_NO_CFG_SRVR(154)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 154
SymbolicName = CFG_GET_DSN_CTRL_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source control data because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DSN_CTRL_CFG_SRVR_GONE(155)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 155
SymbolicName = CFG_GET_DSN_CTRL_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source control data because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_NETWORK_ERROR(160)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 160
SymbolicName = CFG_SET_DSN_CTRL_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source [%2!s!] control changes because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_SQL_WARNING(161)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 161
SymbolicName = CFG_SET_DSN_CTRL_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while applying Data Source control changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_PARAM_ERROR(162)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 162
SymbolicName = CFG_SET_DSN_CTRL_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while applying Data Source control changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_SQL_ERROR(163)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 163
SymbolicName = CFG_SET_DSN_CTRL_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while applying Data Source control changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_NO_CFG_SRVR(164)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 164
SymbolicName = CFG_SET_DSN_CTRL_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source [%1!s!] control changes because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DSN_CTRL_CFG_SRVR_GONE(165)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 165
SymbolicName = CFG_SET_DSN_CTRL_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source [%1!s!] control changes because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_NETWORK_ERROR(170)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 170
SymbolicName = CFG_SET_DS_STATUS_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source status changes because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_SQL_WARNING(171)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 171
SymbolicName = CFG_SET_DS_STATUS_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while applying Data Source status changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_PARAM_ERROR(172)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 172
SymbolicName = CFG_SET_DS_STATUS_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while applying Data Source status changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_SQL_ERROR(173)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 173
SymbolicName = CFG_SET_DS_STATUS_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while applying changes to Data Source status changes: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_NO_CFG_SRVR(174)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 174
SymbolicName = CFG_SET_DS_STATUS_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source [%1!s!] status changes because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SET_DS_STATUS_CFG_SRVR_GONE(175)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 175
SymbolicName = CFG_SET_DS_STATUS_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to apply Data Source [%1!s!] status changes because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_NETWORK_ERROR(180)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 180
SymbolicName = CFG_GET_STARTUP_VALUES_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve startup values because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_SQL_WARNING(181)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 181
SymbolicName = CFG_GET_STARTUP_VALUES_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving startup values: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_PARAM_ERROR(182)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 182
SymbolicName = CFG_GET_STARTUP_VALUES_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving startup values: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_SQL_ERROR(183)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 183
SymbolicName = CFG_GET_STARTUP_VALUES_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving startup values: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_NO_CFG_SRVR(184)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 184
SymbolicName = CFG_GET_STARTUP_VALUES_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve startup values because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_STARTUP_VALUES_CFG_SRVR_GONE(185)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 185
SymbolicName = CFG_GET_STARTUP_VALUES_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve startup values because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_NETWORK_ERROR(190)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 190
SymbolicName = CFG_GET_DATASOURCE_VALUES_NETWORK_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source values for Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_SQL_WARNING(191)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 191
SymbolicName = CFG_GET_DATASOURCE_VALUES_SQL_WARNING
Severity=Warning
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL warning has been detected while retrieving Data Source values for Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_PARAM_ERROR(192)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 192
SymbolicName = CFG_GET_DATASOURCE_VALUES_PARAM_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving Data Source values for Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_SQL_ERROR(193)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 193
SymbolicName = CFG_GET_DATASOURCE_VALUES_SQL_ERROR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz A SQL error has been detected while retrieving Data Source values for Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_NO_CFG_SRVR(194)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 194
SymbolicName = CFG_GET_DATASOURCE_VALUES_NO_CFG_SRVR
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source values for Data Source Name [%1!s!] because the Configuration Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_GET_DATASOURCE_VALUES_CFG_SRVR_GONE(195)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 195
SymbolicName = CFG_GET_DATASOURCE_VALUES_CFG_SRVR_GONE
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Unable to retrieve Data Source values for Data Source Name [%1!s!] because the Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_REQUEST_TIMED_OUT(196)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 196
SymbolicName = CFG_REQUEST_TIMED_OUT
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz The request to Association Server or Configuration Server timed out. Retry again%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_EXCEPTION_OCCURRED(197)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 197
SymbolicName = CFG_EXCEPTION_OCCURRED
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz Exception Occurred in %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_NETWORK_ERROR(200)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 200
SymbolicName = CFG_AS_START_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to Start the Association Server because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_SERVICE_ALREADY_STARTED(201)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 201
SymbolicName = CFG_AS_START_SERVICE_ALREADY_STARTED
Severity=Warning
Facility=AssociationServer
Language=English
zzzzz zzzzz The Association Server was already started.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_SQL_WARNING(202)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 202
SymbolicName = CFG_AS_START_SQL_WARNING
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A SQL warning has been detected while starting the Association Server: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_PARAM_ERROR(203)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 203
SymbolicName = CFG_AS_START_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while starting the Association Server: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_STATE_ERROR(204)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 204
SymbolicName = CFG_AS_START_STATE_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz An internal State transition error was detected while starting the Association Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_PORT_NOT_AVAILABLE(205)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 205
SymbolicName = CFG_AS_START_PORT_NOT_AVAILABLE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A TCP/IP port was not available while starting the Association Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_SQL_ERROR(206)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 206
SymbolicName = CFG_AS_START_SQL_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A SQL error has been detected while starting the Association Server: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_RETRY_EXCEEDED(207)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 207
SymbolicName = CFG_AS_START_RETRY_EXCEEDED
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz The number of attempts [%1!d!] to start the Association Server has been exceeded.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_SERVERS_NOT_CREATED(208)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 208
SymbolicName = CFG_AS_START_SERVERS_NOT_CREATED
Severity=Warning
Facility=AssociationServer
Language=English
zzzzz zzzzz The Association Server was unable to create some of the SQL/MX servers.  Check Event Log for details.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_CFGSRVR_FAILED(209)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 209
SymbolicName = CFG_AS_START_CFGSRVR_FAILED
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz The Configuration Server failed to register back with the Association Server.  Check Event log for details.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_NO_AS_SRVR(210)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 210
SymbolicName = CFG_AS_START_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to Start the Association Server because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_START_AS_SRVR_GONE(211)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 211
SymbolicName = CFG_AS_START_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to Start the Association Server because the Association Server or Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_NETWORK_ERROR(220)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 220
SymbolicName = CFG_AS_STOP_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to Stop the Association Server because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_PARAM_ERROR(221)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 221
SymbolicName = CFG_AS_STOP_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while stopping the Association Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_PROCESS_ERROR(222)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 222
SymbolicName = CFG_AS_STOP_PROCESS_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A process error was detected while stopping the Association Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_STATE_ERROR(223)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 223
SymbolicName = CFG_AS_STOP_STATE_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz An internal State transition error was detected while stopping the Association Server.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_NO_AS_SRVR(224)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 224
SymbolicName = CFG_AS_STOP_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the Association Server because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STOP_AS_SRVR_GONE(225)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 225
SymbolicName = CFG_AS_STOP_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to Stop the Association Server because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STATUS_NETWORK_ERROR(230)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 230
SymbolicName = CFG_AS_STATUS_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to get Status from the Association Server because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STATUS_NO_AS_SRVR(231)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 231
SymbolicName = CFG_AS_STATUS_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to get Status from the Association Server because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_AS_STATUS_AS_SRVR_GONE(232)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 232
SymbolicName = CFG_AS_STATUS_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to get Status from the Association Server because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_NETWORK_ERROR(240)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 240
SymbolicName = CFG_DS_START_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start the Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_PARAM_ERROR(241)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 241
SymbolicName = CFG_DS_START_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while starting the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_SQL_WARNING(242)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 242
SymbolicName = CFG_DS_START_SQL_WARNING
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A SQL warning has been detected while starting the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_NO_DSN(243)
;//*	Severity: 	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 243
SymbolicName = CFG_DS_START_NO_DSN
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start the the Data Source Name [%1!s!] because it does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_ALREADY_STARTED(244)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 244
SymbolicName = CFG_DS_START_ALREADY_STARTED
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz The Data Source Name [%1!s!] was already started.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_STATE_ERROR(245)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 245
SymbolicName = CFG_DS_START_STATE_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Internal State transition error was detected while starting the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_SQL_ERROR(246)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 246
SymbolicName = CFG_DS_START_SQL_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A SQL Error was detected while starting the Data Source Name [%1!s!]: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_SRVR_CREATE(247)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 247
SymbolicName = CFG_DS_START_SRVR_CREATE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A server creation error was detected while starting the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_NO_PORT(248)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 248
SymbolicName = CFG_DS_START_NO_PORT
Severity=Error
Facility=ConfigurationClient
Language=English
zzzzz zzzzz No port was available to start the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_NO_AS_SRVR(249)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 249
SymbolicName = CFG_DS_START_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start the Data Source Name [%1!s!] because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_START_AS_SRVR_GONE(250)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 250
SymbolicName = CFG_DS_START_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start the Data Source Name [%1!s!] because the Association Server or Configuration Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_NETWORK_ERROR(260)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 260
SymbolicName = CFG_DS_STOP_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable stop the Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_PARAM_ERROR(261)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 261
SymbolicName = CFG_DS_STOP_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while stopping the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_NO_DSN(262)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 262
SymbolicName = CFG_DS_STOP_NO_DSN
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the Data Source Name [%1!s!] because it does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_ALREADY_STOPPED(263)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 263
SymbolicName = CFG_DS_STOP_ALREADY_STOPPED
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz The Data Source Name [%1!s!] was already stopped.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_STATE_ERROR(264)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 264
SymbolicName = CFG_DS_STOP_STATE_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Internal State transition error was detected while stopping the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_PROCESS_ERROR(265)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 265
SymbolicName = CFG_DS_STOP_PROCESS_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Terminate Process error was detected while stopping the Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_NO_AS_SRVR(266)
;//*	Severity: 	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 266
SymbolicName = CFG_DS_STOP_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the Data Source Name [%1!s!] because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STOP_AS_SRVR_GONE(267)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 267
SymbolicName = CFG_DS_STOP_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the Data Source Name [%1!s!] because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STATUS_NETWORK_ERROR(270)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 270
SymbolicName = CFG_DS_STATUS_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable retrieve status information on Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STATUS_PARAM_ERROR(271)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 271
SymbolicName = CFG_DS_STATUS_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving status information on Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STATUS_NO_DSN(272)
;//*	Severity: 	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 272
SymbolicName = CFG_DS_STATUS_NO_DSN
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on Data Source Name [%1!s!] because it does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STATUS_NO_AS_SRVR(273)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 273
SymbolicName = CFG_DS_STATUS_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on Data Source Name [%1!s!] because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_STATUS_AS_SRVR_GONE(274)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 274
SymbolicName = CFG_DS_STATUS_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on Data Source Name [%1!s!] because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_ALL_STATUS_NETWORK_ERROR(280)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 280
SymbolicName = CFG_DS_ALL_STATUS_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable retrieve status information on all Data Sources because of a network error: %1!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_ALL_STATUS_PARAM_ERROR(281)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 281
SymbolicName = CFG_DS_ALL_STATUS_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving status information on all Data Sources.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_ALL_STATUS_NO_AS_SRVR(282)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 282
SymbolicName = CFG_DS_ALL_STATUS_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on Data Sources because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_ALL_STATUS_AS_SRVR_GONE(283)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 283
SymbolicName = CFG_DS_ALL_STATUS_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on Data Sources because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_NETWORK_ERROR(290)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 290
SymbolicName = CFG_DS_DETAIL_STATUS_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable retrieve detailed status information on Data Source Name [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_PARAM_ERROR(291)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 291
SymbolicName = CFG_DS_DETAIL_STATUS_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving detailed status information on Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_NO_DSN(292)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 292
SymbolicName = CFG_DS_DETAIL_STATUS_NO_DSN
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve detailed status information on Data Source Name [%1!s!] because it does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_AS_NOT_AVAILABLE(293)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 293
SymbolicName = CFG_DS_DETAIL_STATUS_AS_NOT_AVAILABLE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve detailed status information on Data Source Name [%1!s!] because the Association Server is not available.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_DS_NOT_AVAILABLE(294)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 294
SymbolicName = CFG_DS_DETAIL_STATUS_DS_NOT_AVAILABLE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve detailed status information on Data Source Name [%1!s!] because the Data Source is not available.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_NO_AS_SRVR(295)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 295
SymbolicName = CFG_DS_DETAIL_STATUS_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve detailed status information on Data Source Name [%1!s!] because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_DS_DETAIL_STATUS_AS_SRVR_GONE(296)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 296
SymbolicName = CFG_DS_DETAIL_STATUS_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve detailed status information on Data Source Name [%1!s!] because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_ALL_STATUS_NETWORK_ERROR(300)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 300
SymbolicName = CFG_SRVR_ALL_STATUS_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable retrieve status information on all SQL/MX servers because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_ALL_STATUS_PARAM_ERROR(301)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 301
SymbolicName = CFG_SRVR_ALL_STATUS_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while retrieving detailed status information on Data Source Name [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_ALL_STATUS_AS_NOT_AVAILABLE(302)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 302
SymbolicName = CFG_SRVR_ALL_STATUS_AS_NOT_AVAILABLE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on all SQL/MX servers because the Association Server is not available.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_ALL_STATUS_NO_AS_SRVR(303)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 303
SymbolicName = CFG_SRVR_ALL_STATUS_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on all SQL/MX servers because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_ALL_STATUS_AS_SRVR_GONE(304)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 304
SymbolicName = CFG_SRVR_ALL_STATUS_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to retrieve status information on all SQL/MX servers because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_NETWORK_ERROR(310)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 310
SymbolicName = CFG_SRVR_STOP_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable stop the SQL/MX server [%1!s!] because of a network error: %2!s!.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_PARAM_ERROR(311)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 311
SymbolicName = CFG_SRVR_STOP_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Parameter error has been detected while stopping the SQL/MX server [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_AS_NOT_AVAILABLE(312)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 312
SymbolicName = CFG_SRVR_STOP_AS_NOT_AVAILABLE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the SQL/MX server [%1!s!] because the Association Server is not available.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_SRVR_NOT_FOUND(313)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 313
SymbolicName = CFG_SRVR_STOP_SRVR_NOT_FOUND
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the SQL/MX server [%1!s!] because the it is not found.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_USED_OTHER_CLIENT(314)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 314
SymbolicName = CFG_SRVR_STOP_USED_OTHER_CLIENT
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the SQL/MX server [%1!s!] because it is in use by a different client application.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_PROCESS_ERROR(315)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 315
SymbolicName = CFG_SRVR_STOP_PROCESS_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz A Process Terminate error was detected while stopping the SQL/MX server [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_NO_AS_SRVR(316)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 316
SymbolicName = CFG_SRVR_STOP_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the SQL/MX server because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_AS_SRVR_GONE(317)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 317
SymbolicName = CFG_SRVR_STOP_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to stop the SQL/MX server because the Association Server dissapeared.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_START_CFG_NETWORK_ERROR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 320
SymbolicName = CFG_START_CFG_NETWORK_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to obtain configuration server object reference because of a network error: [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_START_CFG_NO_AS_SRVR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 321
SymbolicName = CFG_START_CFG_NO_AS_SRVR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start configuration server because the Association Server does not exist.%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_START_CFG_PARAM_ERROR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 322
SymbolicName = CFG_START_CFG_PARAM_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to obtain configuration server object reference because a parameter error [%1!s!] has occurred in 
the Association Server .%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_START_CFG_TIMEOUT_ERROR
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 323
SymbolicName = CFG_START_CFG_TIMEOUT_ERROR
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to obtain configuration server object reference because 
the configuration server timed out [%1!s!].%0
.

;//************************************************************************
;//*
;//*	MessageId:	CFG_SRVR_STOP_AS_SRVR_GONE(324)
;//*	Severity:	Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID = 324
SymbolicName = CFG_START_CFG_AS_SRVR_GONE
Severity=Error
Facility=AssociationServer
Language=English
zzzzz zzzzz Unable to start configuration server because the Association Server dissapeared.%0
.

;//********************************EVENT LOG MESSAGES GO HERE ***********************
;// NOTE that the format is different from the Error messages that are sent back to
;// to client. You NEED to follow the event log message format given below
;//
;//***********************************************************************************
;// Categories (EMS Event Standard)
;// COMPONENT			STARTING NUMBER
;// Common				20000
;// SQL/MX server 			20400
;// Configuration Server		20600
;// Configuration Client		20700
;// MXCS Init Program			20800
;// OLE Server				20900
;// Association Server			21000
;// Cluster Administrator		21250
;// ************************ Sample Message *****************************************
;// MessageId: starts with 20000 
;//            note: these messages are not necessarily associated
;//            with SQLSTATE or SQLCODE messages
;//               :: Informational
;//               :: Success
;//               :: Warning
;//               :: Error
;// Severity:  {Success, Informational, Warning, Error}

;// Cause:   Informational.  Indicates starting of MX Connectivity Services
;// Effect:	 none
;// Recovery:none
;// Format: 
;//          parameter 1 indicates the PID 
;//          parameter 2 indicates which MXCS Component creates 
;//          this event message. 
;//          {MX Connectivity Services, SQL/MX server, Cfg Server, Association Server, ...}
;//          parameter 3 displays object reference.
;//          parameter 4 displays cluster name.
;//          parameter 5 displays node ID.

;// Example: 
;// Starting MX Connectivity Services. ...
;//
;// Process ID (PID): 312
;//	Component: Association Server
;//	Object Reference: TCP:TESTPC/18650
;//

  
;// *****************************************************
;//    COMMON EVENT MESSAGES
;// *****************************************************

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:	Informational
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*	Format: 
;//*         parameter 1 indicates NT error 
;//*
;//************************************************************************
MessageId=20000
Severity=Informational
SymbolicName=LogEventLoggingResumed
Language=English
Event logging was temporarily suspended due to NT error %1 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*	Format: 
;//*         parameter 1 indicates the PID 
;//*         parameter 2 indicates MXCS Component creates 
;//*         this event message. 
;//*         parameter 3 displays object reference.
;//*         parameter 6 displays the Windows NT Error detected.		
;//*
;//************************************************************************
MessageId=20001
Severity=Error
SymbolicName=MSG_ODBC_NT_ERROR
Language=English
A Windows NT error %6 has occurred %n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*	Format: 
;//*         parameter 1 indicates the PID 
;//*         parameter 2 indicates MXCS Component creates 
;//*         this event message. 
;//*         parameter 3 displays object reference.
;//*         parameter 6 displays the detailed text for this error condition.
;//*
;//************************************************************************
MessageId=20002
Severity=Error
SymbolicName=MSG_PROGRAMMING_ERROR
Language=English
An unexpected program exception has occurred, contact your service provider%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Detail Text: %6 %n
.

;//At present, we do not internationalize the error text that comes from SQL and Krypton

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20003
Severity=Error
SymbolicName=MSG_KRYPTON_EXCEPTION
Language=English
A Network Component exception %6 has occurred.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Detail Text: %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID=20004
Severity=Error
SymbolicName=MSG_SQL_ERROR
Language=English
A SQL error %7 has occurred.%n
%n
Process ID: %1 %n
Reporting Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Origin Component: %6 %n
Detail Text: %8 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20005
Severity=Error
SymbolicName=MSG_KRYPTON_ERROR
Language=English
A Network Component error %6 has occurred.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Detail Text: %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20006
Severity=Error
SymbolicName=MSG_SRVR_REGISTER_ERROR
Language=English
SQL/MX server failed to register to the MX Connectivity Service due to previous error(s).%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20007
Severity=Error
SymbolicName=MSG_ODBC_NSK_ERROR
Language=English
A NonStop Process Services error %6 has occurred %n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20008
Severity=Informational
SymbolicName=MSG_SRVR_ENV
Language=English
MXCS Object reports Environment %n
%n
Process ID: %1 %n
Component: %2 %n
Object Reference: %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Env: %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20009
Severity=Error
SymbolicName=MSG_MEMORY_ALLOCATION_ERROR
Language=English
Memory allocation Error in the function %6%n
%n
Process ID: %1 %n
Component: %2 %n
Object Reference: %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID=20010
Severity=Warning
SymbolicName=MSG_SQL_WARNING
Language=English
A SQL warning %7 has occurred.%n
%n
Process ID: %1 %n
Reporting Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Origin Component: %6 %n
Detail Text: %8 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID=20011
Severity=Error
SymbolicName=MSG_DEFINESETATTR_ERROR
Language=English
An error %6 has occurred while setting the attribute %7 with value %8 for Define %9.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageID=20012
Severity=Error
SymbolicName=MSG_DEFINESAVE_ERROR
Language=English
A Define error %6 has occurred while saving the Define %7.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;// *****************************************************
;//    SQL/MX server EVENT MESSAGES
;// *****************************************************

;//************************************************************************
;//*
;//*	MessageId: 	MSG_ODBCMX_RG_LOG
;//*	Severity:  	Warning
;//*	Cause:		Estimated cost exceeds the max cost limit.
;//*	Effect:		Log the query.
;//*	Recovery:	None.
;//*	Format: 
;//*         parameter 1 indicates the PID 
;//*         parameter 2 indicates SQL/MX server Component creates 
;//*         this event message. 
;//*         parameter 3 displays object reference.
;//*         parameter 4 displays estimated cost.	
;//*         parameter 5 displays cost limit.
;//*         parameter 6 displays query.
;//*
;//************************************************************************

MessageId=20400
Severity=Warning
SymbolicName=MSG_ODBCMX_RG_LOG
Language=English
Query Estimated cost, %6, exceeds resource policy %7.  Statement written to log.%n
%n
Process ID (PID):  %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Query:  %8 %n
.

;//************************************************************************
;//*
;//*	MessageId: MSG_ODBCMX_RG_STOP
;//*	Severity:  Error
;//*	Cause:   Estimated cost exceeds the max cost limit.
;//*	Effect:	 Stop the query.
;//*	Recovery:Reconstruct your query and make it smaller.
;//*	Format: 
;//*          parameter 1 indicates the PID 
;//*          parameter 2 indicates SQL/MX server Component creates 
;//*          this event message. 
;//*          parameter 3 displays object reference.
;//*          parameter 4 displays estimated cost.	
;//*          parameter 5 displays cost limit.
;//*          parameter 6 displays query.
;//*
;//************************************************************************

MessageId=20401
Severity=Error
SymbolicName=MSG_ODBCMX_RG_STOP
Language=English
Query Estimated cost, %6, exceeds resource policy %7.  Query marked un-executable.%n 
%n
Process ID (PID):  %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Query:  %8 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20402
Severity=Error
SymbolicName=MSG_SRVR_MONITOR_CALL_FAILED
Language=English
The monitor object call %4 to the MX Connectivity Service failed due to previous error(s).%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20403
Severity=Error
SymbolicName=MSG_SRVR_IDLE_TIMEOUT_ERROR
Language=English
The server failed to timeout when in idle state, due to an error in the object call
to MX Connectivity Service or timer creation error. See previous error(s) %n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20404
Severity=Error
SymbolicName=MSG_UPDATE_SRVR_STATE_FAILED
Language=English
SQL/MX server failed to update its state in the MX Connectivity Service due to previous error(s).%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20420
Severity=Error
SymbolicName=MSG_SRVR_DTC_TIP_NOTCONNECTED
Language=English
SQL/MX server failed due to user has been disconnected from the TIP gateway.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20421
Severity=Error
SymbolicName=MSG_SRVR_DTC_TIP_NOTCONFIGURED
Language=English
SQL/MX server failed due to TIP gateway has not been configured.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20422
Severity=Error
SymbolicName=MSG_SRVR_DTC_TIP_ERROR
Language=English
SQL/MX server failed due to TIP gateway error.%n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20423
Severity=Error
SymbolicName=MSG_SRVR_POST_CONNECT_ERROR
Language=English
Client fails to connect due to error in setting the connection context. %n
Context Attribute : %6 %n
Context Attribute Value : %7 %n
%n
Process ID: %1 %n
Component:  %2 %n
Object Reference:  %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;// *****************************************************
;//    MXCS INIT EVENT MESSAGES
;// *****************************************************
;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20800
Severity=Informational
SymbolicName=MSG_ODBC_INIT_STARTED
Language=English
MXCS Initialization Operation [%6] Started %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20801
Severity=Informational
SymbolicName=MSG_ODBC_INIT_COMPLETED
Language=English
MXCS Initialization Operation [%6] Completed %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;// *****************************************************
;//    OLE SERVER EVENT MESSAGES
;// *****************************************************

;//************************************************************************
;//*
;//*	MessageId: MSG_ODBCMX_OLEINIT_FAILURE
;//*	Severity:  Error
;//*	Cause:   Correct OLE libraries are missing.
;//*	Effect:	 OLE initialization failed.
;//*	Recovery:Make sure that the OLE libraries are the correct version.
;//*	Format: 
;//*          parameter 1 indicates the PID 
;//*          parameter 2 indicates which MXCS Component creates 
;//*          this event message. 
;//*          {MX Connectivity Services, SQL/MX server, Cfg Server, Association Server, ...}
;//*          parameter 3 displays object reference.
;//*
;//************************************************************************

;// Example: 
;// OLE initialization failed.  Make sure that the OLE libraries are the correct version.
;//
;// Process ID (PID): 312
;//	Component: MXCS OLE Server
;//  

MessageId=20900
Severity=Error
SymbolicName=MSG_ODBCMX_OLEINIT_FAILURE
Language=English
OLE initialization failed.  Make sure that the OLE libraries are the correct version.%n
%n
Process ID (PID):  %1 %n
Component:  %2 %n
Cluster Name: %3 %n
Node ID: %4 %n
.

;//******************************************************
;// Configuration Server
;//******************************************************
;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=20950
Severity=Informational
SymbolicName=MSG_CONFIG_SRVR_INITIALIZED
Language=English
MXCS Configuration Server is Initialized.%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;// ****************************************************
;//    ASSOCIATION SERVER EVENT MESSAGES
;// ****************************************************   
;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21000
Severity=Error
SymbolicName=MSG_REG_SRVR_ERROR
Language=English
Unable to register the SQL/MX server %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Server Object Reference : %6 %n
Server Type : %7 %n
.


;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21001
Severity=Error
SymbolicName=MSG_SRVR_STATE_CHANGE_ERROR
Language=English
Server State Change Error from %7 to %8 %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Server Object Reference : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21002
Severity=Error
SymbolicName=MSG_START_SRVR_ERROR
Language=English
Starting Server failed due to previous error %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Server port number : %6 %n
Server Type : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21003
Severity=Error
SymbolicName=MSG_STOP_SRVR_ERROR
Language=English
Stopping Server failed due to previous error %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Server Process ID: %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21004
Severity=Error
SymbolicName=MSG_READ_REGISTRY_ERROR
Language=English
Reading SQL/MX and/or MXCS installation registry entries failed due to previous error %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.


;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21005
Severity=Error
SymbolicName=MSG_DS_STATE_CHANGE_ERROR
Language=English
Data source %6 state change error from %7 to %8 %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21006
Severity=Error
SymbolicName=MSG_PORT_NOT_AVAILABLE
Language=English
No more port available to start servers %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21007
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STARTED
Language=English
MX Connectivity Service is started%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21008
Severity=Informational
SymbolicName=MSG_DATASOURCE_STARTED
Language=English
The data source %6 is started %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21009
Severity=Error
SymbolicName=MSG_ODBC_SERVICE_START_FAILED
Language=English
MX Connectivity Service failed to start due to the previous error(s) %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21010
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STARTED_WITH_INFO
Language=English
MX Connectivity Service started with some failure(s). See previous event(s) %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21011
Severity=Error
SymbolicName=MSG_DATASOURCE_START_FAILED
Language=English
The data source %6 failed to start due to previous error(s) %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21012
Severity=Informational
SymbolicName=MSG_DATASOURCE_STARTED_WITH_INFO
Language=English
The data source %6 is started with some failure(s). See previous event(s)%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21013
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STOPPED
Language=English
MX Connectivity Service is stopped%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21014
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPED
Language=English
The data source %6 is stopped %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21015
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STOPPED_WITH_INFO
Language=English
MX Connectivity Service stopped with some failure(s) See previous event(s)%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21016
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPED_WITH_INFO
Language=English
The data source %6 stopped with some failure(s). See previous event(s) %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21017
Severity=Error
SymbolicName=MSG_SET_SRVR_CONTEXT_FAILED
Language=English
Setting the initial Server Context %6 to %7 failed
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21018
Severity=Informational
SymbolicName=MSG_ODBC_SERVER_STARTED_IN_DEBUG
Language=English
SQL/MX server is started in debug%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Debug Flag : %6 %n
DS Id : %7 %n
CEECFG Parms : %8 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21019
Severity=Informational
SymbolicName=MSG_SRVR_STATE_CHANGE_INFO
Language=English
Changing state from %7 to %8 %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Server Object Reference : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21020
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPING
Language=English
The data source %6 is stopping %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21021
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPED_ABRUPT
Language=English
The data source %6 is stopped abruptly %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21022
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPED_ABRUPT_WITH_INFO
Language=English
The data source %6 stopped abruptly with some failure(s). See previous event(s) %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21023
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPING_ABRUPT
Language=English
The data source %6 is stopping abruptly %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21024
Severity=Informational
SymbolicName=MSG_DATASOURCE_STOPPING_DISCONNECT
Language=English
The data source %6 is stopping in mode Stop-When-Disconnected %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %7 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21025
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STOPPED_ABRUPT
Language=English
MX Connectivity Service is stopped abruptly %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Reason : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21026
Severity=Warning
SymbolicName=MSG_ODBC_SERVICE_STARTED_WITH_WARNING
Language=English
MX Connectivity Service started with warning %7 from Configuration Server. %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Warning Text : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21027
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_STOPPED_BY_CA
Language=English
MX Connectivity Service is stopped on this node because CA is going down%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21028
Severity=Warning
SymbolicName=MSG_SQL_NOT_INITIALIZED
Language=English
SQL/MX has not successfully completed its initialization, %n
MX Connectivity Services can not be started. %n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21029
Severity=Informational
SymbolicName=MSG_ODBC_SERVICE_INITIALIZED
Language=English
MX Connectivity Service is initialized%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21030
Severity=Error
SymbolicName=MSG_SAVE_DSSTATUS_FAILED
Language=English
MX Connectivity Service failed to save %7 status change of datasource %6 due to previous error(s).%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21031
Severity=Error
SymbolicName=MSG_SAVE_ASSTATUS_FAILED
Language=English
MX Connectivity Service failed to save %6 status change of MX Connectivity Service due to previous error(s).%n
%n
Process ID: %1 %n
Component : %2 %n
Object Reference : %3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21032
Severity=Informational
Facility=Application
SymbolicName=MSG_INTERNAL_COUNTER_RECALCULATED
Language=English
MX Connectivity Service recalucated its internal counters.%n
The old values are Server Registered : %6 Server Connected : %7 %n
The new values are Server Registered : %8 Server Connected : %9 %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 21033
;//*	Severity:  Warning
;//*	Cause:	   A Trace Collector is not running.		
;//*	Effect:	   SQL/MX server cannot write trace to collector.	
;//*	Recovery:  A Trace Collector should be started.	
;//*
;//************************************************************************
MessageId=21033
Severity=Warning
Facility=ODBCServer
SymbolicName=MSG_SERVER_COLLECTOR_ERROR
Language=English
SQL/MX server failed to write to %5 collector due to error %4.%n
%n
Session ID: %1 %n
Component: %2 %n
Object Reference: %3 %n
Error Message: %4 %n
Collector Name: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 21034
;//*	Severity:  Informational
;//*	Cause:	   A Server Trace Information message.
;//*	Effect:	   MXCS sends this message to trace collector.	
;//*	Recovery:  Informational message for Administrator.	
;//*
;//************************************************************************
MessageId=21034
Severity=Informational
Facility=ODBCServer
SymbolicName=MSG_SERVER_TRACE_INFO
Language=English
MXCS Trace%n
%n
Session ID: %1 %n
Function Name: %2 %n
Sequence ID: %3 %n
%4%n
.
;//************************************************************************
;//*
;//*	MessageId:
;//*    Severity: 
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21035
Severity=Informational
Facility=Application
SymbolicName=MSG_RES_STAT_INFO
Language=English
MXCS Statistics%n
%n
Session ID: %1 %n
Message Attribute : %2 %n
Sequence Number: %3 %n
Message Info: %4 %n
.
;//************************************************************************
;//*
;//*	MessageId:
;//*    Severity: 
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21036
Severity=Informational
Facility=Application
SymbolicName=MSG_QUERY_STATUS_INFO
Language=English
MXCS Query Status%n
%n
Session ID: %1 %n
Message Attribute : %2 %n
Sequence Number: %3 %n
Message Info: %4 %n
.
;// ****************************************************
;//    CLUSTER ADMINISTRATOR EVENT MESSAGES
;// ****************************************************   
;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21250
Severity=Informational
SymbolicName=MSG_CA_SVC_STARTED
Language=English
MXCS Cluster Administrator Started %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21251
Severity=Informational
SymbolicName=MSG_CA_SVC_STOPPED
Language=English
MXCS Cluster Administrator Stopped %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21252
Severity=Error
SymbolicName=MSG_CA_INIT_ERROR 
Language=English
MXCS Cluster Administrator Failed to Initialize due to previous error(s).%n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21253
Severity=Error
SymbolicName=MSG_CA_IPWATCH_IPADDR_ERROR
Language=English
IPWatch Service Failed to Pull Ip Address %7 %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
Error Subcode : %6 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21254
Severity=Error
SymbolicName=MSG_CA_IPWATCH_NOT_DYNAMIC
Language=English
IPWatch Failed to recover IP Address, %6 not dynamic address%n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21255
Severity=Error
SymbolicName=MSG_CA_IPWATCH_NO_SLOTS
Language=English
IPWatch Failed to recover IP Address, %6, no free slots %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21256
Severity=Error
SymbolicName=MSG_CA_IPWATCH_NOT_RELEASED
Language=English
IPWatch Failed to online IP Address, %6, since some other system could not offline this 
IP Address %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21257
Severity=Error
SymbolicName=MSG_CA_IPWATCH_NO_ADAPTOR
Language=English
IPWatch Failed to recover IP Address, %6 no appropriate adaptor %n
%n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21258
Severity=Error
Facility=Application
SymbolicName=MSG_START_ODBC_SERVICE_FAILED
Language=English
Cluster Administrator failed to start MX Connectivity Service due to previous error(s). %n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21259
Severity=Error
Facility=Application
SymbolicName=MSG_STARTING_ODBC_SERVICE
Language=English
Cluster Administrator starting MX Connectivity Service using the command line%n
%6 %n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.

;//************************************************************************
;//*
;//*	MessageId: 
;//*	Severity:  Error
;//*	Cause:		
;//*	Effect:		
;//*	Recovery:	
;//*
;//************************************************************************
MessageId=21260
Severity=Error
Facility=Application
SymbolicName=MSG_INSUFFICIENT_PRIVILEDGES
Language=English
The user has insufficient privileges to start MX Connectivity Service. %n
Process ID: %1 %n
Component : %2 %n
%3 %n
Cluster Name: %4 %n
Node ID: %5 %n
.
