# Protocol Used By LChat
a
Each Message consists of at least two parts, a header and a body:

## Header

|Name|Size (bytes)|Possible Values|Purpose|
|---|---|---|---|
|Message Type| 1 |LeaveRoom, JoinRoom, Chat, User Details, User ID|Inform Server what the rest of the message is about
|Room Index/User Index|4|1 - UINT32_MAX|For Join/Leave Room requests this is the room affected and for messages this is the room to send the message to. For User Detail requests this is the user in question
|Size|4|0 - UINT32_MAX|Only exists for string based messages to inform server how large the incoming text is in bytes

## Body for Chat
|Name|Size (bytes)|Possible Values|Purpose|
|---|---|---|---|
|Date/Time|4|Valid date, encoded with the minute of day carrying 11 bits, year carrying 5 bits, month having 4 and day having another 5|The sender shouldn't fill this, the server tells clients when messages came in|
|User Id|4|1 - UINT32_MAX|What user sent the message|
|Message|variable|UTF-8 text|Message Content|

## Body for User Detail
|Name|Size (bytes)|Possible Values|Purpose|
|---|---|---|---|
|Username|variable|UTF-8 text|Name of user|

## Note
On Connections to the server, the client will recieve a 4-byte message containing thier assigned id, and are expected to send a user detail message back to inform the server thier username
