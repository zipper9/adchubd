-- Simple history script that displays the last n history items
-- History is persisted across restarts

local base = _G

module("history")
base.require('luadchpp')
local adchpp = base.luadchpp

base.assert(base['access'], 'access.lua must be loaded and running before history.lua')
local access = base.access

-- Where to read/write history file - set to nil to disable persistent history
local history_file = adchpp.getDataPath() .. "history.txt"

local os = base.require('os')
local json = base.require('json')
local string = base.require('string')
local aio = base.require('aio')
local autil = base.require('autil')
local table = base.require('table')

local cm = adchpp.getCM()
local sm = adchpp.getSM()
local lm = adchpp.getLM()

local pos = 1
local messages_saved = true

local messages = {}

access.add_setting('history_max', {
	help = "number of messages to keep for +history",

	value = 500
})

access.add_setting('history_default', {
	help = "number of messages to display in +history if the user doesn't select anything else",

	value = 50
})

access.add_setting('history_connect', {
	help = "number of messages to display to the users on connect",
	
	value = 10
})

access.add_setting('history_method', {
	help = "strategy used by the +history script to record messages, restart the hub after the change, 1 = use a hidden bot, 0 = direct ADCH++ interface",

	value = 1
})

access.add_setting('history_prefix', {
	help = "prefix to put before each message in +history",

	value = "[%Y-%m-%d %H:%M:%S] "
})

local function log(message)
	lm:log(_NAME, message)
end

local function get_items(c)
	local items = 1
	local user = access.get_user_c(c)
	local from = user.lastofftime
	if from then
		for hist, data in base.pairs(messages) do
			if data.htime > from then
				items = items + 1
			end
		end
	end
	return items
end

local function get_lines(num)
	if num > access.settings.history_max.value then
		num = access.settings.history_max.value + 1
	end

	local s = 1

	if table.getn(messages) > access.settings.history_max.value then
		s = pos - access.settings.history_max.value + 1
	end

	if num < pos then
		s = pos - num + 1
	end

	local e = pos

	local lines = "Displaying the last " .. (e - s) .. " messages"

	while s <= e and messages[s] do
		lines = lines .. "\r\n" .. messages[s].message
		s = s + 1
	end
	
	return lines
end

access.commands.history = {
	alias = { hist = true },

	command = function(c, parameters)
		local items
		if #parameters > 0 then
			items = base.tonumber(parameters) + 1
			if not items then
				return
			end
		else
			if access.get_level(c) > 0 then
				items = get_items(c)
			end
		end
		if not items then
			items = access.settings.history_default.value + 1
		end
		
		autil.reply(c, get_lines(items))
	end,

	help = "[lines] - display main chat messages logged by the hub (no lines=default / since last logoff)",

	user_command = {
		name = "Chat history",
		params = { autil.ucmd_line("Number of msg's to display (empty=default / since last logoff)") }
		}
}

local function save_messages()
	if not history_file then
		return
	end

	local s = 1
	local e = pos
	if table.getn(messages) >= access.settings.history_max.value then
		s = pos - access.settings.history_max.value
		e = table.getn(messages)
	end

	local list = {}
	while s <= e and messages[s] do
		table.insert(list, messages[s])
		s = s + 1
	end
	messages = list
	pos = table.getn(messages) + 1

	local err = aio.save_file(history_file, json.encode(list))
	if err then
		log('History not saved: ' .. err)
	else
		messages_saved = true
	end
end

local function load_messages()
	if not history_file then
		return
	end

	local ok, list, err = aio.load_file(history_file, aio.json_loader)

	if err then
		log('History loading: ' .. err)
	end
	if not ok then
		return
	end

	for k, v in base.pairs(list) do
		messages[k] = v
		pos = pos + 1
	end
end

local function maybe_save_messages()
	if not messages_saved then
		save_messages()
	end
end

local function parse(cmd)
	if cmd:getCommand() ~= adchpp.AdcCommand_CMD_MSG or cmd:getType() ~= adchpp.AdcCommand_TYPE_BROADCAST then
		return
	end

	local from = cm:getEntity(cmd:getFrom())
	if not from then
		return
	end

	local nick = from:getField("NI")
	if #nick < 1 then
		return
	end

	local now = os.date(access.settings.history_prefix.value)
	local message
	
	if cmd:getParam("ME", 1) == "1" then
		message = now .. '* ' .. nick .. ' ' .. cmd:getParam(0)
	else
		message = now .. '<' .. nick .. '> ' .. cmd:getParam(0)
	end

	messages[pos] = { message = message, htime = os.time() }
	pos = pos + 1

	messages_saved = false
end

history_1 = cm:signalState():connect(function(entity)
	if access.settings.history_connect.value > 0 and entity:getState() == adchpp.Entity_STATE_NORMAL then
		autil.reply(entity, get_lines(access.settings.history_connect.value + 1))
	end
end)

load_messages()

if access.settings.history_method.value == 0 then
	history_1 = cm:signalReceive():connect(function(entity, cmd, ok)
		if not ok then
			return ok
		end

		parse(cmd)

		return true
	end)

else
	hidden_bot = cm:createBot(function(bot, buffer)
		parse(adchpp.AdcCommand(buffer))
	end)
	hidden_bot:setField('ID', hidden_bot:getCID():toBase32())
	hidden_bot:setField('NI', _NAME .. '-hidden_bot')
	hidden_bot:setField('DE', 'Hidden bot used by the ' .. _NAME .. ' script')
	hidden_bot:setFlag(adchpp.Entity_FLAG_HIDDEN)
	cm:regBot(hidden_bot)

	autil.on_unloaded(_NAME, function()
		hidden_bot:disconnect(adchpp.REASON_PLUGIN)
	end)
end

save_messages_timer = sm:addTimedJob(900000, maybe_save_messages)
autil.on_unloading(_NAME, save_messages_timer)

autil.on_unloading(_NAME, maybe_save_messages)
