# Module-Entry System V2
This solution is a bit more complex, so here is a straightforward explanation.

In the new Discord version, reaction role systems are no longer the main method for handling role assignment. Instead, Discord uses "Channels & Roles", where users can assign roles to themselves through a clean built in interface.

The problem is that sometimes you do not want certain roles to be assigned automatically. Some users should only receive them after someone approves the request. Discord does not support this by default. 
This system provides a way to require approval and then give the role once the request is accepted by using the "Channels & Roles" interface.

---

[YAGPDB](https://yagpdb.xyz/) is a powerful bot, but at the time this was made, it cannot trigger a command when someone is given or removed a role. Its competitor, [Custom Commands](https://ccommandbot.com/) (CC), can do that. The issue is that CC is much more strict and its free plan has very tight limits. It is greedy with resources, which makes anything advanced harder to do.

So how do we work around that?  
We generate a random message in a hidden channel. This message is used to pass data from CC to YAGPDB. It basically works as a middle man, a temporary cache, a data transfer point, call it whatever you want.

Here is how it works.  
The CC bot appends data to that message whenever someone is given a specific role. On the YAGPDB side, an interval command (for example every 5 minutes) reads this message. After reading it, YAGPDB adds a reaction to mark that the data was received. CC then before any append sees the reaction, clears the message, and starts adding new data again. Yes, the message can go over the 2000 character limit, but that would only happen on a very active server.

Now we get to the YAGPDB part, where the actual magic happens.
Based on the queries sent by CC, YAGPDB checks the roles. To access a restricted channel, the user needs only one role, but to keep things smooth and allow the user to remove the role on their own through the "Channels & Roles" interface, we need two roles. A primary one (for example called "/Access/") and a secondary one (for example called "/Access/unverified"). The naming matters for the current implementation. The slashes signal that these roles belong to a module.

On the restricted channel, everyone’s view permission must be blocked. The unverified role must block everything. Only the primary role is allowed to view the channel. When a user selects the option in "Channels & Roles", they receive both roles. This means they can technically see the channel before approval, but only new messages, not the history. And since the interval is five minutes, the system strips the roles again every cycle. This is the only downside, but it is acceptable.

Based on the queries received, an embed with buttons is generated. Only the correct roles can approve or deny it. If denied, the user is informed and both roles are removed. If accepted, only the unverified role is removed, leaving the user with full access.

---

There is also so-called request role. This role is applied in the "Channels & Roles" interface in each restricted choice along with other 2 roles. This can be used to show waiting channel where all the requests are bieng processed.

Also, it is good to mention that `process.yag` is too long for the free version of the bot. Just cut off the comments, and it is going to be fine.

To improve functionality, you can comment out the check for module roles in `create_query_for_request.cc` and configure it to strictly use the command when the request role is triggered only. In this case, it will work more precisely. However, whenever a user clicks on a choice in the "Channels & Roles" interface, they must be assigned all three roles, including the request role.

Request to a specific module can be done once per `$requestExpiryTime`. If a user leaves and tries to send a new request, it is possible only after the expiry time in the current implementation.

---

`process.yag` - Trigger: `Interval`, `5min`

`create_query_for_request.cc` - Custom Command bot, Trigger Type: `On Role Give/Take`, Trigger: `add`

`verify_access.yag` - Trigger Type: Message Component, Trigger: `^(approve|reject)-\d+-\d+$`