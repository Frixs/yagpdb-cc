$cooldown[1s]
// ============ CONFIGURATION ============
$let[databaseChannelID;1442546776492998762]
$let[databaseMessageID;1444645737068626051]
$let[debugChannelID;1442546776492998762]
$let[notificationChannelID;1444408680924577916]
$let[databaseHeader;**\[CUSTOMCOMMANDS_AND_YAGPDB_MODULE_ENTRY_DATABASE\]**]
$let[reactionEmoji;👍]
$let[getMessageCooldown;60000] // 60 seconds
$let[timeoutProcessCooldown;70s] // 70 seconds

// DEBUG only:
// $setServerVar[db_timeout_flag;false]
// $channelSendMessage[$get[debugChannelID];DEBUG: Triggered command by user: $authorID, with role: $roleID]

// ============ ROLE FORMAT VALIDATION ============
$if[$regexCheck[$roleName;^/[A-Za-z]+/(unverified)?$]==false]
  $stop
$endIf

// ============ SEND USER NOTIFICATION ============
$channelSendMessage[$get[notificationChannelID];
<@$authorID>
{description:Tvé aktuální žádosti o přístup jsou zaregistrovány a vytvoří se do pár minut.}
{color:#3694e7}
{delete:120000}
;no]

// ============ GET USER UNVERIFIED ROLES ============
/*
$let[userRoles;$userRoles[$authorID;names;|]]
$let[unverifiedRoles;$regexMatch[$get[userRoles];\/[A-Za-z]+\/unverified;g;all;|]]
$let[data;]
$if[$get[unverifiedRoles]!=]
	$let[primaryRoles;$replaceText[$get[unverifiedRoles];unverified;]]
	$let[data;|$get[primaryRoles]]
$endIf
*/

// ============ RECORD PREPARATION ============
$let[currentTime;$timeStamp]
$let[newRecord;$authorID/*|$get[currentTime]$get[data]*/]
$let[timeoutRunning;$getServerVar[db_timeout_flag]] // indicates whether timeout process is running (see the code)

// ============ CHECK LAST GETMESSAGE TIMESTAMP ============
$let[lastGetTime;$getServerVar[db_last_getMessage]]
$if[$get[lastGetTime]==||$get[lastGetTime]==undefined]
	$let[lastGetTime;0]
$endIf
$let[timeSince;$sub[$get[currentTime];$get[lastGetTime]]]

// Check the last run of the $getMessage and check if the timeout process is not in the middle of the run.
$if[$get[timeSince]>$get[getMessageCooldown]&&$get[timeoutRunning]==false]
	// SAFE TO USE $GETMESSAGE - PROCESS DIRECTLY
	// $channelSendMessage[$get[debugChannelID];DEBUG: check 1]
	$setServerVar[db_last_getMessage;$get[currentTime]]

	// Get the content of the database message
	$let[messageContent;$getMessage[$get[databaseChannelID];$get[databaseMessageID];content]]
	$if[$checkContains[$get[messageContent];$get[databaseHeader]]==false]
		$let[messageContent;$get[databaseHeader]]
	$endIf

	// Check if there is reaction indicating need of refresh
	$let[messageReaction;$getReactions[$get[databaseChannelID];$get[databaseMessageID];$get[reactionEmoji];id]]
	$if[$get[messageReaction]==undefined||$get[messageReaction]==none||$get[messageReaction]==]
		// --- APPEND TO THE EXISTING CONTENT ---
		$let[headerLength;$textLength[$get[databaseHeader]]]
		$let[startPosition;$sum[$get[headerLength];0]]
		$let[currentData;$textSlice[$get[messageContent];$get[startPosition]]]

		$if[$get[currentData]!=&&$get[currentData]!=undefined]
			$let[newContent;$get[databaseHeader]$get[currentData],$get[newRecord]]
		$else
			$let[newContent;$get[databaseHeader]$get[newRecord]]
		$endIf
		
	$else
		// --- OVERWRITE THE EXISTING CONTENT ---
		$let[newContent;$get[databaseHeader]$get[newRecord]]
		$clearReactions[$get[databaseChannelID];$get[databaseMessageID];$get[reactionEmoji]]
		
	$endIf

	// Update the database message with the new content
	$editMessage[$get[databaseMessageID];$get[newContent]]

$else
	// COOLDOWN ACTIVE - USE BUFFER
	// $channelSendMessage[$get[debugChannelID];DEBUG: check 2]
	
	// Save it to the temporary server variable until we can save it to the database message once cooldown is ready.
	$let[pendingBuffer;$getServerVar[db_pending_buffer]]
	$if[$get[pendingBuffer]==||$get[pendingBuffer]==undefined]
		$setServerVar[db_pending_buffer;$get[newRecord]]
	$else
		$setServerVar[db_pending_buffer;$get[pendingBuffer],$get[newRecord]]
	$endIf

	// Only set timeout process if not already running
	$let[timeoutRunning;$getServerVar[db_timeout_flag]]
	$if[$get[timeoutRunning]==false]
		$setServerVar[db_timeout_flag;true]
		// $channelSendMessage[$get[debugChannelID];DEBUG: Timeout booting up!]
		
		// === === === TIMEOUT PROCESS === === ===
		// DRY is crying here, but we need to run this because of 1min cooldown on $getMessage
		$setTimeout[$get[timeoutProcessCooldown]]
			// $channelSendMessage[$get[debugChannelID];DEBUG: Timeout is starting!]
			$let[currentTime;$timeStamp]
			
			// SAFE TO USE $GETMESSAGE - PROCESS DIRECTLY
			$setServerVar[db_last_getMessage;$get[currentTime]]
		
			// Get the pending records and reset the value
			$let[pendingRecords;$getServerVar[db_pending_buffer]]
			$setServerVar[db_pending_buffer;]
		
			// Get the content of the database message
			$let[messageContent;$getMessage[$get[databaseChannelID];$get[databaseMessageID];content]]
			$if[$checkContains[$get[messageContent];$get[databaseHeader]]==false]
				$let[messageContent;$get[databaseHeader]]
			$endIf
		
			// Check if there is reaction indicating need of refresh
			$let[messageReaction;$getReactions[$get[databaseChannelID];$get[databaseMessageID];$get[reactionEmoji];id]]
			$if[$get[messageReaction]==undefined||$get[messageReaction]==none||$get[messageReaction]==]
				// --- APPEND TO THE EXISTING CONTENT ---
				$let[headerLength;$textLength[$get[databaseHeader]]]
				$let[startPosition;$sum[$get[headerLength];0]]
				$let[currentData;$textSlice[$get[messageContent];$get[startPosition]]]
		
				$if[$get[currentData]!=&&$get[currentData]!=undefined]
					$let[newContent;$get[databaseHeader]$get[currentData],$get[pendingRecords]]
				$else
					$let[newContent;$get[databaseHeader]$get[pendingRecords]]
				$endIf
				
			$else
				// --- OVERWRITE THE EXISTING CONTENT ---
				$let[newContent;$get[databaseHeader]$get[pendingRecords]]
				$clearReactions[$get[databaseChannelID];$get[databaseMessageID];$get[reactionEmoji]]
				
			$endIf
		
			// Turn off the timeout process FLAG!
			$setServerVar[db_timeout_flag;false]
		
			// Update the database message with the new content
			$editMessage[$get[databaseMessageID];$get[newContent]]
			// When the edit overflows 2k characters, it will throw an error.
		$endTimeout
		// === === === === === === === === === === === ===
	$endIf
$endIf
