# Protocol Used By LChat
a
Each Message consists of at least two parts, a header and a body:

## Header

|Name|Size (bytes)|Possible Values|Purpose|
|---|---|---|---|
|Message Type| 1 |LeaveRoom, JoinRoom, Chat|Inform Server what the rest of the message is about
|Room Index|4|1 - UINT32_MAX|For Join/Leave Room requests this is the room affected and for messages this is the room to send the message to.
|Size|4|0 - UINT32_MAX|Only exists for "Chat"s to inform server how large the incoming text is in bytes

## Body for Chat
|Name|Size (bytes)|Possible Values|Purpose|
|---|---|---|---|
|Date/Time|4|Valid date, encoded with the minute of day carrying 11 bits, year carrying 5 bits, month having 4 and day having another 5|The sender shouldn't fill this, the server tells clients when messages came in|
|User Id|4|1 - UINT32_MAX|What user sent the message|
|Message|variable|UTF-8 text|Message Content|
